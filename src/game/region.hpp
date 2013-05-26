
#ifndef REGION_HPP
#define REGION_HPP

#include <iostream>
#include <tuple>
#include <vector>

#include <boost/noncopyable.hpp>

#include <drunkard.h>

template <typename... Args>
inline auto get_x(Args&&... args) -> decltype(std::get<0>(std::forward<Args>(args)...))
{
    return std::get<0>(std::forward<Args>(args)...);
}

template <typename... Args>
inline auto get_y(Args&&... args) -> decltype(std::get<1>(std::forward<Args>(args)...))
{
    return std::get<1>(std::forward<Args>(args)...);
}

using vec2 = std::tuple<ssize_t, ssize_t>;

enum tile
{
    t_rocks,
    t_floor,
};

class region : private boost::noncopyable
{
public:
    region() : width_(0), height_(0), drunk_(nullptr) { }
    ~region()
    {
        if (drunk_)
            drunkard_destroy(drunk_);
    }

    void generate(size_t width, size_t height)
    {
        tiles_.resize(width * height, t_rocks);

        if (drunk_)
            drunkard_destroy(drunk_);

        drunk_ = drunkard_create((unsigned *)tiles_.data(), width, height);
        drunkard_set_open_threshold(drunk_, t_floor);
        drunkard_seed(drunk_, time(nullptr));

        // Carve seed.
        drunkard_start_fixed(drunk_, width / 2, height / 2);
        drunkard_mark_1(drunk_, t_floor);
        drunkard_flush_marks(drunk_);

        // Carve.
        int tries = std::min(width, height);
        while (tries --> 0)
        {
            drunkard_start_random(drunk_);
            drunkard_target_random_opened(drunk_);

            while (!drunkard_is_on_opened(drunk_))
            {
                drunkard_mark_plus(drunk_, t_floor);
                drunkard_step_to_target(drunk_, 0.90);
            }

            drunkard_flush_marks(drunk_);
        }

        width_ = width;
        height_ = height;

        for (size_t y = 0; y < height; ++y)
        {
            for (size_t x = 0; x < width; ++x)
            {
                if (tile_at(x, y) >= t_floor)
                    std::cout << ".";
                else
                    std::cout << "#";
            }
            std::cout << std::endl;
        }
    }

    vec2 get_random_empty_location()
    {
        unsigned x, y;
        drunkard_random_opened(drunk_, &x, &y);
        return vec2(x, y);
    }

    size_t get_width() const { return width_; }
    size_t get_height() const { return height_; }

    bool walkable(ssize_t x, ssize_t y) const { return tile_at(x, y) >= t_floor; }
    bool in_bounds(ssize_t x, ssize_t y) const
    { return x >= 0 && x < (ssize_t)width_ && y >= 0 && y < (ssize_t)height_; }
    const tile &tile_at(ssize_t x, ssize_t y) const { return tiles_[y * width_ + x]; }
    tile &tile_at(ssize_t x, ssize_t y) { return tiles_[y * width_ + x]; }

private:
    size_t width_;
    size_t height_;
    drunkard *drunk_;
    std::vector<tile> tiles_;
};

#endif
