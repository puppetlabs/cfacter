#include <facter/facts/solaris/memory_resolver.hpp>
#include <facter/logging/logging.hpp>
#include <facter/util/file.hpp>
#include <facter/execution/execution.hpp>
#include <facter/util/regex.hpp>
#include <facter/util/solaris/k_stat.hpp>
#include <boost/algorithm/string.hpp>
#include <sys/sysinfo.h>
#include <sys/sysconfig.h>
#include <sys/stat.h>
#include <sys/swap.h>

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
        const long page_size = sysconf(_SC_PAGESIZE);
        const long max_dev_size = PATH_MAX;
        try {
            k_stat kc;
            k_stat_entry ke = kc[{"unix", "system_pages"}];
            mem_total = ke.value<unsigned long>("physmem") * page_size;
            mem_free = ke.value<unsigned long>("pagesfree") * page_size;

            // Swap requires a little more effort see
            // https://community.oracle.com/thread/1951228?start=0&tstart=0
            // http://www.brendangregg.com/K9Toolkit/swapinfo

            int num = 0;
            if ((num = swapctl(SC_GETNSWP,  0)) == -1) {
                throw runtime_error("swapctl: GETNSWP");
            }
            if (num == 0) {
                // no swap devices configured
                return true;
            }

            vector<char> buffer((num + 1) * sizeof(swapent_t) + sizeof(swaptbl_t));
            vector<vector<char>> str_table(num + 1);

            swaptbl_t* swaps = reinterpret_cast<swaptbl_t*>(buffer.data());
            swaps->swt_n = num + 1;

            for (int i = 0; i < num + 1; i++) {
                str_table[i].resize(max_dev_size);
                swaps->swt_ent[i].ste_path = str_table[i].data();
            }

            if (swapctl(SC_LIST, swaps) == -1) {
                throw runtime_error("swapctl: SC_LIST");
            }

            for (auto ent : swaps->swt_ent) {
                swap_free += ent.ste_free;
                swap_total += ent.ste_pages;
            }

            swap_free = swap_free * page_size;
            swap_total = swap_total * page_size;

            return true;
        } catch (exception &ex) {
            LOG_DEBUG("%1%: memory facts unavailable (%2%)", ex.what(), strerror(errno));
            return facter::facts::posix::memory_resolver::get_memory_statistics(facts, mem_free, mem_total, swap_free, swap_total);
        }
    }

}}}  // namespace facter::facts::solaris
