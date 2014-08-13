#include <facter/facts/posix/operating_system_resolver.hpp>
#include <facter/facts/posix/os.hpp>
#include <facter/facts/posix/os_family.hpp>
#include <facter/facts/scalar_value.hpp>
#include <facter/facts/map_value.hpp>
#include <facter/facts/collection.hpp>
#include <facter/facts/fact.hpp>
#include <re2/re2.h>
#include <map>

using namespace std;

namespace facter { namespace facts { namespace posix {

    operating_system_resolver::operating_system_resolver() :
        resolver(
            "operating system",
            {
                fact::operating_system,
                fact::os_family,
                fact::operating_system_release,
                fact::operating_system_major_release,
                fact::os
            })
    {
    }

    void operating_system_resolver::resolve_facts(collection& facts)
    {
        // Resolve all operating system related facts
        resolve_operating_system(facts);
        resolve_os_family(facts);
        resolve_operating_system_release(facts);
        resolve_operating_system_major_release(facts);
        resolve_os(facts);
    }

    void operating_system_resolver::resolve_operating_system(collection& facts)
    {
        // Default to the same value as the kernel
        auto kernel = facts.get<string_value>(fact::kernel);
        if (!kernel) {
            return;
        }

        facts.add(fact::operating_system, make_value<string_value>(kernel->value()));
    }

    void operating_system_resolver::resolve_os_family(collection& facts)
    {
        // Get the operating system fact
        auto os = facts.get<string_value>(fact::operating_system, false);
        string value;
        if (os) {
            static map<string, string> const systems = {
                { string(os::redhat),                   string(os_family::redhat) },
                { string(os::fedora),                   string(os_family::redhat) },
                { string(os::centos),                   string(os_family::redhat) },
                { string(os::scientific),               string(os_family::redhat) },
                { string(os::scientific_cern),          string(os_family::redhat) },
                { string(os::ascendos),                 string(os_family::redhat) },
                { string(os::cloud_linux),              string(os_family::redhat) },
                { string(os::psbm),                     string(os_family::redhat) },
                { string(os::oracle_linux),             string(os_family::redhat) },
                { string(os::oracle_vm_linux),          string(os_family::redhat) },
                { string(os::oracle_enterprise_linux),  string(os_family::redhat) },
                { string(os::amazon),                   string(os_family::redhat) },
                { string(os::xen_server),               string(os_family::redhat) },
                { string(os::linux_mint),               string(os_family::debian) },
                { string(os::ubuntu),                   string(os_family::debian) },
                { string(os::debian),                   string(os_family::debian) },
                { string(os::cumulus),                  string(os_family::debian) },
                { string(os::suse_enterprise_server),   string(os_family::suse) },
                { string(os::suse_enterprise_desktop),  string(os_family::suse) },
                { string(os::open_suse),                string(os_family::suse) },
                { string(os::suse),                     string(os_family::suse) },
                { string(os::solaris),                  string(os_family::solaris) },
                { string(os::nexenta),                  string(os_family::solaris) },
                { string(os::omni),                     string(os_family::solaris) },
                { string(os::open_indiana),             string(os_family::solaris) },
                { string(os::smart),                    string(os_family::solaris) },
                { string(os::gentoo),                   string(os_family::gentoo) },
                { string(os::archlinux),                string(os_family::archlinux) },
                { string(os::mandrake),                 string(os_family::mandrake) },
                { string(os::mandriva),                 string(os_family::mandrake) },
                { string(os::mageia),                   string(os_family::mandrake) },
            };
            auto const& it = systems.find(os->value());
            if (it != systems.end()) {
                value = it->second;
            }
        }

        if (value.empty()) {
            // Default to the same value as the kernel
            auto kernel = facts.get<string_value>(fact::kernel);
            if (!kernel) {
                return;
            }
            value = kernel->value();
        }
        facts.add(fact::os_family, make_value<string_value>(move(value)));
    }

    void operating_system_resolver::resolve_operating_system_release(collection& facts)
    {
        // Default to the same value as the kernelrelease fact
        auto release = facts.get<string_value>(fact::kernel_release);
        if (!release) {
            return;
        }

        facts.add(fact::operating_system_release, make_value<string_value>(release->value()));
    }

    void operating_system_resolver::resolve_os(collection& facts)
    {
        auto os = facts.get<string_value>(fact::operating_system, false);
        auto family = facts.get<string_value>(fact::os_family, false);
        auto release = facts.get<string_value>(fact::operating_system_release, false);
        auto os_value = make_value<map_value>();

        if (os) {
            os_value->add("name", make_value<string_value>(os->value()));

            if (family) {
                os_value->add("family", make_value<string_value>(family->value()));
            }

            if (release) {
                auto release_map = make_value<map_value>();
                string major;
                string minor;
                if (RE2::PartialMatch(release->value(), "^(\\d+)\\.?(\\d+)?", &major, &minor)) {
                    if (!major.empty()) {
                        release_map->add("major", make_value<integer_value>(stoi(major)));
                    }

                    if (!minor.empty()) {
                        release_map->add("minor", make_value<integer_value>(stoi(minor)));
                    }
                }

                release_map->add("full", make_value<string_value>(release->value()));
                os_value->add("release", move(release_map));
            }
        }

        if (!os_value->empty()) {
            facts.add(fact::os, move(os_value));
        }
    }

}}}  // namespace facter::facts::posix
