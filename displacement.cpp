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
*/
void displacement(Mat &Tx, Mat &Ty)
{
  return;
}

int main(int argc, char* argv[])
{
  // set cx, cy, fx, fy;
  fx = 4320;
  fy = 8041.666666;
  cx = 1080;
  cy = 1920;

  // load normal map and depth map.
  if(read_normal_map("string addr") == -1) cout << "error in normal map" <<endl;
  if(read_depth_map("string addr") == -1) cout << "error in depth map" <<endl;

  // Matrix for depth difference map.
  Mat Tx(image_height, image_width, CV_32FC1);
  Mat Ty(image_height, image_width, CV_32FC1);
  ddm(Tx, Ty);

  // Generate Displacement map.
  displacement(Tx, Ty);

  // Save and Show image.

  return 0;
}