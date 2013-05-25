
#ifndef PLANTS_HPP
#define PLANTS_HPP

#include <functional>
#include <map>
#include <memory>
#include <tuple>
#include <vector>

#include <entity.hpp>

class plant : public entity, private boost::noncopyable
{
public:
    enum type
    {
        p_root,
        p_vine,
        p_flower,
        p_fruit,
    };

    plant(region *reg, plant::type type) :
        entity(reg), type_(type)
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
    using coord = std::tuple<ssize_t, ssize_t>;
    using plant_ptr = std::shared_ptr<plant>;
    using plant_ptr_list = std::vector<plant_ptr>;
    using plant_ptr_map = std::map<coord, plant_ptr>;

    plant_manager() { }

    void add_plant(plant_ptr p, ssize_t x, ssize_t y)
    {
        if (get_plant(x, y))
        {
            remove_plant(x, y);
        }
        plant_list_.push_back(p);
        plant_map_[coord(x, y)] = p;
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
                if (i->get() == p)
                {
                    plant_list_.erase(i);
                    break;
                }
            }
            plant_map_.erase(plant_map_.find(coord(x, y)));
        }
    }

    void move_plant(ssize_t sx, ssize_t sy, ssize_t ex, ssize_t ey)
    {
        auto p = get_plant_ptr(sx, sy);
        remove_plant(sx, sy);
        add_plant(p, ex, ey);
    }

    plant *get_plant(ssize_t x, ssize_t y)
    {
        return get_plant_ptr(x, y).get();
    }

    const plant *get_plant(ssize_t x, ssize_t y) const
    {
        return get_plant_ptr(x, y).get();
    }

    plant_ptr get_plant_ptr(ssize_t x, ssize_t y)
    {
        auto c = coord(x, y);
        if (plant_map_.find(c) == plant_map_.end())
            return nullptr;
        else
            return plant_map_.at(c);
    }

    const plant_ptr get_plant_ptr(ssize_t x, ssize_t y) const
    {
        auto c = coord(x, y);
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
    leaf(region *reg, plant::type type, plant *parent) :
        plant(reg, type), parent_(parent)
    {

    }
    virtual ~leaf() { }

    virtual void act(std::function<void(entity *, entity::did)> on_did)
    {
        (void)on_did;
    }

    virtual const plant *get_parent() const { return parent_; }

protected:
    plant *parent_;
};

class root : public plant, private boost::noncopyable
{
public:
    using leaf_list = std::vector<std::shared_ptr<leaf>>;

    root(region *reg, plant_manager *pm) :
        plant(reg, plant::p_root), plant_manager_(pm)
    {
        growth_time_ = 3;
        growth_timer_ = 0;
        growth_count_ = 1;
    }
    virtual ~root() { }

    virtual void act(std::function<void(entity *, entity::did)> on_did)
    {
        (void)on_did;
    }

protected:
    plant_manager *plant_manager_;
    leaf_list leaf_list_;
    leaf_list growing_list_;
    ssize_t growth_time_;
    ssize_t growth_timer_;
    ssize_t growth_count_;

private:

};

#endif
