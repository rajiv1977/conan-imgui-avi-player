#include "Video.h"

using namespace Gui;

Video::Video(const std::string& src)
    : mFrame{}
    , mClassNames{}
    , mNeuralNetworkModel{}
    , mWindowName{}
    , mMaxScaleUp(100)
    , mScaleFactor(1)
{
    if (src.compare("webcam") == 0)
    {
        mCaption.open(0);
        mWindowName = "webcam";
    }
    else
    {
        mCaption.open(src);
        mWindowName = "video";
    }

    if (mCaption.isOpened() == false)
    {
        std::cout << "Cannot open the video camera" << std::endl;
        std::cin.get();
    }

    mFps = mCaption.get(cv::CAP_PROP_FPS);

    // Classification
    std::ifstream ifs(std::string("object_detection_classes.txt").c_str());
    std::string   line;
    while (getline(ifs, line))
    {
        mClassNames.push_back(line);
    }

    // load the neural network model
    mNeuralNetworkModel = cv::dnn::readNet("inference_graph.pb", "ssd_mobilenet.pbtxt.txt", "TensorFlow");

    showVideo();
}

Video::~Video()
{
    mFrame = {};
    mClassNames.clear();
    mCaption.release();
    mWindowName.clear();
    cv::destroyAllWindows();
}

bool Video::readFrame()
{
    bool status = true;
    status      = mCaption.read(mFrame);
    // cv::Mat I_YUV = {};
    // cv::cvtColor(mFrame, I_YUV, cv::COLOR_BGR2YCrCb);
    // cv::cvtColor(I_YUV, mFrame, cv::COLOR_YCrCb2RGB);
    nnmTracking(mFrame);
    return status;
}

// neural network model based tracking
void Video::nnmTracking(cv::Mat& frame)
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

void Video::scaleImage(int pos, void*)
{
    // Get the Scale factor from the trackbar
    double scaleFactorDouble = 1 + mScaleFactor / 100.0;

    // Set the factor to 1 if becomes 0
    if (scaleFactorDouble == 0)
    {
        scaleFactorDouble = 1;
    }
    cv::Mat scaledImage;

    // Resize the image
    cv::resize(mFrame, scaledImage, cv::Size(), scaleFactorDouble, scaleFactorDouble, cv::INTER_LINEAR);
    mFrame = scaledImage;
}

void Video::showVideo()
{
    while (mCaption.isOpened())
    {
        if (readFrame())
        {

            // Create Trackbars and associate a callback function
            // cv::createTrackbar("scale", mWindowName, &mScaleFactor, mMaxScaleUp, scaleImage);
            scaleImage(25, NULL);

            imshow(mWindowName, mFrame);
            int k = cv::waitKey(10);
            if (k == 113)
            {
                break;
            }
        }
    }
}
