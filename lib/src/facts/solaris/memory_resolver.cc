#include <facter/facts/solaris/memory_resolver.hpp>
#include <facter/logging/logging.hpp>
#include <facter/util/file.hpp>
#include <facter/util/solaris/k_stat.hpp>
#include <boost/algorithm/string.hpp>
#include <sys/sysinfo.h>
#include <sys/sysconfig.h>

using namespace std;
using namespace facter::util;
using namespace facter::util::solaris;
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
        try {
            k_stat kc;
            k_stat_entry ke = kc[{"unix", "system_pages"}];
            mem_total = ke.value<unsigned long>("physmem") * page_size;
            mem_free = ke.value<unsigned long>("pagesfree") * page_size;

            // Swap requires a little more investigation as to the correct method. See
            // https://community.oracle.com/thread/1951228?start=0&tstart=0
            // http://www.brendangregg.com/K9Toolkit/swapinfo
            swap_free = 0;
            swap_total = 0;

            return true;
        } catch (exception &ex) {
            LOG_DEBUG("%1%: memory facts unavailable (%2%)", ex.what(), strerror(errno));
            return facter::facts::posix::memory_resolver::get_memory_statistics(facts, mem_free, mem_total, swap_free, swap_total);
        }
    }

}}}  // namespace facter::facts::solaris
