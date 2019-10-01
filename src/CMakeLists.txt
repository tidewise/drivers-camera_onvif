rock_library(onvif_camera
    SOURCES Dummy.cpp
    HEADERS Dummy.hpp)

rock_executable(onvif_camera_bin Main.cpp
    DEPS onvif_camera)

