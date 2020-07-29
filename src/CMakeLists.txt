rock_find_pkgconfig(gsoap REQUIRED gsoap)
include_directories(${gsoap_PREFIX}/share)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/onvif)

add_definitions(-Wno-nonnull-compare -Wno-address -DWITH_OPENSSL -DWITH_DOM)

list(APPEND gsoap_PLUGINS_SOURCES
    ${gsoap_PREFIX}/share/gsoap/plugin/smdevp.c
    ${gsoap_PREFIX}/share/gsoap/plugin/mecevp.c
    ${gsoap_PREFIX}/share/gsoap/plugin/wsaapi.c
    ${gsoap_PREFIX}/share/gsoap/plugin/wsseapi.cpp
    ${gsoap_PREFIX}/share/gsoap/plugin/wsddapi.c
)

list(APPEND gsoap_GEN
    soapC.cpp
    wsddClient.cpp
    wsddServer.cpp
    soapAdvancedSecurityServiceBindingProxy.cpp
    soapDeviceBindingProxy.cpp
    soapDeviceIOBindingProxy.cpp
    soapImagingBindingProxy.cpp
    soapMediaBindingProxy.cpp
    soapPTZBindingProxy.cpp
    soapPullPointSubscriptionBindingProxy.cpp
    soapRemoteDiscoveryBindingProxy.cpp
)

add_custom_command(
    OUTPUT ${gsoap_GEN}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif.h
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/onvif-h2cpp ./
        ${CMAKE_CURRENT_SOURCE_DIR}/onvif/onvif.h
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

rock_library(camera_onvif
    USE_BINARY_DIR
    SOURCES CameraOnvif.cpp ${gsoap_GEN} ${gsoap_PLUGINS_SOURCES}
    HEADERS CameraOnvif.hpp
    DEPS_PKGCONFIG gsoap++ gsoapssl++ libssl libcrypto base-types)

rock_executable(camera_onvif_bin Main.cpp CameraOnvif.cpp
    DEPS camera_onvif)

set_source_files_properties(${gsoap_PLUGINS_SOURCES} PROPERTIES LANGUAGE CXX)

