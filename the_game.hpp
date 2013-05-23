
#ifndef THE_GAME_HPP
#define THE_GAME_HPP

#include <boost/noncopyable.hpp>
#include <vector>

#include <drunkard.h>

enum ground
{
    g_dirt,
    g_grass
};

enum object
{
    o_none,
    o_rock,
    o_tree
};

struct tile
{
    ground g;
    object o;
};

class region : private boost::noncopyable
{
public:
    region() : width_(0), height_(0) { }
    ~region() { }

    void generate(size_t width, size_t height)
    {
        tiles_.clear();

        tiles_.resize(width);
        for (size_t x = 0; x < width; ++x)
            tiles_[x].resize(height);

        unsigned *map = new unsigned[width * height];
        memset(map, 0, sizeof(unsigned) * width * height);
        drunkard *drunk = drunkard_create(map, width, height);
        drunkard_set_open_threshold(drunk, 1);
        drunkard_seed(drunk, time(NULL));

        // Carve seed.
        drunkard_start_fixed(drunk, width / 2, height / 2);
        drunkard_mark_1(drunk, 1);
        drunkard_flush_marks(drunk);

        // Carve.
        int tries = std::min(width, height);
        while (tries --> 0)
        {
            drunkard_start_random(drunk);
            drunkard_target_random_opened(drunk);

            while (!drunkard_is_on_opened(drunk))
            {
                drunkard_mark_plus(drunk, 1);
                drunkard_step_to_target(drunk, 0.90);
            }

            drunkard_flush_marks(drunk);
        }

        for (size_t y = 0; y < height; ++y)
        {
            for (size_t x = 0; x < width; ++x)
            {
                std::cout << map[y * width + x];
                if (map[y * width + x])
                    tiles_[x][y].o = o_none;
                else
                    tiles_[x][y].o = o_tree;
            }
            std::cout << std::endl;
        }

        width_ = width;
        height_ = height;
        drunkard_destroy(drunk);
        delete []map;
    }

    size_t get_width() const { return width_; }
    size_t get_height() const { return height_; }

    bool in_bounds(ssize_t x, ssize_t y) const
    { return x >= 0 && x < (ssize_t)width_ && y >= 0 && y < (ssize_t)height_; }
    const tile &tile_at(ssize_t x, ssize_t y) const { return tiles_[x][y]; }
    tile &tile_at(ssize_t x, ssize_t y) { return tiles_[x][y]; }

private:
    size_t width_;
    size_t height_;
    std::vector<std::vector<tile>> tiles_;
};

class entity
{
public:
    entity(region *reg, std::vector<std::unique_ptr<entity>> *elist) :
        region_ref_(reg), entity_list_ref_(elist)
    { }
    virtual ~entity() { }

    virtual void move_by(ssize_t dx, ssize_t dy)
    {
        move_to(x_ + dx, y_ + dy);
    }

    virtual void move_to(ssize_t x, ssize_t y)
    {
        x_ = x;
        y_ = y;
    }

protected:
    ssize_t x_;
    ssize_t y_;

    region *region_ref_;
    std::vector<std::unique_ptr<entity>> *entity_list_ref_;
};

class player : public entity
{
public:
    player(region *reg, std::vector<std::unique_ptr<entity>> *elist) :
        entity(reg, elist)
    { }
    virtual ~player() { }
protected:

};

class the_game : public boost::noncopyable
{
public:
    the_game() { }

private:
    std::unique_ptr<region> the_region_;
    std::unique_ptr<player> the_player_;
    std::vector<std::unique_ptr<entity>> entity_list_;
};

#endif
