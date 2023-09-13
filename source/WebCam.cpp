#include "WebCam.h"

using namespace ImGui;

WebCam::WebCam(webcamCallback frameCallback, void* user)
    : mCaption(0)
    , mFrame{}
    , mUser(user)
    , mWebCamCallbackFrame(frameCallback)
{
    if (mCaption.isOpened() == false)
    {
        std::cout << "Cannot open the video camera" << std::endl;
        std::cin.get();
    }
}

WebCam::~WebCam()
{
    mFrame               = {};
    mUser                = NULL;
    mWebCamCallbackFrame = NULL;
}

bool WebCam::readFrame()
{
    bool status = true;
    status      = mCaption.read(mFrame);
    cv::Mat I_YUV = {};
    cv::cvtColor(mFrame, I_YUV, cv::COLOR_BGR2YCrCb);
    cv::cvtColor(I_YUV, mFrame, cv::COLOR_YCrCb2RGB);
    if (mWebCamCallbackFrame)
    {
        mWebCamCallbackFrame(&mFrame, mUser);
    }
    return status;
}
