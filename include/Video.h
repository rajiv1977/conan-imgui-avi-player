#pragma once

#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/types_c.h>
#include <opencv2/dnn.hpp>
#include <opencv2/dnn/all_layers.hpp>
#include <opencv2/highgui.hpp>
#include <string>

namespace Gui
{
class Video
{
  public:
    Video(const std::string& src);
    ~Video();

  private:
    void nnmTracking(cv::Mat& frame);
    bool readFrame();
    void showVideo();
    void scaleImage(int pos, void*);

    cv::VideoCapture         mCaption;
    std::vector<std::string> mClassNames;
    cv::Mat                  mFrame;
    double                   mFps;
    cv::dnn::Net             mNeuralNetworkModel;
    std::string              mWindowName;
    int                      mMaxScaleUp;
    int                      mScaleFactor;
};
} // namespace Gui
