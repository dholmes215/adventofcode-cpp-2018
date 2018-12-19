// Copyright (C) 2018 David Holmes <dholmes@dholmes.us>. All rights reserved.

#ifndef AOC_ANSITERM_HPP
#define AOC_ANSITERM_HPP

#include "termios.hpp"

#include <array>
#include <charconv>
#include <cstdint>
#include <iostream>
#include <optional>
#include <regex>
#include <string_view>

// XXX It is probably not a good idea for me to name a namespace "ansi".
namespace ansi {

using std::int16_t;
using std::uint16_t;
using std::uint8_t;

using std::array;
using std::istream;
using std::optional;
using std::ostream;
using std::string_view;

using term_coordinate = int16_t;
using term_row = term_coordinate;
using term_col = term_coordinate;

struct cursor_position
{
    term_col x;
    term_row y;

    cursor_position() : x(1), y(1) {}
    cursor_position(term_col x, term_row y) : x(x), y(y) {}

    cursor_position & operator+=(const cursor_position & other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    cursor_position operator+(const cursor_position & other) const
    {
        cursor_position out = other;
        out += *this;
        return out;
    }
};

class cursor
{
    friend ostream & operator<<(ostream &, const cursor &);

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
    uint16_t spaces;
};

// Stream insertion operator for cursor.
ostream & operator<<(ostream & stream, const cursor & cur)
{
    stream << "\033[" << cur.spaces << static_cast<char>(cur.dir);
    return stream;
}

class cup
{
    friend ostream & operator<<(ostream &, const cup &);

public:
    cup(cursor_position pos) : pos(pos) {}

private:
    cursor_position pos;
};

// Stream insertion operator for cup.
ostream & operator<<(ostream & stream, const cup & c)
{
    stream << "\033[" << c.pos.y << ';' << c.pos.x << 'H';
    return stream;
}

class graphic
{
    friend ostream & operator<<(ostream &, const graphic &);

public:
    enum class sgr_code : uint8_t
    {
        reset = 0,
        bold = 1,
        faint = 2,
        italic = 3,
        underline = 4,
        slow_blink = 5,
        rapid_blink = 6,
        reverse_video = 7,
        conceal = 8,
        crossed_out = 9,
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

    static graphic bold() { return graphic(sgr_code::bold); }
    static graphic faint() { return graphic(sgr_code::faint); }
    static graphic italic() { return graphic(sgr_code::italic); }
    static graphic underline() { return graphic(sgr_code::underline); }
    static graphic reverse_video() { return graphic(sgr_code::reverse_video); }
    static graphic crossed_out() { return graphic(sgr_code::crossed_out); }

private:
    graphic(sgr_code code) : code(code) {}

    sgr_code code;
};

// Stream insertion operator for graphic.
ostream & operator<<(ostream & stream, const graphic & gr)
{
    stream << "\033[" << static_cast<int>(gr.code) << 'm';
    return stream;
}

// Device Status Report
class dsr
{
    friend ostream & operator<<(ostream &, const dsr &);

public:
    static dsr get_cursor_position()
    {
        dsr d;
        d.code = 6;
        return d;
    }

private:
    uint8_t code;
};

// Stream insertion operator for dsr.
ostream & operator<<(ostream & stream, const dsr & dsr)
{
    stream << "\033[" << static_cast<int>(dsr.code) << 'n';
    return stream;
}

// CPR - Active Position Report (ECMA-48 8.3.14)
struct cpr
{
public:
    static optional<cpr> read(istream & stream);
    cursor_position pos;

private:
    cpr() = default;
    static optional<cpr> parse(string_view str);
};

// XXX Need to fiddle with termios for reading this to actually work.
optional<cpr> cpr::read(istream & stream)
{
    auto pos = stream.tellg();
    array<char, 11> chars;
    stream.getline(chars.begin(), chars.size(), 'R');
    string_view str(chars.data(), chars.size());
    auto maybeCpr = cpr::parse(str);
    if (!maybeCpr) {
        stream.seekg(pos);
    }
    return maybeCpr;
}

// TODO: Does the failure case actually work? Test it
optional<cpr> cpr::parse(string_view str)
{
    static std::regex cprRegex("\\x1b\\[(\\d{1,3});(\\d{1,3})");
    std::cmatch match;
    if (std::regex_search(str.begin(), str.end(), match, cprRegex)) {
        cpr out;
        std::from_chars(match[1].first, match[1].second, out.pos.y);
        std::from_chars(match[2].first, match[2].second, out.pos.x);
        return out;
    } else {
        return {};
    }
}

optional<cursor_position> get_cursor_position(istream & in, ostream & out)
{
    out << ansi::dsr::get_cursor_position();
    auto maybeCpr = ansi::cpr::read(in);
    if (maybeCpr) {
        return maybeCpr->pos;
    } else {
        return {};
    }
}

} // namespace ansi

#endif // AOC_ANSITERM_HPP
