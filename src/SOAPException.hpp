#include <stdexcept>
#include <string>

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
