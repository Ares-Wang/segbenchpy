
// double interp3( 
//     const double *I, int w, int h, int d, 
//     double y, double x, double z ) {
//   x = x<0 ? 0 : (x>w-1.0001 ? w-1.0001 : x);
//   y = y<0 ? 0 : (y>h-1.0001 ? h-1.0001 : y);
//   z = z<0 ? 0 : (z>d-1.0001 ? d-1.0001 : z);
//   int x0=int(x), y0=int(y), z0=int(z);
//   int x1=x0+1, y1=y0+1, z1=z0+1;
//   double dx0=x-x0, dy0=y-y0, dz0=z-z0;
//   double dx1=1-dx0, dy1=1-dy0, dz1=1-dz0;


//     //(row-major is y,x,z
//   return I[(y0*w+x0)*d+z0]*dy1*dx1*dz1 +
//   I[(y0*w+x0)*d+z1]*dy1*dx1*dz0 +
//   I[(y0*w+x1)*d+z0]*dy1*dx0*dz1 +
//   I[(y0*w+x1)*d+z1]*dy1*dx0*dz0 +
//   I[(y1*w+x0)*d+z0]*dy0*dx1*dz1 +
//   I[(y1*w+x0)*d+z1]*dy0*dx1*dz0 +
//   I[(y1*w+x1)*d+z0]*dy0*dx0*dz1 +
//   I[(y1*w+x1)*d+z1]*dy0*dx0*dz0;
// }

#include <math.h>
#include <stdio.h>

#ifdef HAVE_GLOG
  #include <glog/logging.h>
#endif
double inline interp( 
    const double *I, int h, int w, int d, 
    double x, double y, double z ) {


  x = x<0 ? 0 : (x>w-1.0001 ? w-1.0001 : x);
  y = y<0 ? 0 : (y>h-1.0001 ? h-1.0001 : y);
  z = z<0 ? 0 : (z>d-1.0001 ? d-1.0001 : z);
  int x0=int(x), y0=int(y), z0=int(z);
  int x1=x0+1, y1=y0+1, z1=z0+1;
  double dx0=x-x0, dy0=y-y0, dz0=z-z0;
  double dx1=1-dx0, dy1=1-dy0, dz1=1-dz0;
    // row-major here is y,x,z
  return I[(y0*w+x0)*d+z0]*dy1*dx1*dz1 +
  I[(y0*w+x0)*d+z1]*dy1*dx1*dz0 +
  I[(y0*w+x1)*d+z0]*dy1*dx0*dz1 +
  I[(y0*w+x1)*d+z1]*dy1*dx0*dz0 +
  I[(y1*w+x0)*d+z0]*dy0*dx1*dz1 +
  I[(y1*w+x0)*d+z1]*dy0*dx1*dz0 +
  I[(y1*w+x1)*d+z0]*dy0*dx0*dz1 +
  I[(y1*w+x1)*d+z1]*dy0*dx0*dz0;
}

// Python stores arrays in y,x,z format.
// This function swaps x and y to keep this format for a python interface.
double interp3( 
    const double *I, int w, int h, int d, 
    double y, double x, double z ) {
    return interp(I,w,h,d,x,y,z);
}

void edgeNms3d(
    const double *E0, int h, int w, int d,
    const double *dX, int hdx, int wdx, int ddx,
    const double *dY, int hdy, int wdy, int ddy,
    const double *dZ, int hdz, int wdz, int ddz,
    double *E, int h2, int w2, int d2,
    float r, int s, float m) {
    for(int x=0; x<w; ++x ) for( int y=0; y<h; ++y ) for( int z=0; z<d; ++z ) {
          double e=E0[(y*w+x)*d+z];
          E[(y*w+x)*d+z]=e;
          if(!e) continue;
          e*=m;
          double dx = dX[(y*w+x)*d+z], dy = dY[(y*w+x)*d+z], dz = dZ[(y*w+x)*d+z];
          for( int dd=-1; dd<=1; dd++ ) if( dd ) {
          double e0 = interp(E0,h,w,d,
              x+dd*r*dx,
              y+dd*r*dy,
              z+dd*r*dz);
            if(e < e0) { E[(y*w+x)*d+z]=0;break; }
            }
      }
    s=s>w/2?w/2:s; s=s>h/2? h/2:s;s=s>d/2? d/2:s;
    // suppress noisy around x boundaries
    for(int x=0; x<s; ++x ) for( int y=0; y<h; ++y ) for( int z=0; z<d; ++z ) {
        E[(y*w+x)*d+z]*=x/double(s);
        E[(y*w+(w-1-x))*d+z]*=x/double(s);
    }
    // suppress noisy around y boundaries
    for(int x=0; x<w; ++x ) for( int y=0; y<s; ++y ) for( int z=0; z<d; ++z ) {
        E[(y*w+x)*d+z]*=y/double(s);
        E[((h-1-y)*w+x)*d+z]*=y/double(s);
    }
    // suppress noisy around y boundaries
    for(int x=0; x<w; ++x ) for( int y=0; y<h; ++y ) for( int z=0; z<s; ++z ) {
        E[(y*w+x)*d+z]*=z/double(s);
        E[(y*w+x)*d+(d-1-z)]*=z/double(s);
    }
}


#ifdef TEXT_EXE
#include <iostream>
#include <fstream>
#include <vector>
int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  using namespace std;

  std::vector<double> dXArray;
  ifstream dXfile("/home/AD/jmerkow/Code/caffe/notebook/ndim/Results/dX.txt");
  while (dXfile.good())
  {
      double temp;
      dXfile >> temp;
      dXArray.push_back(temp);
  }

  std::vector<double> dYArray;
  ifstream dYfile("/home/AD/jmerkow/Code/caffe/notebook/ndim/Results/dY.txt");
  while (dYfile.good())
  {
      double temp;
      dYfile >> temp;
      dYArray.push_back(temp);
  }

  std::vector<double> dZArray;
  ifstream dZfile("/home/AD/jmerkow/Code/caffe/notebook/ndim/Results/dZ.txt");
  while (dZfile.good())
  {
      double temp;
      dZfile >> temp;
      dZArray.push_back(temp);
  }

  std::vector<double> EArray;
  ifstream Efile("/home/AD/jmerkow/Code/caffe/notebook/ndim/Results/E.txt");
  while (Efile.good())
  {
      double temp;
      Efile >> temp;
      EArray.push_back(temp);
  }

  double Eout[128*128*32];

  edgeNms3d(&EArray[0], 128, 128, 32,
    &dXArray[0], 128, 128, 32,
    &dYArray[0], 128, 128, 32,
    &dZArray[0], 128, 128, 32,
    Eout, 128, 128, 32,
    1,0,1.001);


}
#endif