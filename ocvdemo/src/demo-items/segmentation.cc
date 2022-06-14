#include "demo-items/segmentation.hpp"
#include <iostream>
#ifdef USE_CONTRIB
#include "opencv2/ximgproc.hpp"
#endif


DemoSuperpixels::DemoSuperpixels()
{
  props.id = "superpixels";
}

int DemoSuperpixels::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
#ifdef USE_CONTRIB
  auto slic = cv::ximgproc::createSuperpixelSLIC(
      input.images[0], cv::ximgproc::SLICO,
      input.model.get_attribute_as_int("taille"),
      input.model.get_attribute_as_float("reglage"));
  slic->iterate();
  cv::Mat masque;
  slic->getLabelContourMask(masque);//);


  cv::Mat O = input.images[0].clone();
  O /= 2;
  O.setTo(cv::Scalar(0,0,0), masque);

  output.images[0] = O;
# else
  output.images[0] = input.images[0];
# endif
  return 0;
}

DemoMahalanobis::DemoMahalanobis()
{
  props.id = "mahalanobis";
  props.requiert_roi = true;
  input.roi =cv::Rect(116,77,134-116,96-77);
}


int DemoMahalanobis::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  cv::Mat I = input.images[0];

  output.nout = 2;
  output.images[0] = I.clone();

  if(input.roi.area() == 0)
  {
    output.images[1] = I.clone();
    infos("RDI invalide.");
    return 0;
  }

  cv::Mat ILab;
  cvtColor(I, ILab, cv::COLOR_BGR2Lab);

  infos("Compute covar samples...");
  auto M = ILab(input.roi);
# if 0
  cv::Mat samples(M.rows*M.cols,2,CV_32F);
  for(auto y = 0u; y < (unsigned int) M.rows; y++)
  {
    for(auto x = 0u; x < (unsigned int) M.cols; x++)
    {
      const cv::Vec3b &bgr = M.at<cv::Vec3b>(y,x);
      float sum = bgr[0] + bgr[1] + bgr[2];
      // R / (B + G +R)
      samples.at<float>(x+y*M.cols, 0) = bgr[2] / sum;
      // B / (B + G +R)
      samples.at<float>(x+y*M.cols, 1) = bgr[0] / sum;
    }
  }
# endif

 cv::Mat samples(M.rows*M.cols,3,CV_32F);

  for(auto y = 0u; y < (unsigned int) M.rows; y++)
  {
    for(auto x = 0u; x < (unsigned int) M.cols; x++)
    {
      const cv::Vec3b &Lab = M.at<cv::Vec3b>(y,x);
      samples.at<float>(x+y*M.cols, 0) = Lab[0];
      samples.at<float>(x+y*M.cols, 1) = Lab[1];
      samples.at<float>(x+y*M.cols, 2) = Lab[2];
    }
  }

  infos("Compute covar matrix...");
  cv::Mat covar, covari, mean;
  cv::calcCovarMatrix(samples, covar, mean,
                        cv::COVAR_NORMAL | cv::COVAR_SCALE | cv::COVAR_ROWS);

  cv::invert(covar, covari, cv::DECOMP_SVD);

  std::cout << "mean: " << mean << std::endl;
  std::cout << "covar: " << covar << std::endl;
  std::cout << "covari: " << covari << std::endl;

  mean.convertTo(mean, CV_32F);
  covari.convertTo(covari, CV_32F);

  infos("  Mahalanobis...");
  const cv::Vec3b *iptr = ILab.ptr<cv::Vec3b>();
  cv::Mat v(1,3,CV_32F);
  cv::Mat fmask(I.size(), CV_32F);
  float *optr = fmask.ptr<float>();

  float *pv = v.ptr<float>(0);

  for(auto i = 0u; i < input.images[0].total(); i++)
  {
    const cv::Vec3b &Lab = *iptr++;
    /*float sum = bgr[0] + bgr[1] + bgr[2];
    float cr = bgr[2] / sum;
    float cb = bgr[0] / sum;
    pv[0] = cr;
    pv[1] = cb;*/
    pv[0] = Lab[0];
    pv[1] = Lab[1];
    pv[2] = Lab[2];
    float dst = cv::Mahalanobis(mean, v, covari);
    *optr++ = dst;
  }

  infos(" normalize...");

  cv::normalize(fmask, fmask, 0, 255, cv::NORM_MINMAX);
  //fmask = 255.0 - fmask;
  fmask.convertTo(fmask, CV_8UC1);

  output.images[1] = fmask;

  return 0;
}

WShedDemo::WShedDemo()
{
  props.id = "watershed";
  output.nout = 3;
}

int WShedDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  cv::Mat gray, nb, ret;
  cvtColor(input.images[0], gray, cv::COLOR_BGR2GRAY);
  threshold(gray,nb,0,255,cv::THRESH_BINARY_INV | cv::THRESH_OTSU);



  //Execute morphological-open
  morphologyEx(nb,ret,cv::MORPH_OPEN,cv::Mat::ones(9,9,CV_8SC1),cv::Point(4,4),2);
  cv::Mat distTransformed(ret.rows,ret.cols,CV_32F);
  distanceTransform(ret,distTransformed, cv::DistanceTypes::DIST_L2,3);
  //normalize the transformed image in order to display
  normalize(distTransformed, distTransformed, 0.0, 255.0, cv::NORM_MINMAX);

  output.nout = 4;
  output.images[0] = input.images[0];
  output.images[1] = distTransformed.clone();
  output.names[1] = "Distance Transformation";

  //threshold the transformed image to obtain markers for watershed
  threshold(distTransformed,distTransformed,input.model.get_attribute_as_float("seuil-dist") * 
255,255,cv::THRESH_BINARY);
  distTransformed.convertTo(distTransformed,CV_8UC1);
  output.images[2] = distTransformed.clone();
  output.names[2] = "Thresholded dist. trans.";
  //imshow("Thresholded Distance Transformation",distTransformed);


  std::vector<std::vector<cv::Point> > contours;
  std::vector<cv::Vec4i> hierarchy;
  findContours(distTransformed, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);

  if(contours.empty())
    return -1;
  cv::Mat markers(distTransformed.size(), CV_32S);
  markers = cv::Scalar::all(0);
  int idx, compCount = 0;
  //draw contours

  for(idx = 0; idx >= 0; idx = hierarchy[idx][0], compCount++)
    drawContours(markers, contours, idx, cv::Scalar::all(compCount+1), -1, 8, hierarchy, INT_MAX);

  std::vector<cv::Vec3b> colorTab;
  for(auto i = 0; i < compCount; i++ )
  {
    int b = cv::theRNG().uniform(0, 255);
    int g = cv::theRNG().uniform(0, 255);
    int r = cv::theRNG().uniform(0, 255);
    colorTab.push_back(cv::Vec3b((uchar)b, (uchar)g, (uchar)r));
  }

  //apply watershed with the markers as seeds
  watershed(input.images[0], markers);

  output.images[3] = cv::Mat(markers.size(), CV_8UC3);
  infos("paint the watershed image...");
  for(auto i = 0; i < markers.rows; i++)
  {
    for(auto j = 0; j < markers.cols; j++)
    {
      int index = markers.at<int>(i,j);
      if(index == -1)
        output.images[3].at<cv::Vec3b>(i,j) = cv::Vec3b(255,255,255);
      else if((index <= 0) || (index > compCount))
        output.images[3].at<cv::Vec3b>(i,j) = cv::Vec3b(0,0,0);
      else
        output.images[3].at<cv::Vec3b>(i,j) = colorTab[index - 1];
    }
  }
  output.images[3] = output.images[3] * 0.5 + input.images[0] * 0.5;
  return 0;
}

GrabCutDemo::GrabCutDemo()
{
  props.id = "grabcut";
  props.requiert_roi = true;
  input.roi.x = 137;
  input.roi.y = 5;
  input.roi.width = 251 - 137;
  input.roi.height = 130 - 5;
}

int GrabCutDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  cv::Mat mask, bgmodel, fgmodel;
  cv::Mat I = input.images[0];
  mask = cv::Mat::zeros(I.size(), CV_8UC1);
  trace_verbeuse("grabcut...");
  cv::Mat Id;
  cv::pyrDown(I, Id);
  cv::Rect roi2;
  roi2.x = input.roi.x / 2;
  roi2.y = input.roi.y / 2;
  roi2.width = input.roi.width / 2;
  roi2.height = input.roi.height / 2;
  cv::grabCut(Id, mask, roi2, bgmodel, fgmodel, 1, cv::GC_INIT_WITH_RECT);
  //cv::pyrUp(mask, mask);

  trace_verbeuse("masque...");
  output.nout = 2;
  output.images[0] = I.clone();
  output.images[1] = I.clone();


  //Point3_<uchar> *p = O.ptr<Point3_<uchar>>(0,0);
  for(auto y = 0; y < I.rows; y++)
  {
    for(auto x = 0; x < I.cols; x++)
    {
      uint8_t msk = mask.at<uint8_t>(cv::Point(x/2,y/2));
      if((msk == cv::GC_BGD) || (msk == cv::GC_PR_BGD))
      {
        cv::Vec3b &pix = output.images[1].at<cv::Vec3b>(cv::Point(x,y));
        pix[0] = 0;
        pix[1] = 0;
        pix[2] = 0;
      }
    }
  }
  trace_verbeuse("terminé.");

  return 0;
}

