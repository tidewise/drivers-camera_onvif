#ifndef CAMERA_ONVIF_CAMERAONVIF_HPP
#define CAMERA_ONVIF_CAMERAONVIF_HPP

#include <base/Time.hpp>

#include <string>

class _trt__GetVideoEncoderConfigurationResponse;

namespace camera_onvif {
    struct Resolution {
        int width;
        int height;
        Resolution() = default;
        ~Resolution() = default;
    };

    struct ImageParam {
        float contrast;
        float color_saturation;
        float brightness;
        ImageParam() = default;
        ~ImageParam() = default;
    };

    /**
     * Class to handle the Onvif connection.
     */
    class CameraOnvif {
        // The goal of Private is to avoid leaking the whole SOAP generated code
        struct Private;
        Private* m_private = nullptr;

        std::string m_user;
        std::string m_pass;
        std::string m_uri;
        std::string m_video_token;
        std::string m_video_conf_token;

        base::Time m_timeout;

        //[Min, Max]
        float m_brightness[2];
        //[Min, Max]
        float m_color_saturation[2];
        //[Min, Max]
        float m_contrast[2];

        /*
         * Function to authenticate the connection and then after 10 seconds we shoud call
         * the function again to use ONVIF.
         */
        void setCredentials();
        // to report an error
        void reportError();
        void init();
        void getVideoEncoderConfiguration(_trt__GetVideoEncoderConfigurationResponse& resp);

    public:
        CameraOnvif(const std::string& user, const std::string& pass,
                    const std::string& ip);
        ~CameraOnvif();

        void printCameraInfo();

        // Set image parameters: Receives ImageParam struct as argument.
        void setImageParam (const ImageParam& new_param);
        /*
         * Get image parameters: Returns a struct with brightness, color_saturation and
         * contrast float attributes.
         */
        ImageParam getImageParam();

        void setResolution(int width, int height);
        // Get resolution: Returns a struct with width and height integers attributes.
        Resolution getResolution();

        // Get timeout: Returns an integer with the connection authentication timeout.
        base::Time getTimeout ();
        // Set timeout: Set an integer with the connection authentication timeout.
        void setTimeout (base::Time timeout);
    };
}

#endif