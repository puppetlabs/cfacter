#include <facter/util/environment.hpp>
#include <boost/nowide/cenv.hpp>

#ifndef ARG_MAX
    // ARG_MAX is defined on POSIX systems
    // In this context we use it for the maximum length of a user-defined environment variable.
    // For Windows, that limit is 32,767 (http://msdn.microsoft.com/en-us/library/ms682653.aspx)
    #define ARG_MAX 32767u
#endif

using namespace std;

namespace facter { namespace util {

    environment_too_long_exception::environment_too_long_exception(string const& message) :
        runtime_error(message)
    {
    }

    bool environment::get(string const& name, string& value)
    {
        auto variable = boost::nowide::getenv(name.c_str());
        if (!variable) {
            return false;
        } else if (strlen(variable) > ARG_MAX) {
            throw environment_too_long_exception(str(boost::format("Environment variable %1% exceeds maximum length %2%.") % name % ARG_MAX));
        }

        value = variable;
        return true;
    }

    bool environment::set(string const& name, string const& value)
    {
        return boost::nowide::setenv(name.c_str(), value.c_str(), 1) == 0;
    }

    bool environment::clear(string const& name)
    {
        return boost::nowide::unsetenv(name.c_str()) == 0;
    }

}}  // namespace facter::util
