
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
        vitals_ = {2, 3, 2, 0.66};
        attributes_ = {2, 3, 2, 0.66};
    }
    virtual ~player() { }

    virtual const ssize_t &get_x() const { return x_; }
    virtual ssize_t &get_x() { return x_; }

    virtual const ssize_t &get_y() const { return y_; }
    virtual ssize_t &get_y() { return y_; }

    virtual void perform(ssize_t dx, ssize_t dy, player::action act, std::function<void(entity *, entity::did, entity *targ)> on_did)
    {
        perform_to(x_ + dx, y_ + dy, act, on_did);
    }

    virtual void perform_to(ssize_t x, ssize_t y, player::action act, std::function<void(entity *, entity::did, entity *targ)> on_did)
    {
        switch (act)
        {
        case player::act_move:
            if (region_->in_bounds(x, y) && region_->tile_at(x, y) >= t_floor)
            {
                auto p = plant_manager_->get_plant(x, y);
                if (!p)
                {
                    x_ = x;
                    y_ = y;
                    if (on_did != nullptr)
                        on_did(this, entity::did_move, nullptr);
                }
                else
                {
                    if (rng_->get_uniform() < vitals_.to_hit)
                    {
                        std::printf("Dealt %d damage\n", vitals_.damage);
                        p->take_damage(vitals_.damage);
                        if (on_did != nullptr)
                            on_did(this, entity::did_attack, p.get());
                        if (p->is_dead())
                            plant_manager_->remove_plant(p);
                    }
                    else
                    {
                        std::printf("Missed.\n");
                        if (on_did != nullptr)
                            on_did(this, entity::did_miss, p.get());
                    }
                }
            }
            break;

        default:
            break;
        }
        if (on_did != nullptr)
            on_did(this, entity::did_attack, nullptr);
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
