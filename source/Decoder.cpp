#include "Decoder.h"

#if defined(_WIN32)
#pragma warning(disable : 4996) /// otherwise deprecated funcs cause compile errors
#endif

using namespace H264;

Decoder::Decoder(decoderCallback frameCallback, void* user)
    : codec(NULL)
    , codecContext(NULL)
    , parser(NULL)
    , fp(NULL)
    , frame(0)
    , cbFrame(frameCallback)
    , cbUser(user)
    , frameTimeout(0)
    , frameDelay(0)
    , samplingTime(0.0F)
{
    avcodec_register_all();
}

Decoder::~Decoder()
{

    if (parser)
    {
        av_parser_close(parser);
        parser = NULL;
    }

    if (codecContext)
    {
        avcodec_close(codecContext);
        av_free(codecContext);
        codecContext = NULL;
    }

    if (picture)
    {
        av_free(picture);
        picture = NULL;
    }

    if (fp)
    {
        fclose(fp);
        fp = NULL;
    }

    cbFrame      = NULL;
    cbUser       = NULL;
    frame        = 0;
    frameTimeout = 0;
}

bool Decoder::load(std::string filepath, float fps)
{
    samplingTime = fps;
    codec        = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec)
    {
        printf("Error: cannot find the h264 codec: %s\n", filepath.c_str());
        return false;
    }

    codecContext = avcodec_alloc_context3(codec);

    if (codec->capabilities & CODEC_CAP_TRUNCATED)
    {
        codecContext->flags |= CODEC_FLAG_TRUNCATED;
    }

    if (avcodec_open2(codecContext, codec, NULL) < 0)
    {
        printf("Error: could not open codec.\n");
        return false;
    }

    fp = fopen(filepath.c_str(), "rb");

    if (!fp)
    {
        printf("Error: cannot open: %s\n", filepath.c_str());
        return false;
    }

    picture = av_frame_alloc();
    parser  = av_parser_init(AV_CODEC_ID_H264);

    if (!parser)
    {
        printf("Erorr: cannot create H264 parser.\n");
        return false;
    }

    if (fps > 0.0001f)
    {
        frameDelay   = (1.0f / fps) * 1000ull * 1000ull * 1000ull;
        frameTimeout = rx_hrtime() + frameDelay;
    }

    readBuffer();

    return true;
}

bool Decoder::readFrame()
{

    uint64_t now = rx_hrtime();
    if (now < frameTimeout)
    {
        return false;
    }

    bool needs_more = false;

    while (!update(needs_more))
    {
        if (needs_more)
        {
            readBuffer();
            if (feof(fp) || ferror(fp))
            {
                return false;
            }
        }
    }

    // it may take some 'reads' before we can set the fps
    if (frameTimeout == 0 && frameDelay == 0)
    {
        double fps = av_q2d(codecContext->time_base);
        if (fps > 0.0)
        {
            frameDelay = fps * 1000ull * 1000ull * 1000ull;
        }
    }

    if (frameDelay > 0)
    {
        frameTimeout = rx_hrtime() + frameDelay;
    }

    return true;
}

void Decoder::decodeFrame(uint8_t* data, int size)
{

    AVPacket pkt;
    int      got_picture = 0;
    int      len         = 0;

    av_init_packet(&pkt);

    pkt.data = data;
    pkt.size = size;

    len         = avcodec_send_packet(codecContext, &pkt);
    got_picture = avcodec_receive_frame(codecContext, picture);

    if (len < 0)
    {
        printf("Error while decoding a frame.\n");
    }

    if (got_picture < 0)
    {
        return;
    }

    ++frame;
    // printf("Decode frame %d\n", frame);

    if (cbFrame)
    {
        picture->display_picture_number = static_cast<int>(frame);
        picture->sample_rate            = samplingTime;
        cbFrame(picture, &pkt, cbUser);
    }
}

int Decoder::readBuffer()
{

    int bytes_read = (int) fread(inbuf, 1, H264_INBUF_SIZE, fp);

    if (bytes_read > 0)
    {
        std::copy(inbuf, inbuf + bytes_read, std::back_inserter(buffer));
    }

    return bytes_read;
}

bool Decoder::update(bool& needsMoreBytes)
{

    needsMoreBytes = false;

    if (!fp)
    {
        printf("Cannot update .. file not opened...\n");
        return false;
    }

    if (buffer.size() == 0)
    {
        needsMoreBytes = true;
        return false;
    }

    uint8_t* data = NULL;
    int      size = 0;
    int len = av_parser_parse2(parser, codecContext, &data, &size, &buffer[0], buffer.size(), 0, 0, AV_NOPTS_VALUE);

    if (size == 0 && len >= 0)
    {
        needsMoreBytes = true;
        return false;
    }

    if (len)
    {
        decodeFrame(&buffer[0], size);
        buffer.erase(buffer.begin(), buffer.begin() + len);
        return true;
    }

    return false;
}
