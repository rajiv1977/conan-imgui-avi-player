# conan-matlab-imgui-avi-and-webcam-player

# This branch provides an ImGui Matlab AVI and Web-Cam player based on RGB format.

# Prerequisite (Window or Unix):
* Conan
* Matlab (2023 or above)
* Visual Studio 2022 (Win)

# Build (Window):
* Run **run_conan_imgui_avi_player_debug.bat** or **run_conan_imgui_avi_player_release.bat**
* It will spontaneously open up an example from MATLAB.
* *Run*.

# Build (Unix):
```Matlab
conan install . -c tools.system.package_manager:mode=install -c tools.system.package_manager:sudo=True --output-folder=build --build=missing --settings=build_type=Debug
cd build
cmake .. -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=./build/build/Debug/generators/conan_toolchain.cmake -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
./matlab-imgui-plot-conan (just for testing)
```

# What you need:
**imguiAviMex**

Example: imguiAviMex("name.avi") or imguiAviMex("name.avi", freq) or imguiAviMex("webcam")
