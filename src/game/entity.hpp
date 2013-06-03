
#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <algorithm>
#include <map>
#include <memory>
#include <stdexcept>
#include <utility>

#include <random.hpp>
#include <region.hpp>

struct vitals
{
    int hearts;
    int max_hearts;
    int damage;
    double to_hit;
};

class entity : private boost::noncopyable
{
public:
    enum did
    {
        did_nothing,
        did_spawn,
        did_die,

        did_move,
        did_grow,
        did_attack,
        did_miss
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

    virtual void take_damage(int dam)
    {
        vitals_.hearts = std::max(vitals_.hearts - dam, static_cast<int>(0));
    }

    virtual void heal_damage(int count)
    {
        vitals_.hearts = std::min(vitals_.hearts + count, vitals_.max_hearts);
    }

    virtual void gain_hearts(int count)
    {
        vitals_.max_hearts += count;
        vitals_.hearts += count;
    }

protected:
    vitals vitals_;

    region *region_;
    rng *rng_;
};

template <typename T>
class sparse_2d_map
{
private:
    using weak_ptr = std::weak_ptr<T>;
    using shared_ptr = std::shared_ptr<T>;
    using shared_const_ptr = std::shared_ptr<const T>;
    using int_pair = std::pair<int, int>;
public:
    sparse_2d_map() { }

    bool exists(int_pair coord) const { return coord_map_.find(coord) != coord_map_.end(); }
    bool exists(shared_const_ptr ptr) const { return ptr_map_.find(ptr) != ptr_map_.end(); }

    // Should really only ever be used for a "this" argument.
    weak_ptr get_this_ptr(const T *ent) const
    {
        for (auto &kv : coord_map_)
            if (kv.second.get() == ent)
                return kv.second;
        return weak_ptr();
    }
    weak_ptr get_ptr(int_pair coord) const
    {
        if (!exists(coord))
            return weak_ptr();
        return coord_map_.at(coord);
    }
    int_pair get_coord(shared_const_ptr ptr) const
    {
        if (!exists(ptr))
            throw std::out_of_range("get_coord");
        return ptr_map_.at(ptr);
    }

    void add_ptr(shared_ptr ptr, int_pair coord)
    {
        coord_map_[coord] = ptr;
        ptr_map_[ptr] = coord;
    }

    void add_ptr_later(shared_ptr ptr, int_pair coord)
    {
        add_later_.push_back({ptr, coord});
    }
    void add_ptrs()
    {
        for (auto &el : add_later_)
            add_ptr(el.ptr, el.coord);
        add_later_.clear();
    }

    void del_ptr(shared_const_ptr ptr)
    {
        auto c = get_coord(ptr);
        if (coord_map_[c] == ptr)
            coord_map_.erase(c);
        ptr_map_.erase(ptr);
    }
    void del_ptr_later(shared_ptr ptr)
    {
        del_later_.push_back(ptr);
    }
    void del_ptrs()
    {
        for (auto &p : del_later_)
            del_ptr(p);
        del_later_.clear();
    }
    void del_coord_later(int_pair coord)
    {
        del_ptr_later(get_ptr(coord).lock());
    }

    void move_ptr_by(shared_ptr ptr, int_pair delta)
    {
        auto coord = get_coord(ptr);
        auto to = int_pair(coord.first + delta.first, coord.second + delta.second);
        move_ptr_to(ptr, to);
    }
    void move_ptr_to(shared_ptr ptr, int_pair coord)
    {
        del_ptr(ptr);
        add_ptr(ptr, coord);
    }

    void clear()
    {
        coord_map_.clear();
        ptr_map_.clear();
        add_later_.clear();
    }

    typename std::map<int_pair, shared_ptr>::iterator begin()
    {
        return coord_map_.begin();
    }

    typename std::map<int_pair, shared_ptr>::iterator end()
    {
        return coord_map_.end();
    }

    typename std::map<int_pair, shared_ptr>::const_iterator begin() const
    {
        return coord_map_.cbegin();
    }

    typename std::map<int_pair, shared_ptr>::const_iterator end() const
    {
        return coord_map_.cend();
    }
private:
    struct later
    {
        shared_ptr ptr;
        int_pair coord;
    };

    std::vector<later> add_later_;
    std::vector<shared_ptr> del_later_;
    std::map<int_pair, shared_ptr> coord_map_;
    std::map<shared_const_ptr, int_pair> ptr_map_;
};

#endif
