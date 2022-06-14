/** @file appauto.cc

    Copyright 2016 J.A. / http://www.tsdconseil.fr

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



#include "demo-items/appauto.hpp"
#include <iostream>
#include <vector>
#include <assert.h>
#include <stdio.h>
#include <cmath>
#include <opencv2/opencv.hpp>

using namespace cv::ml;


cv::Ptr<StatModel> creation_classifieur_svm(utils::model::Node &model)
{
  float gamma = model.get_attribute_as_float("svm/svm-rbf/gamma");
  if(gamma == 0)
    gamma = 1.0e-10;
  float C = model.get_attribute_as_float("svm/C");
  if  (C <= 0)
    C = 1.0e-10;

  int kernel = model.get_attribute_as_int("svm/kernel");
  int degre = model.get_attribute_as_int("svm/svm-poly/degre");

  //trace_verbeuse("Gamma = %f, C = %f, kernel = %d.", gamma, C, kernel);

  // A FAIRE : créer un classifieur SVM avec noyau RBF
  cv::Ptr<SVM> svm = SVM::create();
  svm->setTermCriteria(cv::TermCriteria(  cv::TermCriteria::Type::MAX_ITER | cv::TermCriteria::Type::EPS,
                                          1000, 1e-3));
  svm->setGamma(gamma);
  svm->setKernel((SVM::KernelTypes) kernel);//SVM::RBF);
  svm->setNu(0.5);
  svm->setP(0.1);
  svm->setC(C);
  svm->setType(SVM::C_SVC); // ou NU_SVC
  svm->setDegree(degre);
  return svm;
}


cv::Ptr<StatModel> creation_classifieur_knn(utils::model::Node &model)
{
  cv::Ptr<KNearest> knearest = KNearest::create();
  knearest->setDefaultK(model.get_attribute_as_int("kppv/K"));
  knearest->setIsClassifier(true);
  //knearest->setAlgorithmType(KNearest::COMPRESSED_INPUT);//KDTREE);//:BRUTE_FORCE);
  knearest->setAlgorithmType(KNearest::BRUTE_FORCE);
  //knearest->setMaxCategories(model.get_attribute_as_int("nclasses"));
  return knearest;
}

cv::Ptr<StatModel> creation_classifieur_adaboost(utils::model::Node &model)
{
  // A FAIRE : créer un classifieur Adaboost
  cv::Ptr<Boost> res = Boost::create();
  res->setMaxCategories(model.get_attribute_as_int("nclasses"));
  res->setMaxDepth(1);
  return res;
}




DemoAppAuto::DemoAppAuto()
{
  props.id = "2d-2classes";
  props.input_min = 0;
  props.input_max = 0;
}


int DemoAppAuto::proceed(OCVDemoItemInput &entree, OCVDemoItemOutput &sortie)
{
  // (1) Génération d'un jeu de données
  uint16_t nclasses = entree.model.get_attribute_as_int("nclasses");
  uint32_t n = entree.model.get_attribute_as_int("napp"),
      ntraits = 2u;
  float bruit = entree.model.get_attribute_as_float("bruit");
  cv::Mat mat_entrainement(cv::Size(ntraits,n), CV_32F);

  uint32_t sx = 512, sy = 512;
  uint32_t sx2 = 32, sy2 = 32;
  cv::Mat O(cv::Size(sx, sy), CV_8UC3, cv::Scalar(255,255,255));
  cv::Mat O2(cv::Size(sx2, sy2), CV_8UC3);

  cv::RNG rng;
  cv::Mat labels(cv::Size(1,n), CV_32S);
  auto ptr = labels.ptr<int>();

  auto *optr = mat_entrainement.ptr<float>();

  uint16_t dl = (5 * 512) / sx;

  infos("Generation jeu de donnees (n = %d, nclasses = %d)...", n, nclasses);


  int n1 = std::floor(std::sqrt(nclasses));
  int n2 = std::ceil(nclasses / n1);

  trace_verbeuse("Partition du plan : %d * %d.", n1, n2);

# define MAX_CLASSES 4
  cv::Scalar couleurs[MAX_CLASSES];
  couleurs[0] = cv::Scalar(255,0,0);
  couleurs[1] = cv::Scalar(0,255,0);
  couleurs[2] = cv::Scalar(0,0,255);
  couleurs[3] = cv::Scalar(255,0,255);

  for(auto i = 0u; i < n; i++)
  {
    uint8_t classe = 0;
    //float x = rng.uniform(0.0f, 2 * 3.1415926f);
    float x = rng.uniform(-1.0f, 1.0f);
    float y = rng.uniform(-1.0f, 1.0f);

    if(nclasses == 2)
    {
      if(y >= std::sin(3.1415926 * x))
        classe = 1;
    }
    else
    {
      // Entre 0 et 1
      float x2 = (x + 1) / 2;
      float y2 = (y + 1) / 2;

      if((x2 < 0) || (x2 >= 1) || (y2 < 0) || (y2 >= 1))
      {
        i--;
        continue;
      }
      classe = std::floor(x2 * n1) + std::floor(y2 * n2) * n1;
    }

    x = x + rng.gaussian(bruit);
    y = y + rng.gaussian(bruit);

    assert(classe < MAX_CLASSES);
    int xi = sx/2 + x * sx / 2;
    int yi = sy/2 + y * sy / 2;

    cv::line(O, cv::Point(xi-dl,yi), cv::Point(xi+dl,yi), couleurs[classe], 1, cv::LINE_AA);
    cv::line(O, cv::Point(xi,yi-dl), cv::Point(xi,yi+dl), couleurs[classe], 1, cv::LINE_AA);

    *optr++ = x;
    *optr++ = y;
    //mat_entrainement.at<float>(i,0) = x;
    //mat_entrainement.at<float>(i,1) = y;
    *ptr++ = classe;
  }
  trace_verbeuse("ok.");

  // (2) Entrainement SVM

  /*float gamma = input.model.get_attribute_as_float("svm/svm-rbf/gamma");
  if(gamma == 0)
    gamma = 1.0e-10;
  float C = input.model.get_attribute_as_float("svm/C");
  if(C <= 0)
    C = 1.0e-10;

  int kernel = input.model.get_attribute_as_int("svm/kernel");
  int degre = input.model.get_attribute_as_int("svm/svm-poly/degre");

  trace_verbeuse("Gamma = %f, C = %f, kernel = %d.", gamma, C, kernel);

  cv::Ptr<SVM> svm = SVM::create();
  svm->setTermCriteria(TermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 1000, 1e-3));
  svm->setGamma(gamma);
  svm->setKernel((SVM::KernelTypes) kernel); //SVM::RBF);
  svm->setNu(0.5);
  svm->setP(0.1);
  svm->setC(C);//0.01);
  svm->setType(SVM::C_SVC);
  svm->setDegree(degre);*/

  cv::Ptr<StatModel> smodel;

  auto sel = input.model.get_attribute_as_int("sel");
  if(sel == 0)
    smodel = creation_classifieur_svm(input.model);
  else if(sel == 1)
    smodel = creation_classifieur_knn(input.model);
  else
    smodel = creation_classifieur_adaboost(input.model);

  trace_verbeuse("Entrainement...");
  smodel->train(mat_entrainement, ROW_SAMPLE, labels);
  trace_verbeuse("Ok.");

  trace_verbeuse("Echantillonnage du plan...");
  cv::Vec3b *o2ptr = O2.ptr<cv::Vec3b>();
  cv::Vec3b c[MAX_CLASSES];
  for(auto i = 0u; i < MAX_CLASSES; i++)
    for(auto j = 0u; j < 3; j++)
      c[i][j] = couleurs[i][j];

  cv::Mat traits(cv::Size(2,1),CV_32F);
  float *tptr = traits.ptr<float>();
  for(auto y = 0u; y < sy2; y++)
  {
    for(auto x = 0u; x < sx2; x++)
    {
      tptr[0] = (((float) x) - sx2/2) / (sx2 / 2);
      tptr[1] = (((float) y) - sy2/2) / (sy2 / 2);
      int res = smodel->predict(traits);
      assert((res < MAX_CLASSES) && (res >= 0));
      *o2ptr++ = c[res];
    }
  }
  trace_verbeuse("Ok.");

  sortie.nout = 2;
  sortie.images[0] = O;
  sortie.names[0]  = "Entrainement";
  sortie.images[1] = O2;
  sortie.names[1]  = "Classication";
  return 0;
}





