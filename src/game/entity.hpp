
#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <algorithm>

#include <region.hpp>

struct vitals
{
    ssize_t hearts;
    ssize_t max_hearts;
    ssize_t damage;
    double to_hit;
};

class entity : private boost::noncopyable
{
public:
    enum did
    {
        did_nothing,
        did_move,
        did_grow,
        did_attack,
    };

    entity(region *reg, rng *r) :
        region_(reg), rng_(r)
    {
        vitals_ = {3, 3, 1, 0.5};
    }
    virtual ~entity() { }

    virtual const vitals &get_vitals() const { return vitals_; }
    virtual vitals &get_vitals() { return vitals_; }

    virtual bool is_dead() const { return vitals_.hearts <= 0; }
    virtual bool is_alive() const { return !is_dead(); }

    virtual void take_damage(ssize_t dam)
    {
        vitals_.hearts = std::max(vitals_.hearts - dam, static_cast<ssize_t>(0));
    }

    virtual void heal_damage(ssize_t count)
    {
        vitals_.hearts = std::min(vitals_.hearts + count, vitals_.max_hearts);
    }

    virtual void gain_hearts(ssize_t count)
    {
        vitals_.max_hearts += count;
        vitals_.hearts += count;
    }

protected:
    vitals vitals_;

    region *region_;
    rng *rng_;
};

#endif
