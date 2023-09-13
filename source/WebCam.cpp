#include "WebCam.h"
#include <filesystem>


using namespace ImGui;

WebCam::WebCam(webcamCallback frameCallback, bool webCamStatus, void* user)
    : mCaption(0)
    , mFrame{}
    , mUser(user)
    , mWebCamCallbackFrame(frameCallback)
    , mClassNames{}
    , mNeuralNetworkModel{}
    , mWebCamStatus(webCamStatus)
{
    if (mWebCamStatus)
    {
        // Web cam status
        if (mCaption.isOpened() == false)
        {
            std::cout << "Cannot open the video camera" << std::endl;
            std::cin.get();
        }

        std::filesystem::path cwd = std::filesystem::current_path();
        std::cout << cwd << std::endl;

        // Classification
        std::ifstream ifs(std::string("object_detection_classes.txt").c_str());
        std::string   line;
        while (getline(ifs, line))
        {
            mClassNames.push_back(line);
        }

        // load the neural network model
        mNeuralNetworkModel = cv::dnn::readNet("inference_graph.pb", "ssd_mobilenet.pbtxt.txt", "TensorFlow");
    }
}

WebCam::~WebCam()
{
    mFrame               = {};
    mUser                = NULL;
    mWebCamCallbackFrame = NULL;
    mClassNames.clear();
    mCaption.release();
    mWebCamStatus = false;
}

bool WebCam::readFrame()
{
    bool status   = true;
    status        = mCaption.read(mFrame);
    cv::Mat I_YUV = {};
    cv::cvtColor(mFrame, I_YUV, cv::COLOR_BGR2YCrCb);
    cv::cvtColor(I_YUV, mFrame, cv::COLOR_YCrCb2RGB);
    nnmTracking(mFrame);
    if (mWebCamCallbackFrame)
    {
        mWebCamCallbackFrame(&mFrame, mWebCamStatus, mUser);
    }
    return status;
}

// neural network model based tracking
void WebCam::nnmTracking(cv::Mat& frame)
{
    // create blob from image
    cv::Mat blob = cv::dnn::blobFromImage(frame, 1.0, cv::Size(300, 300), cv::Scalar(127.5, 127.5, 127.5), true, false);

    // Set input to the model
    mNeuralNetworkModel.setInput(blob);

    // forward pass through the model to carry out the detection
    cv::Mat output = mNeuralNetworkModel.forward();

    cv::Mat detectionMat(output.size[2], output.size[3], CV_32F, output.ptr<float>());

    for (int i = 0; i < detectionMat.rows; i++)
    {
        int   class_id   = detectionMat.at<float>(i, 1);
        float confidence = detectionMat.at<float>(i, 2);

        // Check if the detection is of good quality
        if (confidence > 0.4)
        {
            int box_x      = static_cast<int>(detectionMat.at<float>(i, 3) * frame.cols);
            int box_y      = static_cast<int>(detectionMat.at<float>(i, 4) * frame.rows);
            int box_width  = static_cast<int>(detectionMat.at<float>(i, 5) * frame.cols - box_x);
            int box_height = static_cast<int>(detectionMat.at<float>(i, 6) * frame.rows - box_y);
            rectangle(frame,
                      cv::Point(box_x, box_y),
                      cv::Point(box_x + box_width, box_y + box_height),
                      cv::Scalar(255, 255, 255),
                      2);
            putText(frame,
                    mClassNames[class_id - 1].c_str(),
                    cv::Point(box_x, box_y - 5),
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.5,
                    cv::Scalar(0, 255, 255),
                    1);
        }
    }
}
