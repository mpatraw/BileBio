
#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <region.hpp>

struct vitals
{
    ssize_t hearts;
    ssize_t max_hearts;
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

    entity(region *reg) :
        region_(reg)
    {
        vitals_ = {3, 3};
    }
    virtual ~entity() { }

    virtual const vitals &get_vitals() const { return vitals_; }
    virtual vitals &get_vitals() { return vitals_; }

    virtual bool is_dead() const { return vitals_.hearts <= 0; }
    virtual bool is_alive() const { return !is_dead(); }

protected:
    vitals vitals_;

    region *region_;
};

#endif
