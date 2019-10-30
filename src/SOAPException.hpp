#ifndef CAMERA_ONVIF_SOAPEXCEPTION_HPP
#define CAMERA_ONVIF_SOAPEXCEPTION_HPP

#include <stdexcept>
#include <string>

namespace camera_onvif {
    /**
     * This class manage to raise Onvif errors in SOAP connection.
     */
    class SOAPException : public std::runtime_error {
    private:
        /* data */
    public:
        SOAPException(const std::string& what_arg);
        ~SOAPException();
    };
    SOAPException::SOAPException(const std::string& what_arg) : std::runtime_error(what_arg){
    }

    SOAPException::~SOAPException(){
    }
}


#endif