/// <summary>
/// H264::Decoder
/// ---------------------------------------
///
/// Example that shows how to use the libav parser system. This class forces a
/// H264 parser and codec. You use it by opening a file that is encoded with x264
/// using the "load()" function. You can pass the framerate you want to use for playback.
/// If you don't pass the framerate, we will detect it as soon as the parser found
/// the correct information.
///
/// After calling "load()", you can call "readFrame()" which will read a new frame when
/// necessary. It will also make sure that it will read enough data from the buffer/file
/// when there is not enough data in the buffer.
///
/// "readFrame()" will trigger calls to the given "h264_decoder_callback" that you pass
/// to the constructor.
/// </summary>

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

#include "tinylib.h"

#define CODEC_CAP_TRUNCATED (1 << 3)
#define CODEC_FLAG_TRUNCATED (1 << 16)
#define H264_INBUF_SIZE 16384 /// number of bytes we read per chunk.
#define FF_INPUT_BUFFER_PADDING_SIZE 32

/// <summary>
/// decoder callback, which will be called when we have decoded a frame.
/// </summary>
typedef void (*decoderCallback)(AVFrame* frame, AVPacket* pkt, void* user);
namespace H264
{
class Decoder
{

  public:
    Decoder(decoderCallback frameCallback,
            void* user); /// pass in a callback function that is called whenever we decoded a video frame, make
    /// sure to call `readFrame()` repeatedly.
    ~Decoder(); /// d'tor, cleans up the allocated objects and closes the codec context.
    bool load(std::string filepath, float fps = 0.0f); /// load a video file which is encoded with x264.
    bool readFrame();                                  /// read a frame if necessary.

  private:
    void decodeFrame(uint8_t* data, int size); /// decode a frame we read from the buffer.
    int  readBuffer();                         /// read a bit more data from the buffer.
    bool update(bool& needsMoreBytes); /// internally used to update/parse the data we read from the buffer or file.

  public:
    std::vector<uint8_t>  buffer;       /// buffer we use to keep track of read/unused bitstream data.
    const AVCodec*        codec;        /// the AVCodec* which represents the H264 decoder.
    AVCodecContext*       codecContext; /// the context; keeps generic state.
    decoderCallback       cbFrame;      /// the callback function which will receive the frame/packet data.
    void*                 cbUser;       /// the void* with user data that is passed into the set callback.
    uint64_t              frameTimeout; /// timeout when we need to parse a new frame.
    uint64_t              frameDelay;   /// delay between frames (in ns).
    FILE*                 fp;           /// file pointer to the file from which we read the h264 data.
    int                   frame;        /// the number of decoded frames.
    uint8_t               inbuf[H264_INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE]; /// used to read chunks from the file.
    AVCodecParserContext* parser;  /// parser that is used to decode the h264 bitstream.
    AVFrame*              picture; /// will contain a decoded picture.
    float                 samplingTime;
};
} // namespace H264
