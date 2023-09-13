#pragma once

#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "opencv2/imgcodecs.hpp"
#include <opencv2/highgui.hpp>
#include "opencv2/videoio.hpp"
#include "opencv2/imgproc.hpp"
#include <opencv2/core/types_c.h>
#include "opencv2/highgui/highgui.hpp"

#include <opencv2/dnn.hpp>
#include <opencv2/dnn/all_layers.hpp>

typedef void (*webcamCallback)(cv::Mat* frame, bool webCamStatus, void* user);
namespace ImGui
{
class WebCam
{
  public:
    WebCam(webcamCallback frameCallback, bool webCamStatus, void* user);
    ~WebCam();
    bool readFrame();

  private:
    void nnmTracking(cv::Mat& frame);

    cv::VideoCapture         mCaption;
    cv::Mat                  mFrame;
    void*                    mUser;
    webcamCallback           mWebCamCallbackFrame;
    std::vector<std::string> mClassNames;
    cv::dnn::Net             mNeuralNetworkModel;
    bool mWebCamStatus;
};
} // namespace ImGui
