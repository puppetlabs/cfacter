#include <facter/util/solaris/k_stat.hpp>

using namespace std;

namespace facter { namespace util { namespace solaris {
    k_stat::k_stat() {
        if (ctrl == nullptr) {
            throw kstat_exception("kstat_open failed");
        }
    }

    k_stat_entry k_stat::operator[](pair<string,string>&& entry)
    {
        return operator[](std::tuple<string, int, string>(entry.first, 0, entry.second));
    }

    k_stat_entry k_stat::operator[](tuple<string, int, string>&& entry)
    {
        auto module = get<0>(entry);
        auto instance = get<1>(entry);
        auto klass = get<2>(entry);

        kstat_t* kp = kstat_lookup(ctrl, const_cast<char*>(module.c_str()), instance, const_cast<char *>(klass.c_str()));
        if (kp == nullptr) {
            throw kstat_exception("kstat_lookup failed");
        }

        if (kstat_read(ctrl, kp, 0) == -1) {
            throw kstat_exception("kstat_read failed");
        }
        return k_stat_entry(kp);
    }

    k_stat_entry::k_stat_entry(kstat_t* kp) :
        k_stat(kp)
    {
    }

    kstat_named_t* k_stat_entry::lookup(const string& attrib)
    {
        kstat_named_t* knp = reinterpret_cast<kstat_named_t*>(kstat_data_lookup(k_stat, const_cast<char*>(attrib.c_str())));
        if (knp == nullptr) {
            throw kstat_exception("kstat_data_lookup failed");
        }
        return knp;
    }

    template<> unsigned long k_stat_entry::value(const std::string& attrib) {
        return lookup(attrib)->value.ul;
    }

}}}  // namespace facter::util::solaris
