
#ifndef PLANTS_HPP
#define PLANTS_HPP

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
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
protected:
    using weak_ptr = std::weak_ptr<entity>;
    using shared_ptr = std::shared_ptr<entity>;
    using shared_const_ptr = std::shared_ptr<const entity>;
    using int_pair = std::pair<int, int>;
    using target_func = std::function<int_pair()>;
    using on_did_func = std::function<void(weak_ptr src, entity::did did, weak_ptr targ)>;
public:
    using type = std::string;

    plant(region *reg, rng *r, plant::type type, sparse_2d_map<entity> *pm, weak_ptr target, std::weak_ptr<plant> parent) :
        entity(reg, r), type_(type), entities_(pm), target_(target), parent_(parent)
    {
        grow_ = type;
    }
    virtual ~plant()
    {
        std::printf("%s destroyed\n", type_.c_str());
    }

    virtual plant::type get_type() const { return type_; }
    virtual weak_ptr get_target() const { return target_; }
    virtual const weak_ptr get_parent() const { return parent_; }
    virtual bool can_spawn_more() const { return false; }
    virtual void spawned_something() { }

    virtual void act(on_did_func on_did)
    {
        if (is_dead())
        {
            on_did(shared_from_this(), entity::did_die, weak_ptr());
            entities_->del_ptr(shared_from_this());
            return;
        }
    }
    virtual void spawn(on_did_func on_did) = 0;

protected:
    plant::type type_;
    plant::type grow_;
    sparse_2d_map<entity> *entities_;
    weak_ptr target_;
    // Can't be shared. Would impose a circular reference.
    std::weak_ptr<plant> parent_;

    // Helper functions.

    std::vector<int_pair> empty_neighbors(weak_ptr pl, int range=1)
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

        auto np = std::make_shared<P>(region_, rng_, entities_, target_, args...);
        entities_->add_ptr_later(np, coord);
    }

    boost::optional<double> distance_between(weak_ptr a, weak_ptr b)
    {
        auto asptr = a.lock();
        auto bsptr = b.lock();

        if (asptr && bsptr && entities_->exists(asptr) && entities_->exists(bsptr))
        {
            auto acoord = entities_->get_coord(asptr);
            auto bcoord = entities_->get_coord(bsptr);
            auto dx = (acoord.first - bcoord.first);
            auto dy = (acoord.second - bcoord.second);
            return boost::optional<double>(std::sqrt(dx * dx + dy * dy));
        }

        return boost::optional<double>();
    }
};

class vine;
class seed : public plant, private boost::noncopyable
{
private:
    using weak_ptr = std::weak_ptr<entity>;
    using shared_ptr = std::shared_ptr<entity>;
    using on_did_func = std::function<void(weak_ptr src, entity::did did, weak_ptr targ)>;
public:

    seed(region *reg, rng *r, sparse_2d_map<entity> *pm, weak_ptr target, std::weak_ptr<plant> parent, plant::type into) :
        plant(reg, r, "seed", pm, target, parent), into_(into)
    {
        vitals_ = {1, 1, 0, 0.0};
        timer_ = 3;
    }
    virtual ~seed()
    {
    }

    virtual void act(on_did_func on_did)
    {
        if (is_dead())
        {
            on_did(shared_from_this(), entity::did_die, weak_ptr());
            entities_->del_ptr(shared_from_this());
            return;
        }

        if (--timer_ <= 0)
        {
            if (into_ == "vine")
                grow_something<vine>(entities_->get_coord(shared_from_this()), parent_);
        }
    }

    virtual void spawn(on_did_func on_did)
    {
        // Seeds don't spawn
        (void)on_did;
    }

protected:
    plant::type into_;
    int timer_;
};

class vine : public plant, private boost::noncopyable
{
private:
    using weak_ptr = std::weak_ptr<entity>;
    using shared_ptr = std::shared_ptr<entity>;
    using on_did_func = std::function<void(weak_ptr src, entity::did did, weak_ptr targ)>;
public:

    vine(region *reg, rng *r, sparse_2d_map<entity> *pm, weak_ptr target, std::weak_ptr<plant> parent) :
        plant(reg, r, "vine", pm, target, parent)
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
        if (is_dead())
        {
            on_did(shared_from_this(), entity::did_die, weak_ptr());
            entities_->del_ptr(shared_from_this());
            return;
        }
    }

    virtual void spawn(on_did_func on_did)
    {
        (void)on_did;

        if (auto parentsptr = parent_.lock())
        {
            if (!parentsptr->can_spawn_more())
                return;

            auto targ_sptr = target_.lock();
            auto distance = distance_between(shared_from_this(), targ_sptr);
            weak_ptr final_target = shared_from_this();
            if (targ_sptr && distance && *distance < 6.0)
                final_target = targ_sptr;

            auto empty = empty_neighbors(final_target);
            if (empty.size() > 0)
            {
                std::printf("vine spawning\n");
                auto r = rng_->get_range(0, empty.size() - 1);
                grow_something<seed>(int_pair(empty[r].first, empty[r].second), shared_from_this(), "vine");
                parentsptr->spawned_something();
            }
        }
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
public:
    root(region *reg, rng *r, sparse_2d_map<entity> *pm, weak_ptr target) :
        plant(reg, r, "root", pm, target, std::weak_ptr<plant>())
    {
        growth_cooldown_ = 3;
        growth_count_ = 1;
    }
    virtual ~root() { }

    virtual bool can_spawn_more() const
    {
        // Growth count has to be above 0.
        return growth_count_ > 0;
    }

    virtual void spawned_something()
    {
        // When limit reached, set the time for next spawn.
        if (--growth_count_ <= 0)
            growth_cooldown_ = 3;
    }

    virtual void act(on_did_func on_did)
    {
        if (is_dead())
        {
            on_did(shared_from_this(), entity::did_die, weak_ptr());
            entities_->del_ptr(shared_from_this());
            return;
        }

        if (--growth_cooldown_ <= 0)
            growth_count_ = 1;
    }

    virtual void spawn(on_did_func on_did)
    {
        (void)on_did;
        if (!can_spawn_more())
            return;

        auto targ_sptr = target_.lock();
        auto distance = distance_between(shared_from_this(), targ_sptr);
        weak_ptr final_target = shared_from_this();
        if (targ_sptr && distance && *distance < 6.0)
            final_target = targ_sptr;

        auto empty = empty_neighbors(final_target);
        if (empty.size() > 0)
        {
            std::printf("root spawning\n");
            auto r = rng_->get_range(0, empty.size() - 1);
            grow_something<seed>(int_pair(empty[r].first, empty[r].second), shared_from_this(), "vine");
            spawned_something();
        }
    }

protected:
    int growth_cooldown_;
    int growth_count_;
};

#endif
