// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#ifndef AOC_TERMIOS_HPP
#define AOC_TERMIOS_HPP

#include <errno.h>
#include <termios.h>

#include <system_error>

namespace posix {

termios tcgetattr(int fileDescriptor)
{
    termios out;
    if (tcgetattr(fileDescriptor, &out) == -1) {
        throw std::system_error(errno, std::generic_category());
    }
    return out;
}

// TODO: figure out optional_actions
void tcsetattr(int fileDescriptor, const termios & in, int optional_actions)
{
    if (tcsetattr(fileDescriptor, optional_actions, &in) == -1) {
        throw std::system_error(errno, std::generic_category());
    }
}

/// RAII wrapper for a modified termios state.
/// It is a common pattern to read termios with tcgetattr(), stash the original
/// away, modify it, and then restore the original later.  This class is an RAII
/// wrapper to encapsulate that behavior in an object which ensures that the
/// original is always restored.
class scoped_termios final
{
public:
    scoped_termios(int fileDescriptor, const termios & modified,
                   const termios & orig)
        : fileDescriptor(fileDescriptor), originalTermios(orig)
    {
        tcsetattr(fileDescriptor, modified, 0);
    }

    ~scoped_termios() { tcsetattr(fileDescriptor, originalTermios, 0); }

    static scoped_termios raw(int fileDescriptor)
    {
        const termios orig = tcgetattr(fileDescriptor);
        termios modified = orig;
        // XXX cfmakeraw() is not POSIX; consider emulating
        cfmakeraw(&modified);
        return scoped_termios(fileDescriptor, modified, orig);
    }

private:
    int fileDescriptor;
    termios originalTermios;
};

} // namespace posix

#endif // AOC_TERMIOS_HPP
