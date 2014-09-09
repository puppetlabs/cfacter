/**
 * @file
 * Declares a simple ioctl wrapper that handles descriptors correctly, and
 * applies ioctl actions on them.
 */
#ifndef FACTER_UTIL_SOLARIS_IO_CTL_HPP_
#define FACTER_UTIL_SOLARIS_IO_CTL_HPP_

#include <facter/util/posix/scoped_descriptor.hpp>
#include <sys/ioctl.h>

namespace facter { namespace util { namespace solaris {
    /**
     * The exception that gets thrown for any io_ctl failure.
     */
    struct io_ctl_exception : std::runtime_error
    {
        /**
         * Constructs a io_ctl exception.
         * @param message The exception message.
         */
        explicit io_ctl_exception(std::string const& message);
    };

    /**
     * A simple wrapper over ioctl system call that manages the descriptor,
     * and converts returned errors to exceptions so that they can be handled
     * at one place.
     */
    struct io_ctl
    {
        /**
         * The io_ctl constructor.
         */
        io_ctl(int desc);

        /**
         * Runs the ioctl request on the passed argument, using our descriptor.
         */
        template <typename T>
        void apply(int req, T& arg);

     private:
        /*
         * Holds the descriptor passed in, and manages its lifetime.
         */
        facter::util::posix::scoped_descriptor descriptor;
    };

    template <typename T>
    void io_ctl::apply(int req, T& arg)
    {
        if (ioctl(descriptor, req, &arg) == -1) {
            throw io_ctl_exception("ioctl failed");
        }
    }
}}}  // namespace facter::util::solaris

#endif  // FACTER_UTIL_SOLARIS_IO_CTL_HPP_
