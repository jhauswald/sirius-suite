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
 * DNN for speech recognition decoding
 *
 * @author: Yiping Kang
 * @contact: ypkang@umich.edu
 */

#include <assert.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <cmath>
#include <glog/logging.h>

#include "caffe/caffe.hpp"

#include "../../utils/memoryman.h"
#include "../../utils/timer.h"
#include "../../utils/memoryman.h"

#define FEATURE_VEC_SIZE 440
#define PROB_VEC_SIZE 1706
#include "ittnotify.h"

#define FEATURE_VEC_SIZE 440  // Number of floats in one input feature vector
#define PROB_VEC_SIZE 1706  // Number of floats in one output probability vector

using caffe::Blob;
using caffe::Caffe;
using caffe::Net;
using caffe::Layer;

using namespace std;

inline bool isEqual(float x, float y) {
  const float epsilon = pow(10, -4);
  return abs(x - y) <= epsilon;
}

void dnn_fwd(float* in, int in_size, float* out, int out_size,
             Net<float>* net) {
  vector<Blob<float>*> in_blobs = net->input_blobs();
  vector<Blob<float>*> out_blobs;

  float loss;

  // Perform DNN forward pass
  in_blobs[0]->set_cpu_data(in);
  out_blobs = net->ForwardPrefilled(&loss);

  assert(out_size == out_blobs[0]->count() && "Output size not expected.");

  memcpy(out, out_blobs[0]->cpu_data(), sizeof(float) * out_size);
}

int dnn_init(Net<float>* net, string weights) {
  net->CopyTrainedLayersFrom(weights);
  return 0;
}

void load_features(float* in, string feature_file) {
  int idx = 0;

  // Read the feature in
  ifstream featFile(feature_file.c_str(), ifstream::in);
  string line;
  getline(featFile, line);  // Get rid of the first line
  while (getline(featFile, line)) {
    istringstream iss(line);
    float temp;
    while (iss >> temp) {
      in[idx] = temp;
      idx++;
    }
  }
}

int main(int argc, char** argv) {
  __itt_pause();
  if (argc < 6) {
    fprintf(stderr, "[ERROR] Input file required.\n\n");
    fprintf(stderr, "Usage: %s [THREADS] [NETWORK] [WEIGHTS] [NUM PASSES] [INPUT FEATURES]\n\n",
            argv[0]);
    exit(0);
  }

  // turn off caffe's logging
  FLAGS_minloglevel = google::ERROR;

  STATS_INIT("kernel", "dnn_automatic_speech_recognition");
  PRINT_STAT_STRING("abrv", "dnn-asr");

  int THREADS = atoi(argv[1]);
  string network(argv[2]);
  string weights(argv[3]);
  int PASSES = atoi(argv[4]);
  string features(argv[5]);

  openblas_set_num_threads(THREADS);
  PRINT_STAT_INT("threads", THREADS);
  PRINT_STAT_STRING("model", network.c_str());
  PRINT_STAT_STRING("weights", weights.c_str());

  // Load DNN model
  Net<float>* dnn = new Net<float>(network);
  dnn_init(dnn, weights);

  // Read in feature from file
  ifstream inFile(features.c_str(), ifstream::in);
  int feat_cnt = count(istreambuf_iterator<char>(inFile),
                       istreambuf_iterator<char>(), '\n') - 1;
  int in_size = feat_cnt * FEATURE_VEC_SIZE;
  int out_size = feat_cnt * PROB_VEC_SIZE;
  float** feature_input = (float**)sirius_malloc(PASSES * sizeof(float));
  float** dnn_output = (float**)sirius_malloc(sizeof(float) * PASSES);

  // load first set of features
  feature_input[0] = (float*)sirius_malloc(feat_cnt * FEATURE_VEC_SIZE * sizeof(float));
  dnn_output[0] = (float*)sirius_malloc(sizeof(float) * out_size);
  load_features(feature_input[0], features);

  PRINT_STAT_INT("in_features", feat_cnt);
  PRINT_STAT_INT("fwd_passes", PASSES);

  // copy first features into other arrays for more forward passes if PASSES > 0
  for(int i = 1; i < PASSES; ++i) {
    feature_input[i] = (float*)sirius_malloc(feat_cnt * FEATURE_VEC_SIZE * sizeof(float));
    dnn_output[i] = (float*)sirius_malloc(sizeof(float) * out_size);
    for (int k = 0; k < feat_cnt * FEATURE_VEC_SIZE; k++) {
      feature_input[i][k] = feature_input[0][k];
    }
  }

  // Perform dnn forward pass

  tic();

  for(int i = 0; i < PASSES; ++i)
    dnn_fwd(feature_input[i], in_size, dnn_output[i], out_size, dnn);
  PRINT_STAT_DOUBLE("pthread_dnn-asr", toc());
  __itt_pause();

  STATS_END();

#if TESTING
  std::string result_file = "../input/correct.out";
  // Read in features from file
  // First need to detect how many feature vectors
  ifstream outFile(result_file.c_str(), ifstream::in);
  int out_cnt = count(istreambuf_iterator<char>(outFile),
                       istreambuf_iterator<char>(), '\n') - 1;
  float* correct_out = (float*)sirius_malloc(out_cnt * PROB_VEC_SIZE * sizeof(float));
  load_features(correct_out, result_file);

  // First check that the numbers of vectors are same
  assert(out_cnt == feat_cnt);

  // Then check that the number actually agrees
  for (int k = 0; k < PASSES; k++) {
    for (int i = 0; i < out_size; i++) {
      assert(isEqual(correct_out[i], dnn_output[k][i]));
    }
  }
  sirius_free(correct_out);

#endif

  // for(int i = 0; i < PASSES; ++i) {
  //   sirius_free(feature_input[i]);
  //   sirius_free(dnn_output[i]);
  // }
  //
  // sirius_free(feature_input);
  // sirius_free(dnn_output);

  return 0;
}
