#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int image_height, image_width;

Mat normal_map;
Mat depth_map;
Mat depth_difference_map[2];
Mat displacement_map;

uint cx, cy, fx, fy;

int read_map()
{
    normal_map = imread("", IMREAD_UNCHANGED);
    if(normal_map.empty()) return -1;

    image_height = normal_map.rows;
    image_width = normal_map.cols;

    assert(normal_map.channels() == 3);
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
void ddm()
{
    Mat Tx(image_height, image_width, CV_32FC1);
    Mat Ty(image_height, image_width, CV_32FC1);

    float nx, ny, nz, z;
    for (int y = 0; y < image_height; y++)
    {
        uchar *depth_pointer = depth_map.ptr<uchar>(y);
        uchar *normal_pointer = normal_map.ptr<uchar>(y);
        uchar *tx_pointer = Tx.ptr<uchar>(y);
        uchar *ty_pointer = Ty.ptr<uchar>(y);

        for (int x = 0; x < image_width; x++)
        {
            nx = normal_pointer[x * 4];
            ny = normal_pointer[x * 4 + 1];
            nz = normal_pointer[x * 4 + 2];
            z =  depth_pointer[x]; // not sure.

            for (int l = 0; l < 3; l++)
            {
                
            }
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
void dispacement()
{

    
}

int main(int argc, char* argv[])
{}