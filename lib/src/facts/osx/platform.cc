#include <facter/facts/fact_map.hpp>
#include <facter/facts/posix/kernel_resolver.hpp>
#include <facter/facts/posix/operating_system_resolver.hpp>
#include <facter/facts/osx/networking_resolver.hpp>
#include <facter/facts/osx/processor_resolver.hpp>
#include <facter/facts/osx/dmi_resolver.hpp>
#include <facter/facts/bsd/uptime_resolver.hpp>

using namespace std;

namespace facter { namespace facts {

    void populate_platform_facts(fact_map& facts)
    {
        facts.add(make_shared<posix::kernel_resolver>());
        facts.add(make_shared<posix::operating_system_resolver>());
        facts.add(make_shared<bsd::uptime_resolver>());
        facts.add(make_shared<osx::networking_resolver>());
        facts.add(make_shared<osx::processor_resolver>());
        facts.add(make_shared<osx::dmi_resolver>());
    }

}}  // namespace facter::facts
