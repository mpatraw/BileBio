
#ifndef THE_GAME_HPP
#define THE_GAME_HPP

#include <boost/noncopyable.hpp>
#include <tuple>
#include <map>
#include <vector>

#include <drunkard.h>

enum tile
{
    t_water,
    t_tree,
    t_rock,
    t_dirt,
    t_grass,
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
        tiles_.resize(width * height, t_water);
        memset(tiles_.data(), 0, sizeof(tile) * width * height);

        if (drunk_)
            drunkard_destroy(drunk_);

        drunk_ = drunkard_create((unsigned *)tiles_.data(), width, height);
        drunkard_set_open_threshold(drunk_, t_dirt);
        drunkard_seed(drunk_, time(NULL));

        // Carve seed.
        drunkard_start_fixed(drunk_, width / 2, height / 2);
        drunkard_mark_1(drunk_, t_dirt);
        drunkard_flush_marks(drunk_);

        // Carve.
        int tries = std::min(width, height);
        while (tries --> 0)
        {
            drunkard_start_random(drunk_);
            drunkard_target_random_opened(drunk_);

            while (!drunkard_is_on_opened(drunk_))
            {
                drunkard_mark_plus(drunk_, t_dirt);
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
                if (tile_at(x, y) >= t_dirt)
                    std::cout << ".";
                else
                    std::cout << "#";
            }
            std::cout << std::endl;
        }
    }

    std::tuple<size_t, size_t> get_random_empty_location()
    {
        unsigned x, y;
        drunkard_random_opened(drunk_, &x, &y);
        return std::make_tuple<size_t, size_t>(x, y);
    }

    size_t get_width() const { return width_; }
    size_t get_height() const { return height_; }

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

struct coord
{
    ssize_t x;
    ssize_t y;
};

struct vitals
{
    ssize_t hearts;
    ssize_t max_hearts;
};

class entity : private boost::noncopyable
{
public:
    enum action
    {
        act_move,
        act_grow,
        act_attack,
    };
    entity(region *reg) :
        region_(reg)
    {
    }
    virtual ~entity() { }

    virtual const coord &get_coord() const { return coord_; }
    virtual coord &get_coord() { return coord_; }

    virtual const vitals &get_vitals() const { return vitals_; }
    virtual vitals &get_vitals() { return vitals_; }

    virtual bool is_dead() const { return vitals_.hearts <= 0; }
    virtual bool is_alive() const { return !is_dead(); }
protected:
    coord coord_;
    vitals vitals_;

    region *region_;
};

struct biomass
{
};

class plant : public entity, private boost::noncopyable
{
public:
    plant(region *reg, std::vector<std::shared_ptr<plant>> *rlist, std::map<std::tuple<ssize_t, ssize_t>, std::shared_ptr<plant>> *pmap) :
        entity(reg), root_list_(rlist), plant_map_(pmap)
    {
        parent_ = nullptr;
    }
    virtual ~plant() { }

    virtual bool is_root() const { return parent_ == nullptr; }
    virtual bool is_child() const { return !is_root(); }

protected:
    std::vector<std::shared_ptr<plant>> *root_list_;
    std::map<std::tuple<ssize_t, ssize_t>, std::shared_ptr<plant>> *plant_map_;

    std::shared_ptr<plant> parent_;
    std::vector<std::shared_ptr<plant>> connected_to_;
};

struct attributes
{
    ssize_t energy;
    ssize_t max_energy;
    ssize_t damage;
    double chance_to_hit;
};

class player : public entity, private boost::noncopyable
{
public:
    enum action
    {
        act_none,
        act_moved,
    };

    player(region *reg, std::vector<std::shared_ptr<plant>> *rlist, std::map<std::tuple<ssize_t, ssize_t>, std::shared_ptr<plant>> *pmap) :
        entity(reg), root_list_(rlist), plant_map_(pmap)
    { }
    virtual ~player() { }

    virtual bool move_by(ssize_t dx, ssize_t dy)
    {
        return move_to(coord_.x + dx, coord_.y + dy);
    }

    virtual bool move_to(ssize_t x, ssize_t y)
    {
        if (region_->in_bounds(x, y) && region_->tile_at(x, y) >= t_dirt)
        {
            coord_ = {x, y};
            return true;
        }
        return false;
    }

    virtual const attributes &get_attributes() const { return attributes_; }
    virtual attributes &get_attributes() { return attributes_; }
protected:
    std::vector<std::shared_ptr<plant>> *root_list_;
    std::map<std::tuple<ssize_t, ssize_t>, std::shared_ptr<plant>> *plant_map_;
    attributes attributes_;
};

class the_game : private boost::noncopyable
{
public:
    the_game(std::function<void(entity *src, entity::action act)> on_action) :
        on_action_(on_action)
    {
        the_region_.reset(new region());
        the_region_->generate(50, 50);
        the_player_.reset(new player(the_region_.get(), &root_list_, &plant_map_));
        auto loc = the_region_->get_random_empty_location();
        the_player_->move_to(std::get<0>(loc), std::get<1>(loc));
    }

    const region &get_region() const { return *the_region_.get(); }
    const player &get_player() const { return *the_player_.get(); }

    void move_player_by(ssize_t dx, ssize_t dy)
    {
        if (the_player_->move_by(dx, dy))
            on_action_(the_player_.get(), entity::act_move);
    }

private:
    std::unique_ptr<region> the_region_;
    std::unique_ptr<player> the_player_;
    std::function<void(entity*src, entity::action act)> on_action_;

    std::vector<std::shared_ptr<plant>> root_list_;
    std::map<std::tuple<ssize_t, ssize_t>, std::shared_ptr<plant>> plant_map_;
};

#endif
