cmake_minimum_required(VERSION 3.9)
project(camera_onvif VERSION 0.1)
set(CMAKE_CXX_STANDARD 11)

find_package(Rock)
rock_init(camera_onvif 0.1)
rock_standard_layout()
