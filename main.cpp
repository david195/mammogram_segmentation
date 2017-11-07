/*
 * David Ruiz García.
 *
 * Segmentación de la mama
 *  -Detección del contorno
 *  -Detección del musculo pectoral
 *  -Deteccíon del pezon
*/


#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <algorithm>
#include <vector>

using namespace cv;
using namespace std;

/*Prototypes*/
void crop_image(Mat img);
void bounds();
int segment(string dir, int i);

/*Global Variables*/
int margin_l = 0;

Mat imgOriginal,imgCropped,imgFinal,imgThresholded;

/*Principal function*/
 int main( int argc, char** argv )
 {
  for(int i=1;i<=22; i++){
    ostringstream cad;
    cad << "bcdr_png/" << i << ".png";
    segment(cad.str(),i);
  }
  return 0;
}


int segment(string dir, int id){
  int iLowH = 0;
  int iHighH = 0;
  int iLowS = 0;
  int iHighS = 0;
  int iLowV = 0;
  int iHighV = 10;

  imgOriginal = imread(dir, CV_LOAD_IMAGE_COLOR);
  if(! imgOriginal.data ) {
     cout <<  "Could not open or find the image" << std::endl ;
     return -1;
  }

  /*Mask application*/
  Mat imgHSV,imgTh;
  cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
  inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgTh); //Threshold the image
  //morphological opening (remove small objects from the foreground)
  erode(imgTh, imgTh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
  dilate( imgTh, imgTh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
  //morphological closing (fill small holes in the foreground)
  dilate( imgTh, imgTh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
  erode(imgTh, imgTh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

  //Recortar margen
  crop_image(imgTh);

  //Delimitar bordes
  bounds();
  ostringstream cad;
  cad << "out/" << id << ".png";
  imwrite( cad.str(), imgFinal );
  //Mostrar imagenes
  /*imshow("Thresholded Image", imgThresholded); //show the thresholded image
  imshow("Original", imgOriginal); //show the original image
  imshow("Cropped", imgCropped); //show the original image
  imshow("Final", imgFinal); //show the original image*/
  cvWaitKey(0);
  return 0;
}

void crop_image(Mat img){
  int w = img.size().width;
  int h = img.size().height;
  int mx = w;
  for(int y=0; y<h; y++){
      float v;
      int x=0;
      do{
        Scalar intensity = img.at<uchar>(y, x);
        v = intensity[0];
        x++;
      }while(v!=0 && x<w);
      if(x<mx)mx=x;
  }
  int mt=0, mb=h-1;
  Scalar intensity = img.at<uchar>(mt, 5);
  float v = intensity[0];
  while(mt<h && v == 255){
    intensity = img.at<uchar>(mt, 5);
    v = intensity[0];
    mt++;
  }
  intensity = img.at<uchar>(mb, 5);
  v = intensity[0];
  while(mb>0 && v == 255){
    intensity = img.at<uchar>(mb, 5);
    v = intensity[0];
    mb--;
  }
  Mat ROI_O(imgOriginal, Rect(mx,mt,w-mx,mb-mt));
  ROI_O.copyTo(imgCropped);
  Mat ROI(img, Rect(mx,mt,w-mx,mb-mt));
  ROI.copyTo(imgThresholded);
  margin_l = mx;
}

void bounds(){
  imgFinal = imgCropped.clone();
  int w = imgCropped.size().width;
  int h = imgCropped.size().height;
  int y = h/2;
  int x = w-margin_l;
  Scalar intensity = imgThresholded.at<uchar>(y, x);
  float v=intensity[0];
  while(x>0 && v==255){
    Scalar intensity = imgThresholded.at<uchar>(y, x);
    v=intensity[0];
    x--;
  }
  int r = (x+w/2)/2;
  int mt = h/2;
  for(int ang=0; ang<360; ang++){
    int x = r* sin(ang);
    int y = r* cos(ang)+mt;
    if( x>0 && y>0){
      Vec3b color;
      color[0] = 0;
      color[1] = 0;
      color[2] = 255;
      imgFinal.at<Vec3b>(Point(x,y)) = color;
    }
  }


}
