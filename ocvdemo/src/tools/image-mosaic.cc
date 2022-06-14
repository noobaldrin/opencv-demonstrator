/** @file image-mosaic.cc
 *  @brief Affichage de plusieurs images sur une même fenêtre

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

#include "tools/image-mosaique.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "ocvdemo.hpp"

// Adapté d'après http://code.opencv.org/projects/opencv/wiki/DisplayManyImages

ImageMosaique::ImageMosaique()
{
  preserve_ratio_aspet = false;
  callback_init_ok = false;
}

void mouse_callback(int event, int x, int y, int flags, void *param)
{
  ((ImageMosaique *) param)->mouse_callback(event, x, y, flags);
}

void ImageMosaique::mouse_callback(int event, int x, int y, int flags)
{
  //infos("imosaic mouse callback x = %d, y = %d.", x, y);

  // Conversion vers coordoonnées image i
  for(auto j = 0u; j < img_pos.size(); j++)
  {
    cv::Rect r = img_pos[j];
    if((x >= r.x) && (y >= r.y) && (x < r.x + r.width) && (y < r.y + r.height))
    {
      int x2 = ((x - r.x) * img_sizes[j].width) / r.width;
      int y2 = ((y - r.y) * img_sizes[j].height) / r.height;
      OCVMouseEvent me;
      me.image = j;
      me.event = event;
      me.x = x2;
      me.y = y2;
      me.flags = flags;
      dispatch(me);
    }
  }
}

void ImageMosaique::update_image(int index, const cv::Mat &img)
{
  trace_verbeuse("ImageMosaique::update_image");
  mutex.lock();
  cv::Rect rect(img_pos[index].x, img_pos[index].y,
      img_pos[index].width, img_pos[index].height);
  cv::Mat roi(disp_img, rect);


  cv::Mat im = img.clone();

  while((im.cols >= 2 * roi.size().width) || (im.rows >= 2 * roi.size().height))
    pyrDown(im, im);
  while((im.cols < roi.size().width / 2) && (im.rows < roi.size().height / 2))
    pyrUp(im, im);

  cv::resize(im, roi, roi.size());
  cv::imshow(title.c_str(), disp_img);
  mutex.unlock();
}

int ImageMosaique::show_multiple_images(std::string title,
    std::vector<cv::Mat> lst,
    std::vector<std::string> titles)
{
  mutex.lock();
  this->title = title;
  cv::Mat img;
  unsigned int nimages = lst.size();

  int sizex, sizey;
  int i;
  int m, n;
  int x, y;

  // w - Maximum number of images in a row
  // h - Maximum number of images in a column
  int w, h;
  // scale - How much we have to resize the image
  float scale;
  //int max;

  bool show_deco = true;

  img_pos.clear();
  img_sizes.clear();

  /*if(nimages < 0)
  {
    erreur("%s: Number of arguments too small.", __func__);
    mutex.unlock();
    return -1;
  }*/

  if((nimages > 0) && (lst[0].cols <= 0))
  {
    erreur("%s: First image empty.", __func__);
    mutex.unlock();
    return -1;
  }

  if(nimages == 0)
  {
    w = h = 1;
    sizex = sizey = 1;
  }
  else if (nimages == 1)
  {
    w = h = 1;
    sizex = lst[0].cols;
    sizey = lst[0].rows;
    show_deco = false;
  }
  else if (nimages == 2)
  {
    w = 2; h = 1;
    sizex = 500;
    sizey = 500;
  }
  else if (nimages == 3)
  {
    w = 3; h = 1;
    sizex = 640;//350;
    sizey = 480;//350;
  }
  else if (nimages == 4)
  {
    w = 2; h = 2;
    sizex = sizey = 350;
  }
  else if (nimages == 5 || nimages == 6)
  {
    w = 3; h = 2;
    sizex = sizey = 250;
  }
  else if (nimages == 7 || nimages == 8)
  {
    w = 4; h = 2;
    sizex = sizey = 250;
  }
  else
  {
    w = (int) ceil(sqrt(nimages));
    h = (int) ceil(((float) nimages) / w);
    //w = 4; h = 3;
    sizex = sizey = 100;
  }

  if(nimages > 1)
    sizey = (sizex * lst[0].rows) / lst[0].cols;

  uint16_t W = /*100*/20 + (20+sizex)*w;
  uint16_t H = 20 + (sizey+30)*h;

  if(nimages <= 1)
  {
    W = sizex;
    H = sizey;
  }

  infos("show_multiple_images (sizex = %d, sizey = %d, w = %d, h = %d, W = %d, H = %d)",
      sizex, sizey, w, h, W, H);

  // Create a new 3 channel image
  disp_img.create(cv::Size(W, H), CV_8UC3);
  disp_img.setTo(cv::Scalar(0));


  // Loop for nArgs number of arguments
  for (i = 0, m = 20, n = 20; i < (int) nimages; i++, m += (20 + sizex))
  {
    // Get the Pointer to the IplImage
    img = lst[i];
    auto sz = img.size();
    x = sz.width;
    y = sz.height;

    float scalex = ((float) x) / sizex;
    float scaley = ((float) y) / sizey;
    scale = std::max(scalex,scaley);

    // Used to Align the images
    if(((i % w) == 0) && (m != 20))
    {
      m  = 20;
      n += 30 + sizey;
    }

    if(nimages == 1)
    {
      m = 0; n = 0;
      scale = 1;
      disp_img = img;
    }

    // Set the image ROI to display the current image
    cv::Rect rect(m, n, (int)(x/scale), (int)(y/scale));

    //trace_verbeuse("di: %d*%d, r:%d,%d,%d,%d.", disp_img.cols, disp_img.rows, rect.x, rect.y, rect.width, rect.height);
    cv::Mat roi(disp_img, rect);

    if(nimages > 1)
      // Resize the input image and copy the it to the Single Big Image
    {
     cv::Mat im = img.clone();

      while((im.cols >= 2 * roi.size().width) || (im.rows >= 2 * roi.size().height))
        pyrDown(im, im);
      while((im.cols <= roi.size().width / 2) && (im.rows <= roi.size().height / 2))
        pyrUp(im, im);

      cv::resize(im, roi, roi.size());
    }



    img_pos.push_back(rect);
    img_sizes.push_back(cv::Size(img.cols, img.rows));

    if(show_deco && (titles[i].size() > 0))
    {
      std::string texte = titles[i];
      int baseLine;
      double tscale = 1.0;
      auto font = cv::FONT_HERSHEY_COMPLEX_SMALL;
      cv::Size si =  getTextSize(texte, font, tscale, 1.2, &baseLine);
      ///int dx = (x/scale);
      int xc = m + (x/(2*scale));
      putText(disp_img, texte,
          cv::Point(xc - si.width / 2, n + y / scale + 1.5 * si.height),
          font,
          tscale,
          cv::Scalar(255,255,255),
          1.2,
          cv::LINE_AA);
    }
  }

  // Create a new window, and show the Single Big Image

  trace_verbeuse("namedWindow...");

  auto model = OCVDemo::get_instance()->get_modele_global();

  if(model.get_attribute_as_boolean("force-taille-sortie"))
  {
    trace_verbeuse("Forçage taille...");
    cv::namedWindow(title.c_str(), cv::WINDOW_KEEPRATIO | cv::WINDOW_NORMAL);
    cv::resizeWindow(title.c_str(),
                     model.get_attribute_as_int("sortie-sx"),
                     model.get_attribute_as_int("sortie-sy"));
  }
  else
  {
    trace_verbeuse("Taille auto...");
    if(nimages == 1)
    {
      trace_majeure("KEEP RATIO");
      cv::namedWindow(title.c_str(), cv::WINDOW_KEEPRATIO | cv::WINDOW_NORMAL);
      cv::resizeWindow(title.c_str(), lst[0].cols, lst[0].rows);
    }
    else
      cv::namedWindow(title.c_str(), 1);


    //cv::moveWindow(title.c_str(), 0,0);
  }

  trace_verbeuse("imwrite");
  //cv::imwrite("./essai.jpg", disp_img); // OK
  trace_verbeuse("imshow: [%s], %d * %d", title.c_str(), disp_img.cols, disp_img.rows);
  cv::imshow(title.c_str(), disp_img);

  //cv::moveWindow(title.c_str(), 0, 0);


  if(!callback_init_ok)
  {
    trace_verbeuse("cbinit");
    cv::setMouseCallback(title, ::mouse_callback, this);
    callback_init_ok = true;
  }


  infos("done.");
  mutex.unlock();
  return 0;
}
