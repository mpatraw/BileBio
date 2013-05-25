
#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <region.hpp>

struct coord
{
    ssize_t x;
    ssize_t y;
};

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
    }
    virtual ~entity() { }

    virtual const coord &get_coord() const { return coord_; }
    virtual coord &get_coord() { return coord_; }

    virtual const vitals &get_vitals() const { return vitals_; }
    virtual vitals &get_vitals() { return vitals_; }

    virtual bool is_dead() const { return vitals_.hearts <= 0; }
    virtual bool is_alive() const { return !is_dead(); }
protected:
    coord coord_;
    vitals vitals_;

    region *region_;
};

#endif
