rock_find_pkgconfig(gsoap REQUIRED gsoap)
include_directories(${gsoap_PREFIX}/share)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/onvif)

add_definitions(-Wno-nonnull-compare -Wno-address -DWITH_OPENSSL -DWITH_DOM)

list(APPEND gsoap_PLUGINS_SOURCES
     ${gsoap_PREFIX}/share/gsoap/plugin/smdevp.c
     ${gsoap_PREFIX}/share/gsoap/plugin/mecevp.c
     ${gsoap_PREFIX}/share/gsoap/plugin/wsaapi.c
     ${gsoap_PREFIX}/share/gsoap/plugin/wsseapi.cpp
     ${gsoap_PREFIX}/share/gsoap/plugin/wsddapi.c)

list(APPEND gsoap_GEN
     onvif/soapC.cpp
     onvif/wsddClient.cpp
     onvif/wsddServer.cpp
     onvif/soapAdvancedSecurityServiceBindingProxy.cpp
     onvif/soapDeviceBindingProxy.cpp
     onvif/soapDeviceIOBindingProxy.cpp
     onvif/soapImagingBindingProxy.cpp
     onvif/soapMediaBindingProxy.cpp
     onvif/soapPTZBindingProxy.cpp
     onvif/soapPullPointSubscriptionBindingProxy.cpp
     onvif/soapRemoteDiscoveryBindingProxy.cpp)

rock_library(camera_onvif
    SOURCES CameraOnvif.cpp ${gsoap_GEN} ${gsoap_PLUGINS_SOURCES}
    HEADERS CameraOnvif.hpp
    DEPS_PKGCONFIG gsoap gsoap++ gsoapssl libssl libcrypto base-types)

rock_executable(camera_onvif_bin Main.cpp CameraOnvif.cpp
    ${gsoap_GEN}
    ${gsoap_PLUGINS_SOURCES}
    DEPS_PKGCONFIG gsoap gsoap++ gsoapssl libssl libcrypto base-types)

set_source_files_properties(${gsoap_PLUGINS_SOURCES} PROPERTIES LANGUAGE CXX)

