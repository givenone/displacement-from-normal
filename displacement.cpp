#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int image_height, image_width;

Mat normal_map;
Mat depth_map;
Mat displacement_map;

float cx, cy, fx, fy;

int read_normal_map(string addr)
{
    normal_map = imread(addr, IMREAD_UNCHANGED);
    if(normal_map.empty()) return -1;

    image_height = normal_map.rows;
    image_width = normal_map.cols;

    assert(normal_map.channels() == 3);
    return 1;
}

int read_depth_map(string addr)
{
    depth_map = imread(addr, IMREAD_UNCHANGED);
    if(depth_map.empty()) return -1;

    if(image_height != depth_map.rows || image_width != depth_map.cols) return -1;
    Mat chans[3];
    split(depth_map, chans);
    depth_map = chans[0];
    assert(depth_map.channels() == 1);
    return 1;
}

/*
  arbitrary point p(x,y) = 
  (-(x-cx) * Z(x,y) / fx , 
   -(y-cy) * Z(x,y) / fz , 
   Z(x, y) ) and Np = (nx, ny, nz)
  
  dP / dx = T_x = 
  ( (1 / fx) * (- x * dz/dx  - Z  + cx * dz / dx),
    -((y - cy) / fy) * (dz / dx),
    dz/dx )

  T_x dot Np = 0
  ->
  (dz / dx) * (nz - nx * (x - cx) / fx - ny * (y - cy) / fy) = nx * z / fx

  Also T_y dot Np is same 
  ->
  (dz / dy) * (nz - nx * (x - cx) / fx - ny * (y - cy) / fy) = ny * z / fy

  depth difference vector is T_x , T_y.
*/
void ddm(Mat &Tx, Mat &Ty)
{
    float nx, ny, nz, z;
    for (int y = 0; y < image_height; y++)
    {
        float *depth_pointer = depth_map.ptr<float>(y);
        float *normal_pointer = normal_map.ptr<float>(y);
        float *tx_pointer = Tx.ptr<float>(y);
        float *ty_pointer = Ty.ptr<float>(y);

        for (int x = 0; x < image_width; x++)
        {
            nx = normal_pointer[x * 3];
            ny = normal_pointer[x * 3 + 1];
            nz = normal_pointer[x * 3 + 2];
            z =  depth_pointer[x]; // not sure.

            float lc = (-1 * nx * (x - cx) / fx) + (-1 * ny * (y- cy) / fy) + nz;
            float rxc = nx * z / fx;
            float ryc = ny * z / fy;
            tx_pointer[x] = rxc / lc;
            ty_pointer[x] = ryc / lc;
        }
    }

}

/* 
uniform approach
- Starting from zero depth integrate depth over circle
- Shoot N rays uniformly distributed over 360 º
- Height is reconstructed from DDM on the fly and added to integration sum
- Target zero displacement on average
- Offset texel by computed average

dsp : displacement map matrix
Tx : x-axis depth-difference (dz / dx)
Ty : y-axis depth-difference (dz / dy)
size : circle size (size for integrating adjacent pixels)
*/
void displacement(Mat &dsp, Mat &Tx, Mat &Ty, int size)
{
  for (int y = 0; y < image_height; y++)
  {
      float *d_pointer = dsp.ptr<float>(y);

      for (int x = 0; x < image_width; x++)
      {
          float depth = 0;
          int cnt = 0;
          // left
          
          if(x > 0)
          {
            depth += depth_map.at<float>(y, x-1) + Tx.at<float>(y, x-1);
            cnt++;
          }

          if(x < image_width - 1)
          {
            depth += depth_map.at<float>(y, x+1) - Tx.at<float>(y, x +1);
            cnt++;
          }
          
          if(y > 0)
          {
            depth += depth_map.at<float>(y-1, x) + Ty.at<float>(y-1, x);
            cnt++;
          }

          if(y < image_height - 1)
          {
            depth += depth_map.at<float>(y+1, x) - Ty.at<float>(y+1, x);
            cnt++;
          }

          d_pointer[x] = depth / cnt;

          /*
          for(int u = -1 * size; u < size; u++)
          {
            for(int v = -1 * size; v <size ; v++)
            {
              int xx = x + u;
              int yy = y + v;
              if(xx < 0 || yy < 0 || xx >= image_width || yy >= image_height) continue;

            }
          }
          */
      }
  }
  return;
}

int main(int argc, char* argv[])
{
  /*
  Mat img = imread("dist0.exr", IMREAD_UNCHANGED);
  if(img.empty()) cout << "failed" << endl;

  printf("%d %d %d \n", img.rows, img.cols, img.channels());
  Vec3f bgrPixel = img.at<Vec3f>(2000, 1000);
  cout << bgrPixel << endl;

  return 0;
  */


  // set cx, cy, fx, fy;
  fx = 4320;
  fy = -8041.666666;
  cx = 1080;
  cy = 1920;

  // load normal map and depth map.
  if(read_normal_map("syn_2.tif") == -1) cout << "error in normal map" <<endl;
  if(read_depth_map("dist0.exr") == -1) cout << "error in depth map" <<endl;

  /*
  Vec3f bgrPixel = normal_map.at<Vec3f>(2000, 1000);
  cout << bgrPixel << endl;
  float Pixel = depth_map.at<float>(2000, 1000);
  cout << Pixel << endl;
  */


  // Matrix for depth difference map.
  Mat Tx(image_height, image_width, CV_32FC1);
  Mat Ty(image_height, image_width, CV_32FC1);
  ddm(Tx, Ty);

  /*
  float Pixelx = Tx.at<float>(2000, 1000);
  float Pixely = Ty.at<float>(2000, 1000);
  cout << Pixelx <<endl;
  cout <<depth_map.at<float>(2000, 1001) - depth_map.at<float>(2000, 1000) <<endl;
  */

  // Generate Displacement map.
  Mat dsp(image_height, image_width, CV_32FC1);
  displacement(dsp, Tx, Ty, 3);
  printf("%f %f\n%f %f\n", dsp.at<float>(1950, 1000), dsp.at<float>(1950, 1001),depth_map.at<float>(1950, 1000), depth_map.at<float>(1950, 1001));
  // Save and Show image.
  Tx.release();
  Ty.release();
  return 0;
}