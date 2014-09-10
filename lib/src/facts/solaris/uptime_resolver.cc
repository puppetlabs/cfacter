#include <facter/facts/solaris/uptime_resolver.hpp>
#include <facter/logging/logging.hpp>
#include <facter/facts/collection.hpp>
#include <facter/facts/fact.hpp>
#include <facter/facts/scalar_value.hpp>
#include <facter/util/solaris/k_stat.hpp>
#include <sys/sysinfo.h>
#include <ctime>
#include <chrono>

LOG_DECLARE_NAMESPACE("facts.solaris.uptime");

using namespace std;
using namespace std::chrono;
using namespace facter::util;
using namespace facter::util::solaris;

namespace facter { namespace facts { namespace solaris {

    int uptime_resolver::uptime_in_seconds()
    {
        try {
            k_stat kc;
            k_stat_entry ke = kc[{"unix", "system_misc"}];
            auto time_at_boot_in_sec = ke.value<unsigned long>("boot_time");

            system_clock::time_point atboot{seconds(time_at_boot_in_sec)};
            system_clock::time_point now{system_clock::now()};
            seconds uptime{duration_cast<seconds>(now - atboot)};

            return uptime.count();
        } catch (exception &ex) {
            LOG_DEBUG("uptime failed: %1% (%2%): using /bin/uptime.", strerror(errno), errno);
            return facter::facts::posix::uptime_resolver::uptime_in_seconds();
        }
    }

}}}  // namespace facter::facts::solaris
