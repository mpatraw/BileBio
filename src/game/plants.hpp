
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
private:
    using weak_ptr = std::weak_ptr<entity>;
    using shared_ptr = std::shared_ptr<entity>;
    using on_did_func = std::function<void(weak_ptr src, entity::did did, weak_ptr targ)>;
public:
    enum type
    {
        p_growing,
        p_root,
        p_vine,
        p_flower,
        p_fruit,
    };

    plant(region *reg, rng *r, plant::type type, std::weak_ptr<plant> parent=std::weak_ptr<plant>()) :
        entity(reg, r), type_(type), parent_(parent)
    {
        grow_ = type;
    }
    virtual ~plant()
    {
    }

    virtual plant::type get_type() const { return type_; }

    virtual void act(on_did_func on_did)
    {
        (void)on_did;
    }

    virtual const weak_ptr get_parent() const { return parent_; }

    virtual void grow_into(plant::type type)
    {
        grow_ = type;
    }

    virtual void grow()
    {
        type_ = grow_;
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
    plant::type grow_;
    // Can't be shared. Would impose a circular reference.
    std::weak_ptr<plant> parent_;
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
private:
    using weak_ptr = std::weak_ptr<entity>;
    using shared_ptr = std::shared_ptr<entity>;
    using shared_const_ptr = std::shared_ptr<const entity>;
    using int_pair = std::pair<int, int>;
    using target_func = std::function<int_pair()>;
    using on_did_func = std::function<void(weak_ptr src, entity::did did, weak_ptr targ)>;
public:
    root(region *reg, rng *r, sparse_2d_map<entity> *pm, target_func target=target_func()) :
        plant(reg, r, plant::p_root), entity_manager_(pm), target_(target)
    {
        growth_time_ = 3;
        growth_timer_ = 0;
        growth_count_ = 1;
        growth_range_ = 5;
    }
    virtual ~root() { }

    virtual void set_target_function(target_func target)
    {
        target_ = target;
    }

    virtual void act(on_did_func on_did)
    {
        (void)on_did;
        if (growing_list_.size())
        {
            if (++growth_timer_ >= growth_time_)
            {
                for (auto &p : growing_list_)
                {
                    if (auto sptr = p.lock())
                        sptr->grow();
                }
                growing_list_.clear();
                growth_timer_ = 0;
            }
        }
        else
        {
            if (child_list_.size())
            {
                std::vector<std::vector<std::weak_ptr<plant>>::iterator> to_delete;
                std::vector<std::weak_ptr<plant>> edges;

                for (auto i = child_list_.begin(); i != child_list_.end(); ++i)
                {
                    if (!i->lock())
                    {
                        to_delete.push_back(i);
                    }
                    else if (empty_neighbors(*i).size() > 0)
                    {
                        edges.push_back(*i);
                    }
                }

                int count = 0;
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
                        //    int_pair targ = target_();
                        //}
                        //else
                        {
                            auto empty = empty_neighbors(p);
                            auto r = rng_->get_range(0, empty.size() - 1);
                            auto np = std::make_shared<plant>(region_, rng_, plant::p_growing, std::static_pointer_cast<plant>(std::shared_ptr<entity>(entity_manager_->get_this_ptr(this))));
                            np->grow_into(plant::p_vine);
                            growing_list_.push_back(np);
                            child_list_.push_back(np);
                            entity_manager_->add_ptr_later(np, int_pair(empty[r].first, empty[r].second));
                        }
                    }
                    // if (grow)
                    // replace
                    // else (upgrade)
                    // grow near target or random
                    // add to growing list
                }

                for (auto &i : to_delete)
                    child_list_.erase(i);
            }
            else
            {
                auto empty = empty_neighbors(entity_manager_->get_this_ptr(this));
                if (empty.size() > 0)
                {
                    auto np = std::make_shared<plant>(region_, rng_, plant::p_growing, std::static_pointer_cast<plant>(std::shared_ptr<entity>(entity_manager_->get_this_ptr(this))));
                    np->grow_into(plant::p_vine);
                    auto r = rng_->get_range(0, empty.size() - 1);
                    growing_list_.push_back(np);
                    child_list_.push_back(np);
                    entity_manager_->add_ptr_later(np, int_pair(empty[r].first, empty[r].second));
                }
                else
                {
                }
            }
        }
    }

protected:
    sparse_2d_map<entity> *entity_manager_;
    std::vector<std::weak_ptr<plant>> child_list_;
    std::vector<std::weak_ptr<plant>> growing_list_;
    target_func target_;
    int growth_time_;
    int growth_timer_;
    int growth_count_;
    int growth_range_;

private:
    std::vector<int_pair> empty_neighbors(std::weak_ptr<entity> p, int range=1)
    {
        std::vector<int_pair> neighbors;
        int_pair around;
        if (entity_manager_->exists(p.lock()))
        {
            around = entity_manager_->get_coord(p.lock());
            for (int dx = -range; dx <= range; ++dx)
            {
                for (int dy = -range; dy <= range; ++dy)
                {
                    int_pair v = {around.first + dx, around.second + dy};
                    if (!entity_manager_->exists(v) && region_->walkable(v.first, v.second))
                        neighbors.push_back(v);
                }
            }
        }
        return neighbors;
    }

    std::vector<int_pair> empty_neighbors(int_pair around, int range=1)
    {
        std::vector<int_pair> neighbors;
        for (int dx = -range; dx <= range; ++dx)
        {
            for (int dy = -range; dy <= range; ++dy)
            {
                int_pair v = {around.first + dx, around.second + dy};
                if (!entity_manager_->exists(v) && region_->walkable(v.first, v.second))
                    neighbors.push_back(v);
            }
        }
        return neighbors;
    }
};

#endif
