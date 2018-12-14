// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#ifndef AOC_ANSITERM_HPP
#define AOC_ANSITERM_HPP

#include <cstdint>
#include <iostream>

// XXX It is probably not a good idea for me to name a namespace "ansi".
namespace ansi {

class cursor
{
    friend std::ostream & operator<<(std::ostream &, const cursor &);

public:
    enum class direction : char
    {
        up = 'A',
        down = 'B',
        right = 'C',
        left = 'D',
    };

    cursor(direction dir, uint16_t spaces) : dir(dir), spaces(spaces) {}

private:
    direction dir;
    std::uint16_t spaces;
};

// Stream insertion operator for cursor.
std::ostream & operator<<(std::ostream & stream, const cursor & cur)
{
    stream << "\033[" << cur.spaces << static_cast<char>(cur.dir);
    return stream;
}

class graphic
{
    friend std::ostream & operator<<(std::ostream &, const graphic &);

public:
    enum class sgr_code : uint8_t
    {
        reset = 0,
        black = 30,
        red = 31,
        green = 32,
        yellow = 33,
        blue = 34,
        magenta = 35,
        cyan = 36,
        white = 37,
    };

    enum class color3 : uint8_t
    {
        black = 30,
        red = 31,
        green = 32,
        yellow = 33,
        blue = 34,
        magenta = 35,
        cyan = 36,
        white = 37,
    };

    static graphic reset() { return graphic(sgr_code::reset); }
    static graphic fg_color(color3 color)
    {
        return graphic(static_cast<sgr_code>(color));
    }
    static graphic bg_color(color3 color)
    {
        return graphic(static_cast<sgr_code>(static_cast<int>(color) + 10));
    }

private:
    graphic(sgr_code code) : code(code) {}

    sgr_code code;
};

// Stream insertion operator for graphic.
std::ostream & operator<<(std::ostream & stream, const graphic & gr)
{
    stream << "\033[" << static_cast<int>(gr.code) << 'm';
    return stream;
}

} // namespace ansi

#endif // AOC_ANSITERM_HPP
