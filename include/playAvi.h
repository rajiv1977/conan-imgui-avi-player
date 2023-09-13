#pragma once

#include <stdlib.h>
#include <stdio.h>

#if defined(__linux) || defined(_WIN32)
#include <GL/glew.h>
#endif

// #define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#define ROXLU_USE_MATH
#define ROXLU_USE_OPENGL
#define ROXLU_IMPLEMENTATION

#include "Decoder.h"
#include "ImGuiPlayer.h"
#include "tinylib.h"

#include "WebCam.h"
#include "imgui.h"
#include "../bindings/imgui_impl_glfw.h"
#include "../bindings/imgui_impl_opengl3.h"

H264::Decoder*        decoder_ptr = NULL;
YUV420P::ImGuiPlayer* player_ptr  = NULL;
ImGui::WebCam* webcam_ptr = NULL;

void        error_callback(int err, const char* desc);
void        frame_callback(AVFrame* frame, AVPacket* pkt, void* user);
static void ShowDockingDisabledMessage();
void        toLower(std::string& data);
void webcam_callback(cv::Mat* frame, void* user);

namespace ImGui
{

class PlayAvi
{
  public:
    PlayAvi() = default;
    PlayAvi(std::string& fileName, float& freq)
    {
        bool webCamStatus = false;
        if (fileName.compare("webcam") == 0)
        {
            webCamStatus = true;
        }

        if (!webCamStatus)
        {
            std::size_t found = fileName.find_last_of(".");
            std::string str = fileName.substr(found + 1, fileName.size());
            toLower(str);
            if (!(str.compare("avi") == 0 || str.compare("tavi") == 0))
            {
                std::string errorMsg = ".avi format only.";
                printf(".avi format only. \n");
            }
        }

        glfwSetErrorCallback(error_callback);
        if (!glfwInit())
        {
            printf("Error: cannot setup glfw.\n");
        }

        glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        GLFWwindow* win = NULL;
        int         w   = 1516;
        int         h   = 853;

        win = glfwCreateWindow(w, h, "ImGui Matlab AVI Player", NULL, NULL);
        if (!win)
        {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
        const char* glsl_version = "#version 150";

        glfwMakeContextCurrent(win);
        glfwSwapInterval(1);

#if defined(__linux) || defined(_WIN32)
        if (glewInit() != 0)
        {
            printf("Error: cannot initialize glew.\n");
            ::exit(EXIT_FAILURE);
        }
#endif

        // ----------------------------------------------------------------
        // THIS IS WHERE YOU START CALLING OPENGL FUNCTIONS, NOT EARLIER!!
        // ----------------------------------------------------------------
        H264::Decoder decoder(frame_callback, NULL);
        ImGui::WebCam webcam(webcam_callback, NULL);
        if (!webCamStatus)
        {
            decoder_ptr = &decoder;
            printf("Loading video file: %s\n", fileName);
            if (!decoder.load(fileName, freq))
            {
                printf("Error loading video file.\n");
                ::Sleep(5000);
                ::exit(EXIT_FAILURE);
            }
        }
        else
        {
            webcam_ptr = &webcam;
        }
        YUV420P::ImGuiPlayer player;
        player_ptr = &player;

        int screen_width, screen_height;
        glfwGetFramebufferSize(win, &screen_width, &screen_height);
        glViewport(0, 0, screen_width, screen_height);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ShowDockingDisabledMessage();

        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(win, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        while (!glfwWindowShouldClose(win))
        {
            glfwPollEvents();
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // feed inputs to dear imgui, start new frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            bool show_another_window = true;

            std::string videoName = "Video";
            if (!webCamStatus)
            {
                decoder.readFrame();
            }
            else
            {
                videoName = "webcam";
                webcam.readFrame();
            }

            ImGui::Begin(videoName.c_str(), &show_another_window);
            player.drawFrame();
            ImGui::End();

            ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(win, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(win);
        }

        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(win);
        glfwTerminate();

        ::Sleep(5000);
    }
    ~PlayAvi(){};
};

} // namespace ImGui

void error_callback(int err, const char* desc)
{
    printf("GLFW error: %s (%d)\n", desc, err);
}

void frame_callback(AVFrame* frame, AVPacket* pkt, void* user)
{
    if (player_ptr)
    {
        player_ptr->getRGBFrame(frame->width,
                                frame->height,
                                frame->data[0],
                                frame->data[1],
                                frame->data[2],
                                frame->linesize,
                                frame->display_picture_number,
                                frame->sample_rate);
    }
}

void webcam_callback(cv::Mat* frame, void* user)
{
    if (webcam_ptr)
    {
        player_ptr->getWebCamFrame(*frame);
    }
}

static void ShowDockingDisabledMessage()
{
    ImGuiIO& io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
    io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows
    io.ConfigDockingNoSplit              = false;
    io.ConfigViewportsNoAutoMerge        = true; // Force all windows to use separate viewport
    io.ConfigWindowsMoveFromTitleBarOnly = true; // Allows mouse functions in world visualization
}

void toLower(std::string& data)
{
    std::transform(data.begin(), data.end(), data.begin(), [](unsigned char c) { return std::tolower(c); });
}
