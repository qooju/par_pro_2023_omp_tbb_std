// Copyright 2023 Bykova Aleksandra
#ifndef MODULES_TASK_2_BYKOVA_A_INCREASING_THE_CONTRAST_OMP_INCREASING_THE_CONTRAST_OMP_H_
#define MODULES_TASK_2_BYKOVA_A_INCREASING_THE_CONTRAST_OMP_INCREASING_THE_CONTRAST_OMP_H_
#include <vector>

std::vector<float> getGaussKernel(int radius, float sigma);
float calcNewPixColor(const int x, const int y, const int width,
                      const int height, const std::vector<float>& kernel,
                      int radius, const std::vector<float>& img);
std::vector<float> getSequentialOperations(const int width, const int height,
                                           const std::vector<float>& kernel,
                                           const std::vector<float>& img);

std::vector<float> getParallelOperations(const int width, const int height,
                                         const std::vector<float>& kernel,
                                         const std::vector<float>& img,
                                         int num_th = 4);

std::vector<float> getRandomImage(int width, int height);

#endif  // MODULES_TASK_2_BYKOVA_A_INCREASING_THE_CONTRAST_OMP_INCREASING_THE_CONTRAST_OMP_H_