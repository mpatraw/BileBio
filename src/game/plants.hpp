
#ifndef PLANTS_HPP
#define PLANTS_HPP

#include <functional>
#include <map>
#include <memory>
#include <tuple>
#include <vector>

#include <entity.hpp>
#include <random.hpp>

using shared_plant = std::shared_ptr<class plant>;
using weak_plant = std::weak_ptr<class plant>;

class plant : public entity, private boost::noncopyable
{
public:
    enum type
    {
        p_growing,
        p_root,
        p_vine,
        p_flower,
        p_fruit,
    };

    plant(region *reg, rng *r, plant::type type, plant *parent=nullptr) :
        entity(reg, r), type_(type), parent_(parent)
    {
    }
    virtual ~plant()
    {
        std::printf("plant killed\n");
    }

    virtual plant::type get_type() const { return type_; }

    virtual void act(std::function<void(entity *, entity::did, entity *targ)> on_did)
    {
        (void)on_did;
    }

    virtual const plant *get_parent() const { return parent_; }

    virtual void grow_into(plant::type type)
    {
        type_ = type;
        switch (type_)
        {
        case plant::p_growing:
            vitals_ = {1, 1, 0, 0.0};
            break;

        case plant::p_vine:
            vitals_ = {3, 3, 0, 0.0};
            break;

        case plant::p_flower:
            vitals_ = {3, 3, 0, 0.0};
            break;

        case plant::p_fruit:
            vitals_ = {1, 1, 0, 0.0};
            break;

        default:
            break;
        }
    }

protected:
    plant::type type_;
    // Can't be shared. Would impose a circular reference.
    plant *parent_;
};

class plant_manager
{
public:
    using shared_plant_list = std::vector<shared_plant>;
    using shared_plant_map = std::map<vec2, shared_plant>;

    plant_manager() { }

    shared_plant get_shared(const plant *ptr)
    {
        for (auto i = plant_map_.begin(); i != plant_map_.end(); ++i)
        {
            if (ptr == i->second.get())
            {
                return i->second;
            }
        }
        return nullptr;
    }

    bool find_plant(shared_plant ptr, vec2 &c)
    {
        for (auto i = plant_map_.begin(); i != plant_map_.end(); ++i)
        {
            if (ptr == i->second)
            {
                c = i->first;
                return true;
            }
        }
        return false;
    }

    void remove_plant(shared_plant ptr)
    {
        vec2 c;
        if (find_plant(ptr, c))
            remove_plant(::get_x(c), ::get_y(c));
    }

    void add_plant(shared_plant p, ssize_t x, ssize_t y)
    {
        if (get_plant(x, y))
        {
            remove_plant(x, y);
        }
        plant_list_.push_back(p);
        plant_map_[vec2(x, y)] = p;
    }

    void add_plant_later(shared_plant p, ssize_t x, ssize_t y)
    {
        plant_later_.push_back({p, x, y});
    }

    void add_plants()
    {
        for (auto &p : plant_later_)
        {
            add_plant(p.ptr, p.x, p.y);
        }
        plant_later_.clear();
    }

    void remove_plant(ssize_t x, ssize_t y)
    {
        auto p = get_plant(x, y);
        if (p)
        {
            for (auto i = plant_list_.begin(); i != plant_list_.end(); ++i)
            {
                if (*i == p)
                {
                    plant_list_.erase(i);
                    break;
                }
            }
            plant_map_.erase(plant_map_.find(vec2(x, y)));
        }
    }

    void move_plant(ssize_t sx, ssize_t sy, ssize_t ex, ssize_t ey)
    {
        auto p = get_plant(sx, sy);
        remove_plant(sx, sy);
        add_plant(p, ex, ey);
    }

    shared_plant get_plant(ssize_t x, ssize_t y)
    {
        auto c = vec2(x, y);
        if (plant_map_.find(c) == plant_map_.end())
            return nullptr;
        else
            return plant_map_.at(c);
    }

    const shared_plant get_plant(ssize_t x, ssize_t y) const
    {
        auto c = vec2(x, y);
        if (plant_map_.find(c) == plant_map_.end())
            return nullptr;
        else
            return plant_map_.at(c);
    }

    shared_plant_list &get_plant_list() { return plant_list_; }
    const shared_plant_list &get_plant_list() const { return plant_list_; }

private:
    struct plant_later
    {
        shared_plant ptr;
        ssize_t x;
        ssize_t y;
    };

    std::vector<plant_later> plant_later_;
    shared_plant_list plant_list_;
    shared_plant_map plant_map_;
};

struct biomass
{
};

/*
1a. If growing
  1. Increment growth period for all plants growing
  2. If done growing
    1. Add plants on growth list to plant list
    2. Set to not growing
1b. If not growing
  1. Find N plants on the outer edges of the whole plant group.
  2a. Check if upgrade, or grow
    1. Replace and set new plant to grow
  2b. If grow instead
    1. Find target to grow, if none, pick random area unnoccupied.
  3. Add to growing list.
2. Update all plants to do something to target. (attack)
*/

class root : public plant, private boost::noncopyable
{
public:
    using plant_list = std::vector<std::shared_ptr<plant>>;

    root(region *reg, rng *r, plant_manager *pm, std::function<vec2()> target=std::function<vec2()>()) :
        plant(reg, r, plant::p_root), plant_manager_(pm), target_(target)
    {
        growth_time_ = 3;
        growth_timer_ = 0;
        growth_count_ = 1;
        growth_range_ = 5;
    }
    virtual ~root() { }

    virtual void set_target_function(std::function<vec2()> target)
    {
        target_ = target;
    }

    virtual void act(std::function<void(entity *, entity::did, entity *targ)> on_did)
    {
        (void)on_did;
        if (growing_list_.size())
        {
            if (++growth_timer_ >= growth_time_)
            {
                for (auto &p : growing_list_)
                {
                    p->grow_into(plant::p_vine);
                }
                growing_list_.clear();
                growth_timer_ = 0;
            }
        }
        else
        {
            if (child_list_.size())
            {
                plant_list edges;

                for (auto &p : child_list_)
                {
                    if (empty_neighbors(p).size() > 0)
                    {
                        edges.push_back(p);
                    }
                }

                ssize_t count = 0;
                for (auto &p : edges)
                {
                    if (++count > growth_count_)
                        break;

                    // 10% to transform.
                    if (rng_->get_uniform() < 0.1)
                    {
                    }
                    else
                    {
                        //if (target_)
                        //{
                        //    vec2 targ = target_();
                        //}
                        //else
                        {
                            auto empty = empty_neighbors(p);
                            auto r = rng_->get_range(0, empty.size() - 1);
                            auto np = std::make_shared<plant>(region_, rng_, plant::p_growing, this);
                            growing_list_.push_back(np);
                            child_list_.push_back(np);
                            plant_manager_->add_plant_later(np, ::get_x(empty[r]), ::get_y(empty[r]));
                        }
                    }
                    // if (grow)
                    // replace
                    // else (upgrade)
                    // grow near target or random
                    // add to growing list
                }
            }
            else
            {
                auto empty = empty_neighbors(plant_manager_->get_shared(this));
                if (empty.size() > 0)
                {
                    auto np = std::make_shared<plant>(region_, rng_, plant::p_growing, this);
                    auto r = rng_->get_range(0, empty.size() - 1);
                    growing_list_.push_back(np);
                    child_list_.push_back(np);
                    plant_manager_->add_plant_later(np, ::get_x(empty[r]), ::get_y(empty[r]));
                }
            }
        }

        std::vector<plant_list::iterator> to_delete;
        for (auto i = child_list_.begin(); i != child_list_.end(); ++i)
        {
            if ((*i)->is_dead())
            {
                to_delete.push_back(i);
            }
            else
            {
                // Do something to target.
            }
        }

        for (auto &i : to_delete)
            child_list_.erase(i);
    }

protected:
    plant_manager *plant_manager_;
    plant_list child_list_;
    plant_list growing_list_;
    std::function<vec2()> target_;
    ssize_t growth_time_;
    ssize_t growth_timer_;
    ssize_t growth_count_;
    ssize_t growth_range_;

private:
    std::vector<vec2> empty_neighbors(std::shared_ptr<plant> p, ssize_t range=1)
    {
        std::vector<vec2> neighbors;
        vec2 around;
        if (plant_manager_->find_plant(p, around))
        {
            for (ssize_t dx = -range; dx <= range; ++dx)
            {
                for (ssize_t dy = -range; dy <= range; ++dy)
                {
                    vec2 v = vec2(::get_x(around) + dx, ::get_y(around) + dy);
                    if (!plant_manager_->get_plant(::get_x(v), ::get_y(v)) &&
                        region_->walkable(::get_x(v), ::get_y(v)))
                        neighbors.push_back(v);
                }
            }
        }
        return neighbors;
    }

    std::vector<vec2> empty_neighbors(vec2 around, ssize_t range=1)
    {
        std::vector<vec2> neighbors;
        for (ssize_t dx = -range; dx <= range; ++dx)
        {
            for (ssize_t dy = -range; dy <= range; ++dy)
            {
                vec2 v = vec2(::get_x(around) + dx, ::get_y(around) + dy);
                if (!plant_manager_->get_plant(::get_x(v), ::get_y(v)) &&
                    region_->walkable(::get_x(v), ::get_y(v)))
                    neighbors.push_back(v);
            }
        }
        return neighbors;
    }
};

#endif
