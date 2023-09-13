#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>
#include "opencv2/imgcodecs.hpp"
#include <opencv2/highgui.hpp>
#include "opencv2/videoio.hpp"
#include "opencv2/imgproc.hpp"
#include <opencv2/core/types_c.h>
#include "opencv2/highgui/highgui.hpp"

typedef void (*webcamCallback)(cv::Mat* frame, void* user);
namespace ImGui
{
class WebCam
{
  public:
    WebCam(webcamCallback frameCallback, void* user);
    ~WebCam();
    bool readFrame();

  private:
    cv::VideoCapture mCaption;
    cv::Mat          mFrame;
    void*            mUser;
    webcamCallback   mWebCamCallbackFrame;
};
} // namespace ImGui
