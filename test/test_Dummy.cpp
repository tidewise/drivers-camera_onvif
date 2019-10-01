#include <boost/test/unit_test.hpp>
#include <onvif_camera/Dummy.hpp>

using namespace onvif_camera;

BOOST_AUTO_TEST_CASE(it_should_not_crash_when_welcome_is_called)
{
    onvif_camera::DummyClass dummy;
    dummy.welcome();
}
