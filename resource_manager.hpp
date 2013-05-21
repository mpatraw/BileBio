
#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>

#include <boost/any.hpp>
#include <boost/noncopyable.hpp>

class resource_manager : private boost::noncopyable
{
public:
    resource_manager() { }
    ~resource_manager() { };

    template <typename R>
    void manage(std::string key, const R &resource)
    {
        auto &resource_map = map_[std::type_index(typeid(R))];
        resource_map[key] = std::move(resource);
    }

    template <typename R>
    R &acquire(std::string key)
    {
        if (map_.find(std::type_index(typeid(R))) == map_.end())
            throw std::out_of_range("aquire");
        if (map_[std::type_index(typeid(R))].find(key) == map_[std::type_index(typeid(R))].end())
            throw std::out_of_range("aquire");
        return boost::any_cast<R &>(map_[std::type_index(typeid(R))][key]);
    }

    template <typename R>
    const R &acquire(std::string key) const
    {
        if (map_.find(std::type_index(typeid(R))) == map_.end())
            throw std::out_of_range("aquire");
        if (map_.find(std::type_index(typeid(R)))->second.find(key) == map_.find(std::type_index(typeid(R)))->second.end())
            throw std::out_of_range("aquire");
        return boost::any_cast<const R &>(map_.find(std::type_index(typeid(R)))->second.find(key)->second);
    }
private:

    std::unordered_map<std::type_index, std::unordered_map<std::string, boost::any>> map_;
};

#endif
