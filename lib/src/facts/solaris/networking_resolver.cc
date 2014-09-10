#include <facter/facts/solaris/networking_resolver.hpp>
#include <facter/facts/collection.hpp>
#include <facter/facts/fact.hpp>
#include <facter/facts/scalar_value.hpp>
#include <facter/execution/execution.hpp>
#include <facter/util/file.hpp>
#include <facter/util/directory.hpp>
#include <facter/util/string.hpp>
#include <facter/util/solaris/io_ctl.hpp>
#include <facter/logging/logging.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <sstream>
#include <cstring>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/sockio.h>
#include <sys/socket.h>
#include <net/if_arp.h>

using namespace std;
using namespace facter::util;
using namespace facter::util::solaris;
using namespace facter::util::posix;
using namespace facter::execution;

LOG_DECLARE_NAMESPACE("facts.solaris.networking");

namespace facter { namespace facts { namespace solaris {
    void networking_resolver::resolve_interface_facts(collection& facts)
    {
        try {
            struct lifnum ifnr{.lifn_family = AF_UNSPEC, .lifn_flags = 0, .lifn_count = 0};
            io_ctl ctl(socket(AF_INET, SOCK_DGRAM, 0));

            // (patterned on bsd impl)
            ctl.apply(SIOCGLIFNUM, ifnr);
            vector<char> buffer(ifnr.lifn_count * sizeof(struct lifreq));
            struct lifconf lifc = {
                .lifc_family = AF_UNSPEC,
                .lifc_flags = 0,
                .lifc_len = static_cast<int>(buffer.size()),
                {.lifcu_buf = buffer.data()}};

            ctl.apply(SIOCGLIFCONF, lifc);
            struct lifreq* ptr = reinterpret_cast<struct lifreq*>(lifc.lifc_buf);
            struct lifreq* endif = ptr + ifnr.lifn_count;

            // put them in a multimap so that similar address can be
            // grouped together.
            multimap<string, const lifreq*> interface_map;
            for (; ptr < endif; ptr++) {
                interface_map.insert({ptr->lifr_name, ptr});
            }
            ostringstream interfaces;

            string primary_interface = get_primary_interface();
            if (primary_interface.empty()) {
                LOG_DEBUG("no primary interface found.");
            }

            auto dhcp_servers_value = make_value<map_value>();


            // Walk the interfaces
            decltype(interface_map.begin()) addr_it;
            for (auto it = interface_map.begin(); it != interface_map.end(); it = addr_it) {
                string const& interface = it->first;
                bool primary = interface == primary_interface;

                auto range = interface_map.equal_range(it->first);
                // Walk the addresses again and resolve facts
                for (addr_it = range.first; addr_it != range.second; ++addr_it) {
                    const struct lifreq *addr = addr_it->second;

                    resolve_links(facts, addr, primary);
                    resolve_address(facts, addr, primary);
                    resolve_network(facts, addr, primary);
                    resolve_mtu(facts, addr);
                }

                string dhcp_server = find_dhcp_server(interface);
                if (!dhcp_server.empty()) {
                    if (primary) {
                        dhcp_servers_value->add("system", make_value<string_value>(dhcp_server));
                    }
                    dhcp_servers_value->add(string(interface), make_value<string_value>(move(dhcp_server)));
                }

                // Add the interface to the interfaces fact
                if (interfaces.tellp() != 0) {
                    interfaces << ",";
                }

                interfaces << interface;
            }

            if (LOG_IS_WARNING_ENABLED() && primary_interface.empty()) {
                LOG_WARNING("no primary interface found: facts %1%, %2%, %3%, %4%, %5%, %6%, and %7% are unavailable.",
                        fact::ipaddress, fact::ipaddress6,
                        fact::netmask, fact::netmask6,
                        fact::network, fact::network6,
                        fact::macaddress);
            }

            // Add the DHCP servers fact
            if (!dhcp_servers_value->empty()) {
                facts.add(fact::dhcp_servers, move(dhcp_servers_value));
            }

            string value = interfaces.str();
            if (value.empty()) {
                return;
            }
            facts.add(fact::interfaces, make_value<string_value>(move(value)));
        } catch (io_ctl_exception& ex) {
            LOG_DEBUG("resolve_interface_facts failed %1% %2%", ex.what(), strerror(errno));
        }
    }

    void networking_resolver::resolve_links(collection& facts, lifreq const* addr, bool primary)
    {
        try {
            if (addr->lifr_addr.ss_family != AF_INET) {
                return;
            }

            io_ctl ctl(socket(addr->lifr_addr.ss_family, SOCK_DGRAM, 0));

            arpreq arp;
            struct sockaddr_in *arp_addr = reinterpret_cast<struct sockaddr_in*>(&arp.arp_pa);
            const struct sockaddr_in *laddr = reinterpret_cast<const struct sockaddr_in*>(&addr->lifr_addr);

            arp_addr->sin_addr.s_addr = laddr->sin_addr.s_addr;

            ctl.apply(SIOCGARP, arp);

            unsigned char* bytes = reinterpret_cast<unsigned char*>(arp.arp_ha.sa_data);
            address_map.insert({reinterpret_cast<const sockaddr*>(&addr->lifr_addr), bytes});

            string address = macaddress_to_string(bytes);
            string factname = fact::macaddress;
            string interface_factname = factname + "_" + addr->lifr_name;

            if (primary) {
                facts.add(move(factname), make_value<string_value>(address));
            }
            facts.add(move(interface_factname), make_value<string_value>(move(address)));
        } catch (io_ctl_exception& ex) {
            LOG_DEBUG("resolve_links failed %1% %2%", ex.what(), strerror(errno));
        }
    }

    void networking_resolver::resolve_address(collection& facts, lifreq const* addr, bool primary)
    {
        string factname = addr->lifr_addr.ss_family == AF_INET ? fact::ipaddress : fact::ipaddress6;
        string address = address_to_string((const sockaddr*)&addr->lifr_addr);

        string interface_factname = factname + "_" + addr->lifr_name;

        if (address.empty()) {
            return;
        }

        if (primary) {
            facts.add(move(factname), make_value<string_value>(address));
        }

        facts.add(move(interface_factname), make_value<string_value>(move(address)));
    }

    void networking_resolver::resolve_network(collection& facts, lifreq const* addr, bool primary)
    {
        try {
            io_ctl ctl(socket(addr->lifr_addr.ss_family, SOCK_DGRAM, 0));
            ctl.apply(SIOCGLIFNETMASK, *addr);

            // Set the netmask fact
            string factname = addr->lifr_addr.ss_family == AF_INET ? fact::netmask : fact::netmask6;
            string netmask = address_to_string((const struct sockaddr*) &addr->lifr_addr);

            string interface_factname = factname + "_" + addr->lifr_name;

            if (primary) {
                facts.add(move(factname), make_value<string_value>(netmask));
            }

            facts.add(move(interface_factname), make_value<string_value>(move(netmask)));

            // Set the network fact
            factname = addr->lifr_addr.ss_family == AF_INET ? fact::network : fact::network6;
            string network = address_to_string((const struct sockaddr*)&addr->lifr_addr, (const struct sockaddr*)&addr->lifr_broadaddr);
            interface_factname = factname + "_" + addr->lifr_name;

            if (primary) {
                facts.add(move(factname), make_value<string_value>(network));
            }

            facts.add(move(interface_factname), make_value<string_value>(move(network)));
        } catch (io_ctl_exception& ex) {
            LOG_DEBUG("resolve_network failed %1% %2%", ex.what(), strerror(errno));
        }
    }

    void networking_resolver::resolve_mtu(collection& facts, lifreq const* addr)
    {
        try {
            io_ctl ctl(socket(addr->lifr_addr.ss_family, SOCK_DGRAM, 0));
            ctl.apply(SIOCGLIFMTU, *addr);

            int mtu = get_link_mtu(string(addr->lifr_name), const_cast<lifreq*>(addr));
            if (mtu == -1) {
                return;
            }
            facts.add(string(fact::mtu) + '_' + addr->lifr_name, make_value<string_value>(to_string(mtu)));
        } catch (io_ctl_exception& ex) {
            LOG_DEBUG("resolve_mtu failed %1% %2%", ex.what(), strerror(errno));
        }
    }

    string networking_resolver::get_primary_interface()
    {
        string value;
        execution::each_line("netstat", { "-rn"}, [&value](string& line) {
            boost::trim(line);
            if (boost::starts_with(line, "default")) {
                vector<string> fields;
                boost::split(fields, line, boost::is_space(), boost::token_compress_on);
                value = fields[5];
                return false;
            }
            return true;
        });
        return value;
    }

    bool networking_resolver::is_link_address(const sockaddr* addr) const
    {
        return address_map.find(addr) != address_map.end();
    }

    uint8_t const* networking_resolver::get_link_address_bytes(const sockaddr * addr) const
    {
        auto ibytes = address_map.find(addr);
        if (ibytes != address_map.end()) {
            return ibytes->second;
        } else {
            return nullptr;
        }
    }

    int networking_resolver::get_link_mtu(std::string const& interface, void* data) const
    {
        return reinterpret_cast<lifreq*>(data)->lifr_metric;
    }

    string networking_resolver::find_dhcp_server(string const& interface)
    {
        // Use ipconfig to get the server identifier
        auto result = execute("dhcpinfo", { "-i", interface, "ServerID" });
        if (!result.first) {
            return {};
        }
        return result.second;
    }

}}}  // namespace facter::facts::solaris
