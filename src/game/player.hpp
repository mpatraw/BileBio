
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
};

class player : public entity, private boost::noncopyable
{
private:
    using weak_ptr = std::weak_ptr<entity>;
    using shared_ptr = std::shared_ptr<entity>;
    using int_pair = std::pair<int, int>;
    using on_did_func = std::function<void(weak_ptr src, entity::did did, weak_ptr targ)>;
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
        entity(reg, r), entities_(pm)
    {
        vitals_ = {2, 3, 2, 0.66};
        attributes_ = {2, 3};
    }
    virtual ~player() { }

    virtual void perform(int_pair delta, player::action act, on_did_func on_did)
    {
        auto p = entities_->get_this_ptr(this);
        if (auto sptr = p.lock())
        {
            auto loc = entities_->get_coord(sptr);
            perform_to({loc.first + delta.first, loc.second + delta.second}, act, on_did);
        }
    }

    virtual void perform_to(int_pair loc, player::action act, on_did_func on_did)
    {
        int x = loc.first;
        int y = loc.second;
        switch (act)
        {
        case player::act_move:
            if (region_->in_bounds(x, y) && region_->tile_at(x, y) >= t_floor)
            {
                auto pl = entities_->get_ptr({x, y});
                auto sptr = pl.lock();
                // Move to location, nothing is there.
                if (!sptr)
                {
                    entities_->move_ptr_to(entities_->get_this_ptr(this).lock(), {x, y});
                    if (on_did != nullptr)
                        on_did(entities_->get_this_ptr(this), entity::did_move, shared_ptr());

                }
                // Attack (default).
                else
                {
                    if (rng_->get_uniform() < vitals_.to_hit)
                    {
                        std::printf("Dealt %d damage\n", vitals_.damage);
                        sptr->take_damage(vitals_.damage);
                        if (on_did != nullptr)
                            on_did(entities_->get_this_ptr(this), entity::did_attack, pl);
                        // Remove plant.
                        if (sptr->is_dead())
                            entities_->del_ptr(sptr);
                    }
                    else
                    {
                        std::printf("Missed.\n");
                        if (on_did != nullptr)
                            on_did(entities_->get_this_ptr(this), entity::did_miss, pl);
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
    sparse_2d_map<entity> *entities_;
    attributes attributes_;
};

#endif
