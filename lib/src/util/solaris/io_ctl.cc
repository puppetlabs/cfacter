#include <facter/util/solaris/io_ctl.hpp>
#include <sys/ioctl.h>

using namespace std;

namespace facter { namespace util { namespace solaris {

    io_ctl_exception::io_ctl_exception(std::string const& message) :
        runtime_error(message)
    {
    }

    io_ctl::io_ctl(int desc) :
        descriptor(desc)
    {
        if (desc == -1) {
            throw io_ctl_exception("invalid descriptor");
        }
    }
}}}
