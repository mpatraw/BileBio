
#ifndef PLANTS_HPP
#define PLANTS_HPP

#include <functional>
#include <map>
#include <memory>
#include <tuple>
#include <vector>

#include <entity.hpp>
#include <random.hpp>

// Current plant types:
// "pod"
// "root"
// "vine"
// "flower"
// "fruit"

struct biomass
{
};

class plant : public entity, public std::enable_shared_from_this<plant>, private boost::noncopyable
{
private:
    using weak_ptr = std::weak_ptr<entity>;
    using shared_ptr = std::shared_ptr<entity>;
    using on_did_func = std::function<void(weak_ptr src, entity::did did, weak_ptr targ)>;
public:

    using type = std::string;

    plant(region *reg, rng *r, plant::type type, weak_ptr target, std::weak_ptr<plant> parent) :
        entity(reg, r), type_(type), target_(target), parent_(parent)
    {
        grow_ = type;
    }
    virtual ~plant()
    {
        std::printf("%s destroyd\n", type_.c_str());
    }

    virtual plant::type get_type() const { return type_; }
    virtual weak_ptr get_target() const { return target_; }
    virtual const weak_ptr get_parent() const { return parent_; }

    virtual void act(on_did_func on_did)
    {
        (void)on_did;
    }

protected:
    plant::type type_;
    plant::type grow_;
    weak_ptr target_;
    // Can't be shared. Would impose a circular reference.
    std::weak_ptr<plant> parent_;
};

class pod : public plant, private boost::noncopyable
{
private:
    using weak_ptr = std::weak_ptr<entity>;
    using shared_ptr = std::shared_ptr<entity>;
    using on_did_func = std::function<void(weak_ptr src, entity::did did, weak_ptr targ)>;
public:

    pod(region *reg, rng *r, weak_ptr target, std::weak_ptr<plant> parent) :
        plant(reg, r, "pod", target, parent)
    {
        vitals_ = {1, 1, 0, 0.0};
    }
    virtual ~pod()
    {
    }

    virtual void act(on_did_func on_did)
    {
        (void)on_did;
    }

protected:
};

class vine : public plant, private boost::noncopyable
{
private:
    using weak_ptr = std::weak_ptr<entity>;
    using shared_ptr = std::shared_ptr<entity>;
    using on_did_func = std::function<void(weak_ptr src, entity::did did, weak_ptr targ)>;
public:

    vine(region *reg, rng *r, weak_ptr target, std::weak_ptr<plant> parent) :
        plant(reg, r, "vine", target, parent)
    {
        vitals_ = {3, 3, 0, 0.0};
            vitals_ = {1, 1, 0, 0.0}; // Growing
            vitals_ = {3, 3, 0, 0.0}; // Flower
            vitals_ = {1, 1, 0, 0.0}; // Fruit
    }
    virtual ~vine()
    {
    }

    virtual void act(on_did_func on_did)
    {
        (void)on_did;
    }

protected:
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
    root(region *reg, rng *r, sparse_2d_map<entity> *pm, weak_ptr target) :
        plant(reg, r, "root", target, std::weak_ptr<plant>()), entities_(pm)
    {
        growth_time_ = 3;
        growth_timer_ = 0;
        growth_count_ = 1;
        growth_range_ = 5;
    }
    virtual ~root() { }

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
                    {
                        auto c = entities_->get_coord(sptr);
                        replace_child<vine>(c);
                    }
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

                // Delete unreferenced plants.
                for (auto &i : to_delete)
                    child_list_.erase(i);

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
                        if (auto sptr = target_.lock())
                        {
                            auto t = entities_->get_coord(p.lock());
                            auto c = entities_->get_coord(sptr);
                            auto a = (c.first - t.first);
                            auto b = (c.second - t.second);
                            if (std::sqrt(a * a + b * b) < 6)
                            {
                                auto empty = empty_neighbors(sptr);
                                if (empty.size() > 0)
                                {
                                    auto r = rng_->get_range(0, empty.size() - 1);
                                    grow_child<pod>(int_pair(empty[r].first, empty[r].second));
                                }
                            }
                        }
                        else
                        {
                            auto empty = empty_neighbors(p);
                            auto r = rng_->get_range(0, empty.size() - 1);
                            grow_child<pod>(int_pair(empty[r].first, empty[r].second));
                        }
                    }
                }
            }
            else
            {
                auto empty = empty_neighbors(entities_->get_this_ptr(this));
                if (empty.size() > 0)
                {
                    auto r = rng_->get_range(0, empty.size() - 1);
                    grow_child<pod>(int_pair(empty[r].first, empty[r].second));
                }
                else
                {
                }
            }
        }
    }

protected:
    sparse_2d_map<entity> *entities_;
    std::vector<std::weak_ptr<plant>> child_list_;
    std::vector<std::weak_ptr<plant>> growing_list_;
    int growth_time_;
    int growth_timer_;
    int growth_count_;
    int growth_range_;

private:
    std::vector<int_pair> empty_neighbors(std::weak_ptr<entity> p, int range=1)
    {
        std::vector<int_pair> neighbors;
        int_pair around;
        if (entities_->exists(p.lock()))
        {
            around = entities_->get_coord(p.lock());
            for (int dx = -range; dx <= range; ++dx)
            {
                for (int dy = -range; dy <= range; ++dy)
                {
                    int_pair v = {around.first + dx, around.second + dy};
                    if (!entities_->exists(v) && region_->walkable(v.first, v.second))
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
                if (!entities_->exists(v) && region_->walkable(v.first, v.second))
                    neighbors.push_back(v);
            }
        }
        return neighbors;
    }

    template <typename P>
    void grow_child(int_pair coord)
    {
        auto np = std::make_shared<P>(region_, rng_, target_, shared_from_this());
        growing_list_.push_back(np);
        child_list_.push_back(np);
        entities_->add_ptr_later(np, coord);
    }

    template <typename P>
    void replace_child(int_pair coord)
    {
        entities_->del_coord(coord);
        auto np = std::make_shared<P>(region_, rng_, target_, shared_from_this());
        child_list_.push_back(np);
        entities_->add_ptr_later(np, coord);
    }
};

#endif
