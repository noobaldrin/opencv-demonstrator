/** @file 3d.cc
    @brief Démonstratation relatives à la 3D

   Démonstrations inspirées du livre :
    G. Bradski, Learning OpenCV: Computer vision with the OpenCV library, 2008
    Et aussi d'après l'exemple fourni avec opencv: opencv/sources/samples/cpp/stereo_calib.cpp

    Copyright 2015 J.A. / http://www.tsdconseil.fr

    Project web page: http://www.tsdconseil.fr/log/opencv/demo/index-en.html

    This file is part of OCVDemo.

    OCVDemo is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OCVDemo is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with OCVDemo.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "demo-items/3d.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <cmath>
#include <iostream>

#ifndef M_PI
# define M_PI 3.14159265358979323846
#endif


StereoCalDemo *StereoCalDemo::instance;
StereoCalResultats stereo_cal;


class MatText
{
public:
  MatText(cv::Mat I)
  {
    this->I = I;
    x = 0; y = 25;
    last_dy = 20;
  }

  inline void print(std::string s, ...)
  {
    va_list ap;
    va_start(ap, s);
    printv(s, ap);
    va_end(ap);
  }
private:

  inline void printv(std::string s, va_list ap)
  {
    char tmp_buffer[1000];
    if(vsnprintf(tmp_buffer, 1000, s.c_str(), ap) > 0)
      printi1(std::string(tmp_buffer));
  }

  void printi1(const std::string &s)
  {
    std::string tamp;
    for(auto i = 0u; i < s.length(); i++)
    {
      if(s[i] == '\n')
      {
        printi(tamp);
        tamp = "";
        x = 0;
        y += last_dy;
      }
      else
      {
        char c[2];
        c[0] = s[i];
        c[1] = 0;
        tamp += std::string(c);
      }
    }
    printi(tamp);
  }

  void printi(const std::string &s)
  {
    if(s.size() == 0)
      return;
    int baseLine;
    double tscale = 2.0;
    auto font = cv::FONT_HERSHEY_COMPLEX_SMALL;
    cv::Size si =  getTextSize(s, font, tscale, 1.2, &baseLine);
    putText(I, s,
            cv::Point(x, y),
            font,
            tscale,
            cv::Scalar(255,255,255),
            1.2,
            cv::LINE_AA);
    last_dy = (last_dy > (si.height + 2)) ? last_dy : (si.height + 2);
    x += si.width;
    if(x >= (unsigned int) I.cols)
    {
      x = 0;
      y += si.height + 2;
    }
  }
  cv::Mat I;
  uint32_t x, y;
  int last_dy;
};


StereoFMatDemo::StereoFMatDemo()
{
  props.id = "stereo-fmat";
}

int StereoFMatDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  return 0;
}

StereoCalDemo *StereoCalDemo::get_instance()
{
  return instance;
}

int StereoCalDemo::lookup_corners(cv::Mat &I,
                                  cv::Mat &O,
                                  utils::model::Node &model,
                                  std::vector<cv::Point2f> &coins)
{
  unsigned int bw = input.model.get_attribute_as_int("bw"),
               bh = input.model.get_attribute_as_int("bh");
  cv::Size dim_damier(bw,bh);

  O = I.clone();

  bool found = cv::findChessboardCorners(I, dim_damier, coins,
    cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE);

  //trace_verbeuse("Image %d: trouvé %d coins.", k, coins.size());

# if 0
  // Résolution d'ambiguité si damier carré
  if((coins.size() == bh * bw) && (bh == bw))
  {
    // Si les coins sont alignés verticalement, fait une rotation de 90° (pour lever l'ambiguité)
    auto d = coins[1] - coins[0];
    auto angle = std::fmod(atan2(d.y, d.x) + 2 * M_PI, 2 * M_PI);
    if(angle > M_PI)
      angle -= 2 * M_PI;
    if((std::fabs(angle) > M_PI / 4) && (std::fabs(angle) < 3 * M_PI / 4))
    {
      infos("Image: coins verticaux (%.2f radians) -> transposition.", angle);
      std::vector<cv::Point2f> coins2 = coins;
      for(auto i = 0u; i < bw; i++)
        for(auto j = 0u; j < bw; j++)
          coins[i+j*bw] = coins2[i*bw+j];
    }

    auto signe = coins[0].y - coins[2*bw].y;
    if(signe < 0)
    {
      infos("Image: miroir vertical.");
      std::reverse(coins.begin(), coins.end());
    }

    signe = coins[0].x - coins[2].x;
    if(signe < 0)
    {
      infos("Image: miroir horizontal.");
      for(auto i = 0u; i < bw; i++)
        std::reverse(coins.begin()+i*bw, coins.begin()+(i+1)*bw);
    }
  }
# endif


  if(!found || (coins.size() < 5))
  {
    output.errmsg = utils::langue.get_item("coins-non-trouves");
    avertissement("%s", output.errmsg.c_str());
    avertissement("Coins.size = %d.", coins.size());
    return -1;
  }
  cv::Mat gris;
  cv::cvtColor(I, gris, cv::COLOR_BGR2GRAY);
  cv::cornerSubPix(gris, coins, cv::Size(11,11), cv::Size(-1,-1),
                   cv::TermCriteria(cv::TermCriteria::Type::MAX_ITER + cv::TermCriteria::Type::EPS, 30, 
0.01));

  // Dessin des coins
  cv::drawChessboardCorners(O, dim_damier, cv::Mat(coins), found);

  return 0;
}



// Stereo cal demo :
//  - (1) une démo à partir de vidéo (cu1), une autre démo à partir d'un jeu d'images (cu2) ?
//  - (2) Une démo qui gère les deux
//
//  cu1 : nécessite boutons suivants :
//     b1: détection des coins (ou automatique ?)
//     b2: calibration à partir des paires déjà trouvées
//

StereoCalLiveDemo::StereoCalLiveDemo()
{
  props.id = "stereo-cal-live";
  props.input_min = 2;
  props.input_max = 2;
}

int StereoCalLiveDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  unsigned int bw = input.model.get_attribute_as_int("bw"),
               bh = input.model.get_attribute_as_int("bh");

  output.nout = 2;

  std::vector<cv::Point2f> coins[2];

  auto scal = StereoCalDemo::get_instance();

  for(auto i = 0u; i < 2; i++)
    scal->lookup_corners(input.images[i],
                         output.images[i],
                         input.model,
                         coins[i]);

  if((coins[0].size() == bw * bh) && (coins[1].size() == bw * bh))
  {
    pairs.push_back(coins[0]);
    pairs.push_back(coins[1]);
    infos("Nouvelle paires ajoutée (%d au total).", pairs.size() / 2);
  }

  return 0;
}


//////////////////////////////////////////////////////////
/// STEREO CALIBRATION DEMO                       ////////
//////////////////////////////////////////////////////////
StereoCalDemo::StereoCalDemo()
{
  instance = this;
  props.id = "stereo-cal";
  props.input_min = 2;
  props.input_max = -1; // indefinite number of pairs
  stereo_cal.valide = false;
}

int StereoCalDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  stereo_cal.valide = false;

  output.images[0] = input.images[0];
  cv::Size dim_img = input.images[0].size();


  if(dim_img != input.images[1].size())
  {
    avertissement("%s: images de dim différentes.");
    output.errmsg = "Les 2 images / vidéo doivent être de même résolution.";
    return -1;
  }

  // Nombre de coins (x,y)
  unsigned int bw = input.model.get_attribute_as_int("bw"),
               bh = input.model.get_attribute_as_int("bh");
  cv::Size dim_damier(bw,bh);

  // Coordonnées 3D des coins du damier, dans le repère lié au damier (e.g. Z = 0)
  std::vector<std::vector<cv::Point3f>> points_obj;
  // Coordonnées 2D des coins du damier, dans les deux images

  std::vector<std::vector<cv::Point2f>> points_img[2];

  // Pour l'instant, on ne gère qu'une seule paire d'image
  unsigned int npaires = input.images.size() / 2;

  // Largeur d'un carré du damier (unité physique arbitraire)
  // Ici, 2 cm
  float largeur_carre = input.model.get_attribute_as_float("taille");

  infos("Calibration stéréo...");

  points_obj.resize(npaires);
  points_img[0].resize(npaires);
  points_img[1].resize(npaires);

  output.nout = 2 * npaires;
  for(auto i = 0u; i < 2 * npaires; i++)
    output.names[i] = "Image " + utils::str::int2str(i);

  trace_verbeuse(" 1. Recherche des coins...");
  for(auto p = 0u; p < npaires; p++)
  {
    for(auto k = 0; k < 2; k++)
    {
      if(lookup_corners(input.images[2*p+k], output.images[2*p+k], input.model, points_img[k][p]))
      {
        avertissement("Paire %d, image %d: coins non trouvés.", p, k);
        return -1;
      }
    }
  }

  trace_verbeuse(" 2. Calibration...");

  // Calcul des coordonnées 3D
  for(auto j = 0; j < dim_damier.height; j++ )
    for(auto k = 0; k < dim_damier.width; k++ )
      points_obj[0].push_back(cv::Point3f(k * largeur_carre, j * largeur_carre, 0));
  for(auto j = 1u; j < npaires; j++)
    points_obj[j] = points_obj[0];


  // Initialisation des matrices de caméra (identité)
  for(auto i = 0u; i < 2; i++)
    stereo_cal.matrices_cameras[i] = cv::Mat::eye(3, 3, CV_64F);

  float rms = cv::stereoCalibrate(points_obj, points_img[0], points_img[1],
      stereo_cal.matrices_cameras[0], stereo_cal.dcoefs[0],
      stereo_cal.matrices_cameras[1], stereo_cal.dcoefs[1],
      dim_img,
      stereo_cal.R, stereo_cal.T, stereo_cal.E, stereo_cal.F,
      cv::CALIB_FIX_ASPECT_RATIO |
      cv::CALIB_ZERO_TANGENT_DIST |
      //cv::CALIB_FIX_FOCAL_LENGTH |
      cv::CALIB_SAME_FOCAL_LENGTH |
      cv::CALIB_RATIONAL_MODEL |
      cv::CALIB_FIX_K3 | cv::CALIB_FIX_K4 | cv::CALIB_FIX_K5);

  trace_verbeuse("Erreur RMS calibration: %.3f.", (float) rms);

  cv::stereoRectify(stereo_cal.matrices_cameras[0], stereo_cal.dcoefs[0],
      stereo_cal.matrices_cameras[1], stereo_cal.dcoefs[1],
      dim_img, stereo_cal.R, stereo_cal.T,
      stereo_cal.rectif_R[0], stereo_cal.rectif_R[1],
      stereo_cal.rectif_P[0], stereo_cal.rectif_P[1],
      stereo_cal.Q,
      cv::CALIB_ZERO_DISPARITY,
      1 // 0 : Seulement les pixels valides sont visibles,
      // 1 : tous les pixels (y compris noirs) sont visibles
      );

  // Calcul des LUT pour la rectification de caméra
  for(auto k = 0u; k < 2; k++)
    cv::initUndistortRectifyMap(
        stereo_cal.matrices_cameras[k], stereo_cal.dcoefs[k],
        stereo_cal.rectif_R[k], stereo_cal.rectif_P[k],
        dim_img, CV_32FC1, //CV_16SC2,
        stereo_cal.rmap[k][0], stereo_cal.rmap[k][1]);

  // A FAIRE:
  // - Vérifier qualité de la calibration

  stereo_cal.valide = true;
  return 0;
}


RectificationDemo::RectificationDemo()
{
  props.id = "rectif";
  props.input_min = 2;
  props.input_max = 2;
}


static void rectification(const cv::Mat &I0, const cv::Mat &I1,
                          cv::Mat &O0, cv::Mat &O1)
{
  cv::remap(I0, O0,
            stereo_cal.rmap[0][0],
            stereo_cal.rmap[0][1],
            cv::INTER_LINEAR);
  cv::remap(I1, O1,
            stereo_cal.rmap[1][0],
            stereo_cal.rmap[1][1],
            cv::INTER_LINEAR);
}

int RectificationDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  output.nout = 2;
  output.names[0] = "Image 1";
  output.names[1] = "Image 2";

  if(!stereo_cal.valide)
  {
    output.errmsg = utils::langue.get_item("pas-de-cal-stereo");
    return -1;
  }

  for(auto i = 0u; i < 2; i++)
    rectification(input.images[0], input.images[1],
                  output.images[0], output.images[1]);
  return 0;
}


//////////////////////////////////////////////////////////
/// DEMO LIGNES EPIPOLAIRES                       ////////
//////////////////////////////////////////////////////////
EpiDemo::EpiDemo()
{
  props.id = "epi";
  props.input_min = 2;
  props.input_max = 2;
}

void EpiDemo::raz()
{
  points[0].clear();
  points[1].clear();
}

void EpiDemo::on_mouse(int x, int y, int evt, int wnd)
{
  trace_verbeuse("epi demo souris %d, %d.", x, y);
  cv::Point2f pt(x,y);
  points[wnd].push_back(pt);
  OCVDemoItemRefresh e;
  CProvider<OCVDemoItemRefresh>::dispatch(e);
}

int EpiDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  // L'utilisateur choisi des points sur l'image 1
  // On calcule les épilignes correspondantes sur l'image 2

  if(!stereo_cal.valide)
  {
    output.errmsg = utils::langue.get_item("pas-de-cal-stereo");
    return -1;
  }

  output.nout = 2;
  output.images[0] = input.images[0].clone();
  output.images[1] = input.images[1].clone();

  for(auto k = 0u; k < 2; k++)
  {
    auto npts = points[k].size();

    if(npts == 0)
      continue;

    //Mat F = stereo_cal.F;

    std::vector<cv::Vec3f> epilignes;
    cv::computeCorrespondEpilines(cv::Mat(points[k]), 1 + k, stereo_cal.F, epilignes);

    int sx = input.images[k].cols;

    cv::RNG rng(0);
    for(auto i = 0u; i < npts; i++)
    {
      cv::Scalar color(rng(256),rng(256),rng(256));
      cv::line(output.images[1-k],
               cv::Point(0, -epilignes[i][2]/epilignes[i][1]),
               cv::Point(sx,-(epilignes[i][2]+epilignes[i][0]*sx)/epilignes[i][1]),
               color);
      cv::circle(output.images[k], points[k][i], 3, color, -1, cv::LINE_AA);
    }
  }
  return 0;
}



//////////////////////////////////////////////////////////
/// DISPARITY MAP DEMO                            ////////
//////////////////////////////////////////////////////////
DispMapDemo::DispMapDemo()
{
  props.id = "disp-map";
  props.input_min = 2;
  props.input_max = 2;
}


int DispMapDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  if(input.images.size() != 2)
  {
    output.errmsg = "Disparity map demo: needs 2 input images.";
    return -1;
  }

  cv::Mat I[2];
  I[0] = input.images[0].clone();
  I[1] = input.images[1].clone();


  if(input.model.get_attribute_as_boolean("rectifie"))
  {
    if(!stereo_cal.valide)
    {
      output.errmsg = utils::langue.get_item("pas-de-cal-stereo");
      return -1;
    }
    for(auto i = 0u; i < 2; i++)
      rectification(I[0], I[1], I[0], I[1]);
  }

  cv::Ptr<cv::StereoMatcher> matcher;

  int sel = input.model.get_attribute_as_int("sel");

  if(sel == 0)
  {
    auto sb = cv::StereoBM::create();
    // Parameters below from Opencv/samples/stereo_match.cpp
    sb->setPreFilterCap(31);
    int bsize = input.model.get_attribute_as_int("bm/taille-bloc");
    if((bsize & 1) == 0)
      bsize++;
    sb->setBlockSize(bsize);
    sb->setMinDisparity(0);
    sb->setNumDisparities(((I[0].cols/8) + 15) & -16);
    sb->setTextureThreshold(input.model.get_attribute_as_int("bm/seuil-texture"));
    sb->setUniquenessRatio(input.model.get_attribute_as_int("bm/ratio-unicite"));
    sb->setSpeckleWindowSize(input.model.get_attribute_as_int("bm/speckles-fen"));
    sb->setSpeckleRange(input.model.get_attribute_as_int("bm/speckles-intervalle"));
    sb->setDisp12MaxDiff(input.model.get_attribute_as_int("bm/disp-max-diff"));
    matcher = sb;

    // StereoBM ne supporte que des images en niveaux de gris
    cvtColor(I[0], I[0], cv::COLOR_BGR2GRAY);
    cvtColor(I[1], I[1], cv::COLOR_BGR2GRAY);
  }
  else if(sel == 1)
  {
    // int minDisparity, int numDisparities, int blockSize,
    matcher = cv::StereoSGBM::create(0, 16 * 100, 7);
  }


  cv::Mat disp, disp8, dispf;

  matcher->compute(I[0], I[1], disp);

  // ==> 3 = CV_16S
  trace_verbeuse("disp orig depth = %d, nchn = %d, size = %d * %d.", disp.depth(), disp.channels(), disp.rows, disp.cols);

  // !!!! disp.convertTo(dispf, CV_32F, 1. / 16);
  disp.convertTo(dispf, CV_32F, 1.);

  trace_verbeuse("dispf depth = %d, nchn = %d, size = %d * %d.", dispf.depth(), dispf.channels(), dispf.rows, dispf.cols);

  dispf = dispf * (1.0 / 16);
  /*{
    double minv, maxv;
    cv::minMaxLoc(dispf, &minv, &maxv);
    infos("Min dispf(z) = %lf, max dispf(z) = %lf.", minv, maxv);
    cv::minMaxLoc(disp, &minv, &maxv);
    infos("Min disp(z) = %lf, max disp(z) = %lf.", minv, maxv);
    for(auto ii = 0u; ii < 100; ii++)
    {
      std::cout << "disp: " << disp.at<short>(ii,ii) << std::endl;
      std::cout << "dispf:" << dispf.at<float>(ii,ii) << std::endl;
    }
  }*/


  normalize(disp, disp8, 0, 255, cv::NORM_MINMAX, CV_8U);
  output.images[0] = disp8;
  output.names[0] = utils::langue.get_item("disp-map");


  if(input.model.get_attribute_as_boolean("calc-prof"))
  {
    cv::Mat im3d(disp.size(), CV_32FC3);
    cv::reprojectImageTo3D(dispf, im3d, stereo_cal.Q, true);
    int pairs[2] = {2, 0}; // 2 -> 0
    cv::Mat z(im3d.size(), CV_32F);


    printf("im3d: %d * %d: im3d[..] = ", im3d.cols, im3d.rows);//%f, %f, %f.",
        //);//, im3d.at<Vec3f>(50,50)[0],
//        , im3d.at<Vec3f>(50,50)[0], , im3d.at<Vec3f>(50,50)[0]);
    std::cout << im3d.at<cv::Vec3f>(50,50) << std::endl;
    std::cout << im3d.at<cv::Vec3f>(51,50) << std::endl;
    std::cout << im3d.at<cv::Vec3f>(100,50) << std::endl;

    cv::mixChannels(&im3d, 1, &z, 1, pairs, 1);

    /*double minv, maxv;
    cv::minMaxLoc(z, &minv, &maxv);
    infos("Min(z) = %lf, max(z) = %lf.", minv, maxv);*/

    float zmax = input.model.get_attribute_as_float("zmax");

    auto mask = z > zmax;
    z.setTo(cv::Scalar(zmax), mask);



    z = zmax - z;

    cv::Mat z8;
    normalize(z, z8, 0, 255, cv::NORM_MINMAX, CV_8U);
    output.images[0] = z;
    output.names[0] = utils::langue.get_item("prof");
  }

  return 0;
}



//////////////////////////////////////////////////////////
/// CAMERA CALIBRATION DEMO                       ////////
//////////////////////////////////////////////////////////

CamCalDemo::CamCalDemo()
{
  props.id = "cam-cal";
  output.nout = 3;
  output.names[0] = "Detection des coins";
  output.names[1] = "Distortion corrigee";
}


int CamCalDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  cv::Mat Ig;
  int sel = input.model.get_attribute_as_int("sel");
  int bw = input.model.get_attribute_as_int("bw");
  int bh = input.model.get_attribute_as_int("bh");



  cv::Size board_size(bw,bh);
  std::vector<std::vector<cv::Point2f>> imagePoints;
  std::vector<cv::Point2f> pointbuf;

  cvtColor(input.images[0], Ig, cv::COLOR_BGR2GRAY);


  output.images[2] = cv::Mat(/*Ig.size()*/cv::Size(480,640), CV_8UC3);
  output.images[1] = output.images[2];

  bool trouve;
  if(sel == 0)
  {
    trouve = cv::findChessboardCorners(Ig, board_size, pointbuf,
                                           cv::CALIB_CB_ADAPTIVE_THRESH |
                                           cv::CALIB_CB_FAST_CHECK      | 
                                           cv::CALIB_CB_NORMALIZE_IMAGE);
    // improve the found corners' coordinate accuracy
     if(trouve)
       cv::cornerSubPix(Ig, pointbuf, cv::Size(11,11),
         cv::Size(-1,-1), cv::TermCriteria( cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.1 ));
  }
  else if(sel == 1)
    trouve = cv::findCirclesGrid(Ig, board_size, pointbuf );
  else
    trouve = cv::findCirclesGrid(Ig, board_size, pointbuf, cv::CALIB_CB_ASYMMETRIC_GRID );


  //cvtColor(I, O[0], cv::COLOR_GRAY2BGR);

  cv::Mat Ior = input.images[0].clone();
  output.images[0] = Ior.clone();
  if(trouve)
   cv::drawChessboardCorners(output.images[0], board_size, cv::Mat(pointbuf), trouve);

  trace_majeure("Trouvé %d coins (found = %d).",
      pointbuf.size(), (int) trouve);

  if(trouve)
  {
    cv::Mat distCoeffs;
    cv::Mat cameraMatrix;
    cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
    //if( flags & CALIB_FIX_ASPECT_RATIO )
      //  cameraMatrix.at<double>(0,0) = aspectRatio;
    distCoeffs = cv::Mat::zeros(8, 1, CV_64F);

    float square_size = 1;

    std::vector<std::vector<cv::Point3f> > objectPoints(1);
    std::vector<cv::Point3f> &corners = objectPoints[0];
    if(sel <= 1)
    {
      for( int i = 0; i < board_size.height; i++ )
            for( int j = 0; j < board_size.width; j++ )
                corners.push_back(cv::Point3f(float(j*square_size),
                                          float(i*square_size), 0));
    }
    else
    {
      for( int i = 0; i < board_size.height; i++ )
          for( int j = 0; j < board_size.width; j++ )
              corners.push_back(cv::Point3f(float((2*j + i % 2)*square_size),
                                        float(i*square_size), 0));
    }

    imagePoints.push_back(pointbuf);
    objectPoints.resize(imagePoints.size(),objectPoints[0]);

    std::vector<cv::Mat> rvecs, tvecs;
    // Fonction obsoléte ?
    double rms = cv::calibrateCamera(objectPoints,
                                     imagePoints, Ior.size(),
                    cameraMatrix, distCoeffs, rvecs, tvecs,
                    cv::CALIB_FIX_K4 | cv::CALIB_FIX_K5);
    infos("RMS error reported by calibrateCamera: %g\n", rms);


    cv::undistort(Ior, output.images[1], cameraMatrix, distCoeffs);

    cv::Size sz = Ior.size();
    sz.height = sz.width = cv::max(sz.width, sz.height);
    sz.height = sz.width = cv::max(sz.width, 500);

    output.images[2] = cv::Mat::zeros(sz, CV_8UC3);

    double fovx, fovy, focal, ar;
    cv::Point2d ppoint;
    cv::calibrationMatrixValues(cameraMatrix, Ior.size(), 1, 1, fovx, fovy, focal, ppoint, ar);


    MatText mt(output.images[2]);

    std::stringstream str;

    mt.print("Camera matrix:\n");
    str << cameraMatrix << "\n";
    mt.print("Focal : %.2f, %.2f, %.2f\n", (float) fovx, (float) fovy, (float) focal);
    mt.print("Point principal: %.2f, %.2f\n", (float) ppoint.x, (float) ppoint.y);

    mt.print(str.str());

    /*Mat tmp = output.images[2].clone();
    cv::namedWindow("essai", CV_WINDOW_NORMAL);
    cv::imshow("essai", tmp);
    cv::waitKey();*/
    //bool ok = checkRange(cameraMatrix) && checkRange(distCoeffs);
    //totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints,
    //            rvecs, tvecs, cameraMatrix, distCoeffs, reprojErrs);
  }

  return 0;
}

DemoLocalisation3D::DemoLocalisation3D()
{
  props.id = "suivi-balle-3d";
  props.input_min = 2;
  props.input_max = 2;
}

int DemoLocalisation3D::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  if(!stereo_cal.valide)
  {
    output.errmsg = utils::langue.get_item("pas-de-cal-stereo");
    return -1;
  }

  cv::Point balle_loc[2];

  output.nout = 3;

  for(auto k = 0u; k < 2; k++)
  {
    cv::Mat &I = input.images[k];
    cv::Mat tsv, masque;
    cvtColor(I, tsv, cv::COLOR_BGR2HSV);
    inRange(tsv, cv::Scalar(18, 100, 128),
            cv::Scalar(30, 255, 255), masque);
    cv::Mat dst;
    cv::distanceTransform(masque, dst,cv::DistanceTypes::DIST_L2, 3);
    cv::minMaxLoc(dst, nullptr, nullptr, nullptr, &balle_loc[k]);

    infos("Balle loc[%d]: %d, %d.", balle_loc[k].x, balle_loc[k].y);

    cv::Mat O = input.images[k].clone();

    // Position de la balle détectée
    cv::line(O, balle_loc[0] - cv::Point(5,0), balle_loc[0] + cv::Point(5,0),
             cv::Scalar(0,0,255), 1);
    cv::line(O, balle_loc[0] - cv::Point(0,5), balle_loc[0] + cv::Point(0,5),
             cv::Scalar(0,0,255), 1);
    output.images[k] = O;
  }

  //Mat O = input.images[0].clone();

  //p1(1) = Point2f(balle_loc[1].x,balle_loc[1].y);

  // Calcul de la position en 3D
  cv::Point2f loc_rectifiee[2];
  for(auto k = 0u; k < 2; k++)
  {
    cv::Mat_<cv::Point2f> p1(1,1), p2(1,1);
    p1(0) = cv::Point2f(balle_loc[0].x,balle_loc[0].y);
    cv::undistortPoints(p1,
         p2,
         stereo_cal.matrices_cameras[k],
         stereo_cal.dcoefs[k],
         stereo_cal.rectif_R[k],
         stereo_cal.rectif_P[k]);
    loc_rectifiee[k] = p2(0);
  }

  // 2D vers 3D
  float y_dist = loc_rectifiee[0].y - loc_rectifiee[1].y;
  float x_dist = loc_rectifiee[0].x - loc_rectifiee[1].x;

  infos("X dist = %.2f, Y dist = %.2f.", x_dist, y_dist);

  cv::Point3f p0, p1;

  p0.x = loc_rectifiee[0].x;
  p0.y = loc_rectifiee[0].y;
  p0.z = x_dist;
  //p0[3] = 0;

  cv::Mat_<cv::Point3f> mp0(1,1), mp1(1,1);

  mp0(0) = p0;
  cv::perspectiveTransform(mp0, mp1, stereo_cal.Q); // ????
  p1 = mp1(0);

  infos("Coordonnées 3D = %.2f, %.2f, %.2f.", p1.x, p1.y, p1.z);


  /*Mat O;
  cv::remap(input.images[0], O,
            stereo_cal.rmap[0][0],
            stereo_cal.rmap[0][1],
            cv::INTER_LINEAR);*/
  cv::Mat O = input.images[0];

  // Juste un test : reprojection du point 3d vers l'image 0
  // ==> On devrait avoir à peu près les coordonnées du point détecté.
  cv::Mat_<cv::Point2f> mp2; // Coordonnées dans l'image 0
  cv::projectPoints(mp1,
          stereo_cal.R, stereo_cal.T,
          stereo_cal.matrices_cameras[0],
          stereo_cal.dcoefs[0],
          mp2);
  infos("Balle reprojetee sur img0 : %.2f, %.2f.", mp2(0).x, mp2(0).y);

  output.images[2] = O;

  return 0;
}


