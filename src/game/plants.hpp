
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
// "seed"
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

    virtual plant::type grows_into() = 0;

protected:
    plant::type type_;
    plant::type grow_;
    weak_ptr target_;
    // Can't be shared. Would impose a circular reference.
    std::weak_ptr<plant> parent_;
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
            vitals_ = {1, 1, 0, 0.0}; // Seed
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

    virtual plant::type grows_into()
    {
        return "";
    }

protected:
};

class seed : public plant, private boost::noncopyable
{
private:
    using weak_ptr = std::weak_ptr<entity>;
    using shared_ptr = std::shared_ptr<entity>;
    using on_did_func = std::function<void(weak_ptr src, entity::did did, weak_ptr targ)>;
public:

    seed(region *reg, rng *r, weak_ptr target, std::weak_ptr<plant> parent, plant::type into) :
        plant(reg, r, "seed", target, parent), into_(into)
    {
        vitals_ = {1, 1, 0, 0.0};
    }
    virtual ~seed()
    {
    }

    virtual void act(on_did_func on_did)
    {
        (void)on_did;
    }

    virtual plant::type grows_into()
    {
        return into_;
    }

protected:
    plant::type into_;
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
        growing_ = false;
        growth_time_ = 3;
        growth_timer_ = 0;
        growth_count_ = 1;
        growth_range_ = 5;
    }
    virtual ~root() { }

    virtual void act(on_did_func on_did)
    {
        (void)on_did;

        std::vector<std::vector<std::weak_ptr<plant>>::iterator> to_delete;
        for (auto i = child_list_.begin(); i != child_list_.end(); ++i)
            if (!i->lock())
                to_delete.push_back(i);
        for (auto &i : to_delete)
            child_list_.erase(i);

        if (growing_)
        {
            if (++growth_timer_ >= growth_time_)
            {
                for (auto &child : child_list_)
                {
                    if (auto sptr = child.lock())
                    {
                        if (sptr->get_type() == "seed")
                        {
                            hatch_seed(std::static_pointer_cast<seed>(sptr));
                        }
                    }
                }
                growing_ = false;
                growth_timer_ = 0;
            }
        }
        else
        {
            // Find edges.
            std::vector<std::weak_ptr<plant>> edges;
            for (auto &child : child_list_)
                if (empty_neighbors(child).size() > 0)
                    edges.push_back(child);
            if (empty_neighbors(shared_from_this()).size() > 0)
                edges.push_back(shared_from_this());

            rng_->shuffle(edges.begin(), edges.end());

            int count = 0;
            for (auto &edge : edges)
            {
                if (auto edge_sptr = edge.lock())
                {
                    if (++count > growth_count_)
                        break;

                    // 10% to transform.
                    if (rng_->get_uniform() < 0.1)
                    {
                    }
                    else
                    {
                        if (auto targ_sptr = target_.lock())
                        {
                            auto t = entities_->get_coord(edge_sptr);
                            auto c = entities_->get_coord(targ_sptr);
                            auto a = (c.first - t.first);
                            auto b = (c.second - t.second);
                            if (std::sqrt(a * a + b * b) < 6)
                            {
                                auto empty = empty_neighbors(targ_sptr);
                                if (empty.size() > 0)
                                {
                                    auto r = rng_->get_range(0, empty.size() - 1);
                                    grow_seed("vine", int_pair(empty[r].first, empty[r].second));
                                }
                            }
                            else
                            {
                                auto empty = empty_neighbors(edge_sptr);
                                auto r = rng_->get_range(0, empty.size() - 1);
                                grow_seed("vine", int_pair(empty[r].first, empty[r].second));
                            }
                        }
                        else
                        {
                            auto empty = empty_neighbors(edge_sptr);
                            auto r = rng_->get_range(0, empty.size() - 1);
                            grow_seed("vine", int_pair(empty[r].first, empty[r].second));
                        }
                    }
                }
            }
            growing_ = true;
        }
    }

    virtual plant::type grows_into()
    {
        return "";
    }

protected:
    sparse_2d_map<entity> *entities_;
    std::vector<std::weak_ptr<plant>> child_list_;
    bool growing_;
    int growth_time_;
    int growth_timer_;
    int growth_count_;
    int growth_range_;

private:
    std::vector<int_pair> empty_neighbors(std::weak_ptr<entity> pl, int range=1)
    {
        std::vector<int_pair> neighbors;
        if (auto sptr = pl.lock())
        {
            auto around = entities_->get_coord(sptr);
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

    template <typename P, typename... Args>
    void grow_something(int_pair coord, Args... args)
    {
        if (entities_->exists(coord))
            entities_->del_coord(coord);

        auto np = std::make_shared<P>(args...);
        child_list_.push_back(np);
        entities_->add_ptr_later(np, coord);
    }

    void hatch_seed(std::shared_ptr<seed> seed)
    {
        auto coord = entities_->get_coord(seed);
        auto into = seed->grows_into();
        if (into == "vine")
            grow_something<vine>(coord, region_, rng_, target_, shared_from_this());
    }

    void grow_seed(plant::type into, int_pair coord)
    {
        grow_something<seed>(coord, region_, rng_, target_, shared_from_this(), into);
    }
};

#endif
