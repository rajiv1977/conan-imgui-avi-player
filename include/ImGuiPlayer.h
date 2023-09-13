#pragma once

#include <Eigen/Dense>
#include <GL/glew.h>
#include <mutex>

#include "imgui.h"
#include "WebCam.h"

using Vector2f_t = Eigen::Vector2f;
namespace YUV420P
{
enum
{
    LVIO_SUCCESS        = 0,
    LVIO_EOF            = 1,
    LVIO_FILEIOERR      = 2,
    LVIO_DECODE_FAILURE = 3
};

enum VideoIOFrameType_e
{
    LVIO_YUV422 = 0,
    LVIO_RGB_8BPP
};

struct VideoIOFrame_t
{
    mutable std::mutex lock;
    uint32_t           width;
    uint32_t           height;
    uint32_t           stride;
    uint32_t           frameNumber;
    uint8_t*           pixels;
    uint32_t           decodeStatus;
    VideoIOFrameType_e type;
    int64_t            fps;
};

struct VideoData_t
{
    std::mutex            dataMutex;
    bool                  isInitialized = false;
    GLuint                imageTexture  = 0;
    const VideoIOFrame_t* frame         = nullptr;
};

class ImGuiPlayer
{
  public:
    ImGuiPlayer();
    ~ImGuiPlayer();

    void drawFrame();
    void getRGBFrame(int      width,
                     int      height,
                     uint8_t* data1,
                     uint8_t* data2,
                     uint8_t* data3,
                     int*     lineSize,
                     int      displayPictureNumber,
                     float    samplingTime);
    void getWebCamFrame(cv::Mat& frame);
    template <class T>
    void addText(const std::string&        text,
                 const T                   value,
                 const Vector2f_t&         pixelPos,
                 float                     width,
                 float                     height,
                 const std::vector<float>& color,
                 float                     alpha = 1.0F);

  private:
    void                       cleanupFrame(VideoData_t& videoData);
    std::array<Vector2f_t, 2U> computeCurrentImageSize(const VideoData_t& video);
    void                       initializeFrame(VideoData_t& videoData);
    void                       updateFrame(VideoData_t& videoData, const VideoIOFrame_t& frame);

    VideoIOFrame_t mLibVideoIOFrame;
    VideoData_t    mVideo;
    bool           mWebCamOn;
};

namespace RGB24BPP
{
static inline int pixelSaturate(int v, int minimum, int maximum);

// YUV20 to RGB24BPP format
uint8_t* rgb(const uint8_t* Yo,
             const uint8_t* Uo,
             const uint8_t* Vo,
             uint32_t       cols,
             uint32_t       rows,
             uint32_t       strideOut,
             uint32_t       strideY,
             uint32_t       strideU,
             uint32_t       strideV);
} // namespace RGB24BPP

} // namespace YUV420P
