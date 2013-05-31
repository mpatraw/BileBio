
#ifndef DIRECTIONAL_HPP
#define DIRECTIONAL_HPP

#include <cmath>

constexpr double pi = std::atan2(0, -1);

constexpr double deg2rad(double deg) { return deg * pi / 180; }
constexpr double rad2deg(double rad) { return rad * 180 / pi; }
constexpr const char *direction_strings[] = {
    "south", "southeast", "east", "northeast",
    "north", "northwest", "west", "southwest",
    "none"
};

using direction = int;
constexpr int dir_none = -1;
constexpr int dir_south = 0;
constexpr int dir_southeast = 1;
constexpr int dir_east = 2;
constexpr int dir_northeast = 3;
constexpr int dir_north = 4;
constexpr int dir_northwest = 5;
constexpr int dir_west = 6;
constexpr int dir_southwest = 7;

constexpr const char *get_direction_string(direction dir)
{
    return direction_strings[dir];
}

inline direction get_direction(int dx, int dy, int dirs=4)
{
    if (dx == 0 && dy == 0)
        return dir_none;
    // Prefers north/south when the direction is diagonal.

    double thresh = 45.0;
    int inc = 2;
    if (dirs == 8)
    {
        thresh = 22.5;
        inc = 1;
    }

    auto degrees = rad2deg(atan2(dx, dy));
    if (degrees < 0)
        degrees = 180 + (180 - std::abs(degrees));
    direction dir = dir_south;

    while (degrees > thresh)
    {
        degrees -= thresh * 2;
        dir = (dir + inc) % 8;
    }

    return dir;
}

#endif
