
#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <boost/noncopyable.hpp>

#include <entity.hpp>
#include <plants.hpp>
#include <random.hpp>

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
        act_special1,
        act_special2,
        act_special3,
        act_item
    };

    player(region *reg, rng *r, plant_manager *pm) :
        entity(reg, r), plant_manager_(pm)
    {
        attributes_ = {3, 3, 2, 0.66};
    }
    virtual ~player() { }

    virtual const ssize_t &get_x() const { return x_; }
    virtual ssize_t &get_x() { return x_; }

    virtual const ssize_t &get_y() const { return y_; }
    virtual ssize_t &get_y() { return y_; }

    virtual bool move_by(ssize_t dx, ssize_t dy)
    {
        return move_to(x_ + dx, y_ + dy);
    }

    virtual bool move_to(ssize_t x, ssize_t y)
    {
        if (region_->in_bounds(x, y) && region_->tile_at(x, y) >= t_dirt)
        {
            if (!plant_manager_->get_plant(x, y))
            {
                x_ = x;
                y_ = y;
                return true;
            }
        }
        return false;
    }

    virtual const attributes &get_attributes() const { return attributes_; }
    virtual attributes &get_attributes() { return attributes_; }

protected:
    ssize_t x_;
    ssize_t y_;
    plant_manager *plant_manager_;
    attributes attributes_;
};

#endif
