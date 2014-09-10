#include <facter/facts/solaris/processor_resolver.hpp>
#include <facter/logging/logging.hpp>
#include <facter/facts/scalar_value.hpp>
#include <facter/facts/map_value.hpp>
#include <facter/facts/array_value.hpp>
#include <facter/facts/collection.hpp>
#include <facter/facts/fact.hpp>
#include <unordered_set>
#include <sys/processor.h>
#include <facter/util/solaris/k_stat.hpp>

using namespace std;
using namespace facter::facts;
using namespace facter::facts::posix;
using namespace facter::util;
using namespace facter::util::solaris;

LOG_DECLARE_NAMESPACE("facts.solaris.processor");
/*
 * https://blogs.oracle.com/mandalika/entry/solaris_show_me_the_cpu
 * What we want to do is to count the distinct number of chip_id (#nproc),
 * then the distinct number of core_id (#ncores) and the number of instances
 * of hardware threads (given by valid procid).
 *
 * Our info comes from the following structure
 *
 $ kstat -m cpu_info
   module: cpu_info                        instance: 0
   name:   cpu_info0                       class:    misc
           brand                           Intel(r) Core(tm) i7-4850HQ CPU @ 2.30GHz
           cache_id                        0
           chip_id                         0
           clock_MHz                       2300
           clog_id                         0
           core_id                         0
           cpu_type                        i386
           crtime                          6.654772184
           current_clock_Hz                2294715939
           current_cstate                  0
           family                          6
           fpu_type                        i387 compatible
           implementation                  x86 (chipid 0x0 GenuineIntel family 6 model 70 step 1 clock 2300 MHz)
           model                           70
           ncore_per_chip                  1
           ncpu_per_chip                   1
           pg_id                           -1
           pkg_core_id                     0
           snaptime                        22631.883297199
           state                           on-line
           state_begin                     1409334365
           stepping                        1
           supported_frequencies_Hz        2294715939
           supported_max_cstates           1
           vendor_id                       GenuineIntel
 */
namespace facter { namespace facts { namespace solaris {

    void processor_resolver::resolve_structured_processors(collection& facts)
    {
        auto processors_value = make_value<map_value>();
        auto processor_list = make_value<array_value>();

        size_t hardware_thread_count = 0;
        unordered_set<int> cores;
        unordered_set<int> chips;

        k_stat kc;
        // man p_online(2) suggests that we have to iterate upto
        // SC_CPUID_MAX to determine the valid cpus.
        long max_cpuid = sysconf(_SC_CPUID_MAX);

        for (processorid_t p_id = 0; p_id < max_cpuid; ++p_id) {
            try {
                k_stat_entry ke = kc[{"cpu_info", p_id}];
                // we have a valid hardware thread so let us account for it.
                ++hardware_thread_count;

                auto brand = ke.value<string>("brand");
                processor_list->add(make_value<string_value>(brand));
                auto chip_id = ke.value<int32_t>("chip_id");
                chips.insert(chip_id);
                auto core_id = ke.value<int32_t>("core_id");
                cores.insert(core_id);
            } catch (exception& ex) {
                // not an error, the cpu_id passed was not valid
            }
        }

        // Add the model facts
        if (!processor_list->empty()) {
            processors_value->add("models", move(processor_list));
        }
        // Add the count facts
        if (!cores.empty()) {
            processors_value->add("count", make_value<integer_value>(cores.size()));
        }
        if (!chips.empty()) {
            processors_value->add("physicalcount", make_value<integer_value>(chips.size()));
        }
        if (hardware_thread_count> 0) {
            processors_value->add("hardwarethreads", make_value<integer_value>(hardware_thread_count));
        }

        if (!processors_value->empty()) {
            facts.add(fact::processors, move(processors_value));
        }
    }

}}}  // namespace facter::facts::solaris
