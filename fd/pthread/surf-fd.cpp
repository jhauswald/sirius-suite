/*
 *  Copyright (c) 2015, University of Michigan.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/**
 * TODO:
 *
 * @author: Johann Hauswald
 * @contact: jahausw@umich.edu
 */

#include <assert.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <pthread.h>

#include "../../utils/pthreadman.h"
#include "../../utils/timer.h"

#include "opencv2/core/core.hpp"
#include "opencv2/core/types_c.h"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/gpu.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/stitching/stitcher.hpp"

using namespace cv;
using namespace std;

int NTHREADS;
int OVERLAP;

DescriptorExtractor *extractor = new SurfDescriptorExtractor();
int iterations;
vector<Mat> segs;
vector<vector<KeyPoint> > keys;
vector<Mat> descs;

vector<vector<KeyPoint> > read_keys(char *input) {
  ifstream f(input);
  vector<vector<KeyPoint> > k;
  float x=0, y=0, size=0, angle=0, response=0;
  int octave=0, class_id=0;
  int num_kps=0;

  for(int i = 0; i < segs.size(); ++i) {
    vector<KeyPoint> keys;
    f >> num_kps;
    int read = 0;
    while(read < num_kps) {
      f >> x >> y >> size >> angle >> response >> octave >> class_id;
      KeyPoint kp;
      kp.pt.x = x;
      kp.pt.y = y;
      kp.size = size;
      kp.angle = angle;
      kp.response = response;
      kp.octave = octave;
      kp.class_id = class_id;
      keys.push_back(kp);
      ++read;
    }
    k.push_back(keys);
  }

  return k;
}

vector<Mat> segment(const Mat &img) {
  int height_inc = img.size().height / NTHREADS;
  int width_inc = img.size().width / NTHREADS;

  vector<Mat> segments;

  for (int r = 0; r < img.size().height; r += height_inc) {
    for (int c = 0; c < img.size().width; c += width_inc) {
      int rectoverlap = OVERLAP * 2;
      Rect roi;
      roi.x = c;
      roi.y = r;
      roi.width = (c + width_inc > img.size().width) ? (img.size().width - c)
                                                     : width_inc;
      roi.height = (r + height_inc > img.size().height)
                       ? (img.size().height - r)
                       : height_inc;
      if (r == 0) {  // top row
        if (c != 0)  // top row
          roi.x -= OVERLAP;
        if (c == 0 || c == img.size().width - width_inc)  // corners
          roi.width += OVERLAP;
        else
          roi.width += rectoverlap;
        roi.height += OVERLAP;
      } else if (c == 0) {  // first column
        roi.y -= OVERLAP;
        if (r == img.size().height - height_inc)  // bot left corner
          roi.height += OVERLAP;
        else
          roi.height += rectoverlap;  // any first col segment
        roi.width += OVERLAP;         // cst width
      } else if (r == img.size().height - height_inc) {  // bot row
        roi.x -= OVERLAP;
        roi.y -= OVERLAP;
        if (c == img.size().width - width_inc)  // bot right corner
          roi.width += OVERLAP;
        else
          roi.width += rectoverlap;
        roi.height += OVERLAP;
      } else if (c == img.size().width - width_inc) {  // last col, corners
                                                       // already
                                                       // acnumber_desced for
        roi.x -= OVERLAP;
        roi.y -= OVERLAP;
        roi.width += OVERLAP;
        roi.height += rectoverlap;
      } else {  // any middle image
        roi.x -= OVERLAP;
        roi.y -= OVERLAP;
        roi.width += rectoverlap;
        roi.height += rectoverlap;
      }
      Mat imgroi(img, roi);
      segments.push_back(imgroi);
    }
  }

  return segments;
}

Mat exec_desc(const Mat &img, vector<KeyPoint> keypoints) {
  Mat descriptors;

  extractor->compute(img, keypoints, descriptors);

  descriptors.convertTo(descriptors, CV_32F);

  return descriptors;
}

void *desc_thread(void *tid) {
  int start, *mytid, end;
  mytid = (int *)tid;
  start = (*mytid * iterations);
  end = start + iterations;

  for (int i = start; i < end; ++i) descs[i] = exec_desc(segs[i], keys[i]);

  return NULL;
}

int main(int argc, char **argv) {
  if (argc < 4) {
    fprintf(stderr, "[ERROR] Invalid arguments provided.\n\n");
    fprintf(stderr, "Usage: %s [NUMBER OF THREADS] [OVERLAP] [INPUT IMAGE] [INPUT KEYPOINTS]\n\n",
            argv[0]);
    exit(0);
  }

  STATS_INIT("kernel", "pthread_feature_description");
  PRINT_STAT_STRING("abrv", "pthread_fd");

  NTHREADS = atoi(argv[1]);
  OVERLAP = atoi(argv[2]);
  PRINT_STAT_INT("threads", NTHREADS);

  // read in test image
  Mat img = imread(argv[3], CV_LOAD_IMAGE_GRAYSCALE);
  if (img.empty()) {
    printf("image not found\n");
    exit(-1);
  }

  // make tiles
  segs = segment(img);

  // read in test keypoints
  keys = read_keys(argv[4]);
  int total = 0;
  for(int i = 0; i < keys.size(); ++i)
    total += keys[i].size();
  PRINT_STAT_INT("in_keypoints", total);

  tic();
  int tids[NTHREADS];
  pthread_t threads[NTHREADS];
  pthread_attr_t attr;
  iterations = (keys.size() / NTHREADS);
  descs.resize(keys.size());
  sirius_pthread_attr_init(&attr);
  sirius_pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  tic();
  sirius_pthread_attr_init(&attr);
  sirius_pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  for (int i = 0; i < NTHREADS; i++) {
    tids[i] = i;
    sirius_pthread_create(&threads[i], &attr, desc_thread, (void *)&tids[i]);
  }

  for (int i = 0; i < NTHREADS; i++) sirius_pthread_join(threads[i], NULL);
  PRINT_STAT_DOUBLE("pthread_fd", toc());

  STATS_END();

#ifdef TESTING
  FILE *f = fopen("../input/surf-fd.pthread", "w");

  int number_desc = 0;
  for (int i = 0; i < descs.size(); ++i) number_desc += descs[i].size().height;

  fprintf(f, "number of descriptors: %d\n", number_desc);

  fclose(f);
#endif

  // Clean up
  delete extractor;

  return 0;
}
