/**
 * @file
 * Declares the k_stat resource.
 */
#ifndef FACTER_UTIL_SOLARIS_K_STAT_HPP_
#define FACTER_UTIL_SOLARIS_K_STAT_HPP_

#include "scoped_kstat.hpp"

namespace facter { namespace util { namespace solaris {

    /**
     * Wrapper around the kstat_t structure.
     */
    struct k_stat_entry {
        /**
         * Default constructor. Should be created only by k_stat[] call.
         */
        k_stat_entry(kstat_t* kp);

        /*
         * Get unsigned long value out of our kstat_named_t
         * pointer. We might need other data types too when
         * other resolvers using kstat are added.
         */
        template <typename T>
        T value(const std::string& attrib);

     private:
        /**
         * Lookup the given attribute in the kstat structure.
         */
        kstat_named_t* lookup(const std::string& attrib);

        /**
         * The main data in the struct, obtained by a lookup
         */
        kstat_t* k_stat;
    };

    struct k_stat
    {
        /**
         * Default constructor.
         * This constructor will handle calling kstat_open.
         */
        k_stat();

        k_stat_entry operator[](std::pair<std::string, std::string>&& entry);

        k_stat_entry operator[](std::tuple<std::string, int, std::string>&& entry);

     private:
        scoped_kstat ctrl;
    };

}}}  // namespace facter::util::solaris

#endif  // FACTER_UTIL_SOLARIS_K_STAT_HPP_
