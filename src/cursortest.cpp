// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#include "ansiterm.hpp"
#include "termios.hpp"

#include <unistd.h>

#include <iostream>

int main(int /*argc*/, char ** /*argv*/)
{
    const auto original = posix::tcgetattr(STDIN_FILENO);
    auto modified = original;
    modified.c_lflag &= ~ECHO;
    modified.c_lflag &= ~ICANON;
    posix::tcsetattr(STDIN_FILENO, modified, 0);

    std::cout << ansi::dsr::get_cursor_position();
    auto maybeCpr = ansi::cpr::read(std::cin);

    posix::tcsetattr(STDIN_FILENO, original, 0);

    auto [row, column] = *maybeCpr;
    std::cout << row << ',' << column << '\n';

    return 0;
}
