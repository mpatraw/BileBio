
#ifndef PLANTS_HPP
#define PLANTS_HPP

#include <functional>
#include <map>
#include <memory>
#include <tuple>
#include <vector>

#include <entity.hpp>

struct biomass
{
};

class plant : public entity, private boost::noncopyable
{
public:
    plant(region *reg) :
        entity(reg)
    {
    }
    virtual ~plant() { }

    virtual void act(std::function<void(entity *, entity::did)> on_did) = 0;
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

    plant *get_plant(ssize_t x, ssize_t y)
    {
        auto c = coord(x, y);
        if (plant_map_.find(c) == plant_map_.end())
            return nullptr;
        else
            return plant_map_.at(c).get();
    }

    const plant *get_plant(ssize_t x, ssize_t y) const
    {
        auto c = coord(x, y);
        if (plant_map_.find(c) == plant_map_.end())
            return nullptr;
        else
            return plant_map_.at(c).get();
    }

    plant_ptr_list &get_plant_list() { return plant_list_; }
    const plant_ptr_list &get_plant_list() const { return plant_list_; }

private:
    plant_ptr_list plant_list_;
    plant_ptr_map plant_map_;
};

#endif
