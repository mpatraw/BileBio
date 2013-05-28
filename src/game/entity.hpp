
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

    bool exists(int_pair vec) const { return vec_map_.find(vec) != vec_map_.end(); }
    bool exists(shared_const_ptr ptr) const { return ptr_map_.find(ptr) != ptr_map_.end(); }

    // Should really only ever be used for a "this" argument.
    weak_ptr get_this_ptr(const T *ent) const
    {
        for (auto &kv : vec_map_)
            if (kv.second.get() == ent)
                return kv.second;
        return weak_ptr();
    }
    weak_ptr get_ptr(int_pair vec) const
    {
        if (!exists(vec))
            return weak_ptr();
        return vec_map_.at(vec);
    }
    int_pair get_vec(shared_const_ptr ptr) const
    {
        if (!exists(ptr))
            throw std::out_of_range("get_vec");
        return ptr_map_.at(ptr);
    }

    void add_ptr(shared_ptr ptr, int_pair vec)
    {
        vec_map_[vec] = ptr;
        ptr_map_[ptr] = vec;
    }

    void add_ptr_later(shared_ptr ptr, int_pair vec)
    {
        later_.push_back({ptr, vec});
    }
    void add_ptrs()
    {
        for (auto &el : later_)
            add_ptr(el.ptr, el.vec);
        later_.clear();
    }

    void del_ptr(shared_const_ptr ptr)
    {
        vec_map_.erase(get_vec(ptr));
        ptr_map_.erase(ptr);
    }
    void del_vec(int_pair vec)
    {
        ptr_map_.erase(get_ptr(vec).lock());
        vec_map_.erase(vec);
    }

    void move_ptr_by(shared_ptr ptr, int_pair delta)
    {
        auto vec = get_vec(ptr);
        auto to = int_pair(vec.first + delta.first, vec.second + delta.second);
        move_ptr_to(ptr, to);
    }
    void move_ptr_to(shared_ptr ptr, int_pair vec)
    {
        del_ptr(ptr);
        add_ptr(ptr, vec);
    }

    void clear()
    {
        vec_map_.clear();
        ptr_map_.clear();
        later_.clear();
    }

    typename std::map<int_pair, shared_ptr>::iterator begin()
    {
        return vec_map_.begin();
    }

    typename std::map<int_pair, shared_ptr>::iterator end()
    {
        return vec_map_.end();
    }

    typename std::map<int_pair, shared_ptr>::const_iterator begin() const
    {
        return vec_map_.cbegin();
    }

    typename std::map<int_pair, shared_ptr>::const_iterator end() const
    {
        return vec_map_.cend();
    }
private:
    struct later
    {
        shared_ptr ptr;
        int_pair vec;
    };

    std::vector<later> later_;
    std::map<int_pair, shared_ptr> vec_map_;
    std::map<shared_const_ptr, int_pair> ptr_map_;
};

#endif
