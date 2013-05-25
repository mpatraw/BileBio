
#ifndef THE_GAME_HPP
#define THE_GAME_HPP

#include <boost/noncopyable.hpp>
#include <map>
#include <vector>

#include <region.hpp>
#include <entity.hpp>
#include <plants.hpp>

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
        act_move,
    };

    player(region *reg, plant_manager *pm) :
        entity(reg), plant_manager_(pm)
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
    plant_manager *plant_manager_;
    attributes attributes_;
};

class the_game : private boost::noncopyable
{
public:
    the_game(std::function<void(entity *src, entity::did did)> on_action) :
        on_action_(on_action)
    {
        the_region_.reset(new region());
        the_region_->generate(50, 50);
        plant_manager_.reset(new plant_manager());
        the_player_.reset(new player(the_region_.get(), plant_manager_.get()));
        auto loc = the_region_->get_random_empty_location();
        the_player_->move_to(std::get<0>(loc), std::get<1>(loc));
    }

    const region &get_region() const { return *the_region_.get(); }
    const player &get_player() const { return *the_player_.get(); }

    void player_act(ssize_t dx, ssize_t dy, player::action act)
    {
        if (act == player::act_move)
            if (the_player_->move_by(dx, dy))
                on_action_(the_player_.get(), entity::did_move);
    }

    void rest_act()
    {

    }

    const plant *get_plant_at(ssize_t x, ssize_t y) const { return plant_manager_->get_plant(x, y); }

private:
    std::unique_ptr<region> the_region_;
    std::unique_ptr<plant_manager> plant_manager_;
    std::unique_ptr<player> the_player_;
    std::function<void(entity*src, entity::did did)> on_action_;
};

#endif
