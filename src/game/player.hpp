
#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <boost/noncopyable.hpp>

#include <entity.hpp>
#include <plants.hpp>
#include <random.hpp>

struct attributes
{
    int energy;
    int max_energy;
    int damage;
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

    player(region *reg, rng *r, sparse_2d_map<entity> *pm) :
        entity(reg, r), entity_manager_(pm)
    {
        vitals_ = {2, 3, 2, 0.66};
        attributes_ = {2, 3, 2, 0.66};
    }
    virtual ~player() { }

    virtual const int &get_x() const { return x_; }
    virtual int &get_x() { return x_; }

    virtual const int &get_y() const { return y_; }
    virtual int &get_y() { return y_; }

    virtual void perform(int dx, int dy, player::action act, std::function<void(std::weak_ptr<entity> src, entity::did did, std::weak_ptr<entity> targ)> on_did)
    {
        perform_to(x_ + dx, y_ + dy, act, on_did);
    }

    virtual void perform_to(int x, int y, player::action act, std::function<void(std::weak_ptr<entity> src, entity::did did, std::weak_ptr<entity> targ)> on_did)
    {
        switch (act)
        {
        case player::act_move:
            if (region_->in_bounds(x, y) && region_->tile_at(x, y) >= t_floor)
            {
                auto p = entity_manager_->get_ptr({x, y});
                if (!p.lock())
                {
                    x_ = x;
                    y_ = y;
                    entity_manager_->move_ptr_to(entity_manager_->get_this_ptr(this).lock(), {x, y});
                    if (on_did != nullptr)
                        on_did(entity_manager_->get_this_ptr(this), entity::did_move, std::shared_ptr<entity>());
                }
                else
                {
                    if (rng_->get_uniform() < vitals_.to_hit)
                    {
                        std::printf("Dealt %d damage\n", vitals_.damage);
                        p.lock()->take_damage(vitals_.damage);
                        if (on_did != nullptr)
                            on_did(entity_manager_->get_this_ptr(this), entity::did_attack, p);
                        if (p.lock()->is_dead())
                            entity_manager_->del_ptr(p.lock());
                    }
                    else
                    {
                        std::printf("Missed.\n");
                        if (on_did != nullptr)
                            on_did(entity_manager_->get_this_ptr(this), entity::did_miss, p);
                    }
                }
            }
            break;

        default:
            break;
        }

    }

    virtual const attributes &get_attributes() const { return attributes_; }
    virtual attributes &get_attributes() { return attributes_; }

protected:
    int x_;
    int y_;
    sparse_2d_map<entity> *entity_manager_;
    attributes attributes_;
};

#endif
