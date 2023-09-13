#include "ImGuiPlayer.h"

// using namespace YUV420P;

YUV420P::ImGuiPlayer::ImGuiPlayer()
    : mVideo{}
    , mLibVideoIOFrame{}
    , mWebCamOn(false)
{
}

YUV420P::ImGuiPlayer::~ImGuiPlayer()
{
    // Clean-up the graphics resources
    cleanupFrame(mVideo);
}

void YUV420P::ImGuiPlayer::cleanupFrame(VideoData_t& videoData)
{
    // Delete the texture
    glDeleteTextures(1, &videoData.imageTexture);
}

std::array<Vector2f_t, 2U> YUV420P::ImGuiPlayer::computeCurrentImageSize(const VideoData_t& video)
{
    Vector2f_t contentMinPos = {};
    Vector2f_t imageSize     = {};

    if (video.frame != nullptr)
    {
        // Get the content window size
        Vector2f_t windowPos = Vector2f_t(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
        contentMinPos =
            Vector2f_t(ImGui::GetWindowContentRegionMin().x, ImGui::GetWindowContentRegionMin().y) + windowPos;
        Vector2f_t contentMaxPos =
            Vector2f_t(ImGui::GetWindowContentRegionMax().x, ImGui::GetWindowContentRegionMax().y) + windowPos;

        // Compute the window aspect ratio
        Vector2f_t windowSize        = contentMaxPos - contentMinPos;
        float      windowAspectRatio = windowSize.x() / windowSize.y();

        // Compute the image aspect ratio
        float imageAspectRatio = static_cast<float>(video.frame->width) / static_cast<float>(video.frame->height);

        // Truncate the positions to maintain image scaling
        imageSize = (windowAspectRatio > imageAspectRatio)
                        ? Vector2f_t(imageAspectRatio * windowSize.y(), windowSize.y())
                        : Vector2f_t(windowSize.x(), windowSize.x() / imageAspectRatio);
    }
    return {contentMinPos, imageSize};
}

void YUV420P::ImGuiPlayer::drawFrame()
{
    // Scoped mutex lock
    std::lock_guard<std::mutex> dataLock(mVideo.dataMutex);

    // Only draw valid frames
    if (mVideo.isInitialized && mVideo.frame != nullptr)
    {
        // For convenience, get a reference to the frame
        const VideoIOFrame_t& frame = *mVideo.frame;

        // Scoped mutex lock for the frame
        std::lock_guard<std::mutex> dataLock(frame.lock);

        // Bind image texture
        glBindTexture(GL_TEXTURE_2D, mVideo.imageTexture);

        // Set filtering parameters for display
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Copy the current information to the texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.width, frame.height, 0, GL_RGB, GL_UNSIGNED_BYTE, frame.pixels);

        // plot the image
        const auto currentSize = computeCurrentImageSize(mVideo);
        ImVec2     p_min       = {currentSize.at(0).x(), currentSize.at(0).y()};
        ImVec2 p_max = {currentSize.at(0).x() + currentSize.at(1).x(), currentSize.at(0).y() + currentSize.at(1).y()};
        ImGui::GetWindowDrawList()->AddImage((void*) mVideo.imageTexture, p_min, p_max);

        if (!mWebCamOn)
        {
            addText<uint32_t>("Frame ID: ",
                mVideo.frame->frameNumber,
                Vector2f_t(10.0F, 10.0F),
                mVideo.frame->width,
                mVideo.frame->height,
                { 0.27F, 0.40F, 1.0F },
                1.0F);
            addText<uint32_t>("Freq (Hz): ",
                mVideo.frame->fps,
                Vector2f_t(10.0F, 40.0F),
                mVideo.frame->width,
                mVideo.frame->height,
                { 0.27F, 0.40F, 1.0F },
                1.0F);
         }

        // Unbind image texture by restoring default
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else
    {
        // Attempt to initialize the required textures
        initializeFrame(mVideo);

        // Display unavailable message
        ImGui::Text("Video unavailable");
    }
}

void YUV420P::ImGuiPlayer::initializeFrame(VideoData_t& videoData)
{
    // Create a OpenGL texture identifier
    glGenTextures(1, &videoData.imageTexture);

    // Set the initialization flag
    videoData.isInitialized = videoData.imageTexture > 0;
}

void YUV420P::ImGuiPlayer::updateFrame(VideoData_t& videoData, const VideoIOFrame_t& frame)
{
    // Scoped mutex lock
    std::lock_guard<std::mutex> dataLock(videoData.dataMutex);

    // Update the frame information
    videoData.frame = &frame;
}

void YUV420P::ImGuiPlayer::getWebCamFrame(cv::Mat& frame)
{
    auto iplFrame = cvIplImage(frame);
    mLibVideoIOFrame.width = iplFrame.width;
    mLibVideoIOFrame.height = iplFrame.height;
    mLibVideoIOFrame.stride = (iplFrame.width + 3) & 0xfffffffc;
    mLibVideoIOFrame.pixels = (uint8_t*)iplFrame.imageData;
    mLibVideoIOFrame.type = YUV420P::LVIO_RGB_8BPP;
    mLibVideoIOFrame.decodeStatus = YUV420P::LVIO_SUCCESS;
    updateFrame(mVideo, mLibVideoIOFrame);
    mWebCamOn = true;
}

void YUV420P::ImGuiPlayer::getRGBFrame(int      width,
                                       int      height,
                                       uint8_t* data1,
                                       uint8_t* data2,
                                       uint8_t* data3,
                                       int*     lineSize,
                                       int      displayPictureNumber,
                                       float    samplingTime)
{
    // YUV420P::VideoIOFrame_t libVideoIOFrame = {};
    mLibVideoIOFrame.width  = width;
    mLibVideoIOFrame.height = height;
    mLibVideoIOFrame.stride = (width + 3) & 0xfffffffc; // round to multiple of 4

    mLibVideoIOFrame.pixels = RGB24BPP::rgb(data1 /* Y - Input */,
                                            data2 /* U - Input */,
                                            data3 /* V - Input */,
                                            mLibVideoIOFrame.width,
                                            mLibVideoIOFrame.height,
                                            mLibVideoIOFrame.stride * 3, // need stride value in bytes here
                                            lineSize[0],
                                            lineSize[1],
                                            lineSize[2]);

    mLibVideoIOFrame.type         = YUV420P::LVIO_RGB_8BPP;
    mLibVideoIOFrame.decodeStatus = YUV420P::LVIO_SUCCESS;
    mLibVideoIOFrame.frameNumber  = displayPictureNumber;
    mLibVideoIOFrame.fps          = samplingTime;

    updateFrame(mVideo, mLibVideoIOFrame);
}

inline int YUV420P::RGB24BPP::pixelSaturate(int v, int minimum, int maximum)
{
    if (v < minimum)
    {
        return minimum;
    }
    if (v > maximum)
    {
        return maximum;
    }
    return v;
}

uint8_t* YUV420P::RGB24BPP::rgb(const uint8_t* Yo,
                                const uint8_t* Uo,
                                const uint8_t* Vo,
                                uint32_t       cols,
                                uint32_t       rows,
                                uint32_t       strideOut,
                                uint32_t       strideY,
                                uint32_t       strideU,
                                uint32_t       strideV)
{
    // YUV-->RGB conversion factors for JPEG data (from https://en.wikipedia.org/wiki/YCbCr#ITU-R_BT.601_conversion)
    const int K1 = int(1.402f * (1 << 16));
    const int K2 = int(0.714f * (1 << 16));
    const int K3 = int(0.334f * (1 << 16));
    const int K4 = int(1.772f * (1 << 16));

    uint8_t* rgb = (uint8_t*) malloc(strideOut * rows);
    if (rgb == NULL)
    {
        char msg[256];
        sprintf_s(&msg[0],
                  sizeof(msg),
                  "Out of memory error in libVideoIO. Could not allocate for size: {width:%u,height:%u}",
                  strideOut / 3,
                  rows);
        throw std::runtime_error(static_cast<const char*>(&msg[0]));
    }

    // convert pixel values 4-at-a-time from YUV to RGB
    for (int y = 0; y < (int) (rows - 1); y += 2)
    {
        const uint8_t* Y1 = Yo + strideY * y;
        const uint8_t* Y2 = Yo + strideY * (y + 1);
        const uint8_t* U  = Uo + strideU * y / 2;
        const uint8_t* V  = Vo + strideV * y / 2;

        uint8_t* out_ptr1 = rgb + y * strideOut;
        uint8_t* out_ptr2 = rgb + (y + 1) * strideOut;

        for (int x = 0; x < (int) (cols - 1); x += 2)
        {
            int8_t uf = *U - 128;
            int8_t vf = *V - 128;

            int R = *Y1 + (K1 * vf >> 16);
            int G = *Y1 - (K2 * vf >> 16) - (K3 * uf >> 16);
            int B = *Y1 + (K4 * uf >> 16);

            pixelSaturate(R, 0, 255);
            pixelSaturate(G, 0, 255);
            pixelSaturate(B, 0, 255);

            *out_ptr1++ = (uint8_t) R;
            *out_ptr1++ = (uint8_t) G;
            *out_ptr1++ = (uint8_t) B;

            Y1++;
            R = *Y1 + (K1 * vf >> 16);
            G = *Y1 - (K2 * vf >> 16) - (K3 * uf >> 16);
            B = *Y1 + (K4 * uf >> 16);

            pixelSaturate(R, 0, 255);
            pixelSaturate(G, 0, 255);
            pixelSaturate(B, 0, 255);

            *out_ptr1++ = (uint8_t) R;
            *out_ptr1++ = (uint8_t) G;
            *out_ptr1++ = (uint8_t) B;

            R = *Y2 + (K1 * vf >> 16);
            G = *Y2 - (K2 * vf >> 16) - (K3 * uf >> 16);
            B = *Y2 + (K4 * uf >> 16);

            pixelSaturate(R, 0, 255);
            pixelSaturate(G, 0, 255);
            pixelSaturate(B, 0, 255);

            *out_ptr2++ = (uint8_t) R;
            *out_ptr2++ = (uint8_t) G;
            *out_ptr2++ = (uint8_t) B;
            Y2++;
            R = *Y2 + (K1 * vf >> 16);
            G = *Y2 - (K2 * vf >> 16) - (K3 * uf >> 16);
            B = *Y2 + (K4 * uf >> 16);

            pixelSaturate(R, 0, 255);
            pixelSaturate(G, 0, 255);
            pixelSaturate(B, 0, 255);

            *out_ptr2++ = (uint8_t) R;
            *out_ptr2++ = (uint8_t) G;
            *out_ptr2++ = (uint8_t) B;

            Y1++;
            Y2++;
            U++;
            V++;
        }
    }
    return rgb;
}

template <class T>
void YUV420P::ImGuiPlayer::addText(const std::string&        text,
                                   const T                   value,
                                   const Vector2f_t&         pixelPos,
                                   float                     width,
                                   float                     height,
                                   const std::vector<float>& color,
                                   float                     alpha)
{
    std::string strVal    = std::to_string(value);
    std::string newString = text + strVal;
    const auto  answer =
        Vector2f_t(computeCurrentImageSize(mVideo).at(0).x() +
                       (pixelPos.x() / mVideo.frame->width) * computeCurrentImageSize(mVideo).at(1).x(),
                   computeCurrentImageSize(mVideo).at(0).y() +
                       (pixelPos.y() / mVideo.frame->height) * computeCurrentImageSize(mVideo).at(1).y());
    ImVec2 data = {answer.x(), answer.y()};

    const auto windowAndImageSizes = computeCurrentImageSize(mVideo);
    const auto aspectRatio =
        3.0F * ImGui::GetFontSize() * (windowAndImageSizes[1].x() / width) * (windowAndImageSizes[1].y() / height);

    ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(),
                                        aspectRatio,
                                        data,
                                        IM_COL32(color[0] * 255.0F, color[1] * 255.0F, color[2] * 255.0F, alpha * 255),
                                        newString.c_str());
}
