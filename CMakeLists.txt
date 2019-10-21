# CMakeLists.txt has to be located in the project folder and cmake has to be
# executed from 'project/build' with 'cmake ../'.
set(CMAKE_CXX_STANDARD 11)
cmake_minimum_required(VERSION 3.0)
find_package(Rock)
rock_init(camera_onvif 0.1)
rock_standard_layout()
