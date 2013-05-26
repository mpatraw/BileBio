
#ifndef PLANTS_HPP
#define PLANTS_HPP

#include <functional>
#include <map>
#include <memory>
#include <tuple>
#include <vector>

#include <entity.hpp>
#include <random.hpp>

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

    plant(region *reg, rng *r, plant::type type) :
        entity(reg, r), type_(type)
    {
    }
    virtual ~plant() { }

    virtual plant::type get_type() const { return type_; }

    virtual void act(std::function<void(entity *, entity::did)> on_did) = 0;

protected:
    plant::type type_;
};

class plant_manager
{
public:
    using plant_ptr = std::shared_ptr<plant>;
    using plant_ptr_list = std::vector<plant_ptr>;
    using plant_ptr_map = std::map<vec2, plant_ptr>;

    plant_manager() { }

    plant_ptr get_shared(plant *ptr)
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

    bool find_plant(plant_ptr ptr, vec2 &c)
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

    void remove_plant(plant_ptr ptr)
    {
        vec2 c;
        if (find_plant(ptr, c))
            remove_plant(::get_x(c), ::get_y(c));
    }

    void add_plant(plant_ptr p, ssize_t x, ssize_t y)
    {
        if (get_plant(x, y))
        {
            remove_plant(x, y);
        }
        plant_list_.push_back(p);
        plant_map_[vec2(x, y)] = p;
    }

    void add_plant_later(plant_ptr p, ssize_t x, ssize_t y)
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

    plant_ptr get_plant(ssize_t x, ssize_t y)
    {
        auto c = vec2(x, y);
        if (plant_map_.find(c) == plant_map_.end())
            return nullptr;
        else
            return plant_map_.at(c);
    }

    const plant_ptr get_plant(ssize_t x, ssize_t y) const
    {
        auto c = vec2(x, y);
        if (plant_map_.find(c) == plant_map_.end())
            return nullptr;
        else
            return plant_map_.at(c);
    }

    plant_ptr_list &get_plant_list() { return plant_list_; }
    const plant_ptr_list &get_plant_list() const { return plant_list_; }

private:
    struct plant_later
    {
        plant_ptr ptr;
        ssize_t x;
        ssize_t y;
    };

    std::vector<plant_later> plant_later_;
    plant_ptr_list plant_list_;
    plant_ptr_map plant_map_;
};

struct biomass
{
};

class leaf : public plant, private boost::noncopyable
{
public:
    leaf(region *reg, rng *r, plant::type type, plant *parent) :
        plant(reg, r, type), parent_(parent)
    {

    }
    virtual ~leaf() { }

    virtual void act(std::function<void(entity *, entity::did)> on_did)
    {
        (void)on_did;
    }

    virtual const plant *get_parent() const { return parent_; }

    virtual void grow_into(plant::type type)
    {
        type_ = type;
    }

protected:
    plant *parent_;
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
    using leaf_list = std::vector<std::shared_ptr<leaf>>;

    root(region *reg, rng *r, plant_manager *pm) :
        plant(reg, r, plant::p_root), plant_manager_(pm)
    {
        growth_time_ = 3;
        growth_timer_ = 0;
        growth_count_ = 1;
    }
    virtual ~root() { }

    virtual void set_target_function(std::function<vec2()> target)
    {
        target_ = target;
    }

    virtual void act(std::function<void(entity *, entity::did)> on_did)
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
            if (leaf_list_.size())
            {
                leaf_list edges;

                for (auto &p : leaf_list_)
                {
                    if (empty_neighbors(p).size() > 0)
                    {
                        std::printf("here\n");
                        edges.push_back(p);
                    }
                }

                ssize_t count = 0;
                for (auto &p : edges)
                {
                    if (count >= growth_count_)
                        break;

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
                    auto np = std::make_shared<root>(region_, rng_, plant_manager_);
                    auto r = rng_->get_range(0, empty.size() - 1);
                    plant_manager_->add_plant_later(np, ::get_x(empty[r]), ::get_y(empty[r]));
                }
            }
        }

        for (auto &p : leaf_list_)
        {
            // Do something to target.
        }
    }

protected:
    plant_manager *plant_manager_;
    leaf_list leaf_list_;
    leaf_list growing_list_;
    std::function<vec2()> target_;
    ssize_t growth_time_;
    ssize_t growth_timer_;
    ssize_t growth_count_;

private:
    std::vector<vec2> empty_neighbors(std::shared_ptr<plant> p)
    {
        static ssize_t delta[8][2] = {
            {-1, -1},
            {-1,  0},
            {-1,  1},
            { 0, -1},
            { 0,  1},
            { 1, -1},
            { 1,  0},
            { 1,  1}
        };
        std::vector<vec2> neighbors;
        vec2 c;
        if (plant_manager_->find_plant(p, c))
        {
            for (auto &i : delta)
            {
                vec2 d = vec2(::get_x(c) + i[0], ::get_y(c) + i[1]);
                if (!plant_manager_->get_plant(::get_x(d), ::get_y(d)) &&
                    region_->walkable(::get_x(d), ::get_y(d)))
                    neighbors.push_back(d);
            }
        }
        return neighbors;
    }
};

#endif
