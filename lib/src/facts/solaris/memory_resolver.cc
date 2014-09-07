#include <facter/facts/solaris/memory_resolver.hpp>
#include <facter/logging/logging.hpp>
#include <facter/util/file.hpp>
#include <boost/algorithm/string.hpp>
#include <kstat.h>
#include <sys/sysinfo.h>
#include <sys/sysconfig.h>

using namespace std;
using namespace facter::util;
LOG_DECLARE_NAMESPACE("facts.solaris.memory");

namespace facter { namespace facts { namespace solaris {
    bool memory_resolver::get_memory_statistics(
            collection& facts,
            uint64_t& mem_free,
            uint64_t& mem_total,
            uint64_t& swap_free,
            uint64_t& swap_total)
    {
        long page_size = sysconf(_SC_PAGESIZE);
        kstat_ctl_t *kc;
        if ((kc = kstat_open()) == nullptr) {
            LOG_DEBUG("kstat_open failed: %1% (%2%).", strerror(errno), errno);
            return facter::facts::posix::memory_resolver::get_memory_statistics(facts, mem_free, mem_total, swap_free, swap_total);
        }

        kstat_t *kp;
        kstat_named_t *knp;

        if ((kp = kstat_lookup(kc, const_cast<char*>("unix"), 0, const_cast<char *>("system_pages"))) == nullptr) {
            kstat_close(kc);
            LOG_DEBUG("kstat_lookup failed: memory facts unavailable %1% (%2%)", strerror(errno), errno);
            return facter::facts::posix::memory_resolver::get_memory_statistics(facts, mem_free, mem_total, swap_free, swap_total);
        }

        if (kstat_read(kc, kp, 0) == -1) {
            kstat_close(kc);
            LOG_DEBUG("kstat_lookup failed: memory facts unavailable %1% (%2%)", strerror(errno), errno);
            return facter::facts::posix::memory_resolver::get_memory_statistics(facts, mem_free, mem_total, swap_free, swap_total);
        }

        if ((knp = reinterpret_cast<kstat_named_t*>(kstat_data_lookup(kp, const_cast<char *>("physmem")))) == nullptr) {
            kstat_close(kc);
            LOG_DEBUG("kstat_lookup failed: memory facts unavailable %1% (%2%)", strerror(errno), errno);
            return facter::facts::posix::memory_resolver::get_memory_statistics(facts, mem_free, mem_total, swap_free, swap_total);
        }
        mem_total = knp->value.ul * page_size;

        if ((knp = reinterpret_cast<kstat_named_t*>(kstat_data_lookup(kp, const_cast<char *>("pagesfree")))) == nullptr) {
            kstat_close(kc);
            LOG_DEBUG("kstat_lookup failed: memory facts unavailable %1% (%2%)", strerror(errno), errno);
            return facter::facts::posix::memory_resolver::get_memory_statistics(facts, mem_free, mem_total, swap_free, swap_total);
        }
        mem_free = knp->value.ul * page_size;

        kstat_close(kc);

        // Swap requires a little more investigation as to the correct method. See
        // https://community.oracle.com/thread/1951228?start=0&tstart=0
        // http://www.brendangregg.com/K9Toolkit/swapinfo
        swap_free = 0;
        swap_total = 0;

        return true;
    }

}}}  // namespace facter::facts::solaris
