/** @file color-demo.cc
 *  @brief Gestion des espaces de couleurs

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

#include "demo-items/espaces-de-couleurs.hpp"




HSVDemo::HSVDemo()
{
  props.id = "hsv";
  output.names[0] = "Couleur";
}

int HSVDemo::proceed(OCVDemoItemInput &input, OCVDemoItemOutput &output)
{
  int T = input.model.get_attribute_as_int("T");
  int S = input.model.get_attribute_as_int("S");
  int V = input.model.get_attribute_as_int("V");

  auto I = input.images[0];
  int sx = I.cols;
  int sy = I.rows;

  //I.setTo(cv::Scalar(0,0,0));
  I = cv::Mat::zeros(cv::Size(sx,sy),CV_8UC3);

  cv::Mat tsv = cv::Mat::zeros(cv::Size(sx,sy),CV_8UC3);
  tsv.setTo(cv::Scalar(T,S,V));
  cvtColor(tsv, output.images[0], cv::COLOR_HSV2BGR);
  return 0;
}

DemoBalleTennis::DemoBalleTennis()
{
  props.id = "tennis";
}

int DemoBalleTennis::proceed(OCVDemoItemInput &input,
                             OCVDemoItemOutput &output)
{
  cv::Mat &I = input.images[0];

  // Solutions :
  // (1) Mahalanobis distance, puis seuillage
  // (2) Projection arrière d'histogramme
  // (3) Seuillage simple sur teinte / saturation

  cv::Mat tsv, masque;
  cvtColor(I, tsv, cv::COLOR_BGR2HSV);
  inRange(tsv, cv::Scalar(input.model.get_attribute_as_int("T0"),
      input.model.get_attribute_as_int("S0"),
      input.model.get_attribute_as_int("V0")),
      cv::Scalar(input.model.get_attribute_as_int("T1"),
          input.model.get_attribute_as_int("S1"),
          input.model.get_attribute_as_int("V1")),
          masque);

  cv::Mat dst;
  cv::Point balle_loc;
  cv::distanceTransform(masque, dst, cv::DistanceTypes::DIST_L2, 3);
  cv::minMaxLoc(dst, nullptr, nullptr, nullptr, &balle_loc);

  cv::Mat O = input.images[0].clone();

  // Position de la balle détectée
  cv::line(O, balle_loc - cv::Point(5,0), balle_loc + cv::Point(5,0),
           cv::Scalar(0,0,255), 1);
  cv::line(O, balle_loc - cv::Point(0,5), balle_loc + cv::Point(0,5),
           cv::Scalar(0,0,255), 1);

  output.images[0] = O;

  return 0;
}

