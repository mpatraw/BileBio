
#ifndef THE_GAME_HPP
#define THE_GAME_HPP

#include <boost/noncopyable.hpp>
#include <map>
#include <vector>

#include <random.hpp>
#include <region.hpp>
#include <entity.hpp>
#include <plants.hpp>
#include <player.hpp>

struct level_settings
{
    ssize_t number_of_roots;
    double chance_to_be_good;
    double chance_to_be_neutral;
    double chance_to_be_evil;
};

static constexpr level_settings settings[] = {
    {1, 0.8, 0.0, 0.1},
};

static constexpr ssize_t max_levels = 1;

class the_game : private boost::noncopyable
{
public:
    the_game(std::function<void(std::weak_ptr<entity> src, entity::did did, std::weak_ptr<entity> targ)> on_did) :
        on_did_(on_did)
    {
        the_region_.reset(new region());
        entity_manager_.reset(new sparse_2d_map<entity>());
        the_player_ = std::make_shared<player>(the_region_.get(), &rng_, entity_manager_.get());
        level_ = 0;
        reset();
    }

    void reset()
    {
        the_region_->generate(20, 20);
        auto loc = the_region_->get_random_empty_coord();
        entity_manager_->clear();
        entity_manager_->add_ptr(the_player_, {0, 0});
        the_player_->perform_to({loc.first, loc.second}, player::act_move, nullptr);

        auto set = settings[level_];
        for (ssize_t i = 0; i < set.number_of_roots; ++i)
        {
            auto v = the_region_->get_random_empty_coord();
            if (v.first != loc.first && v.second != loc.second)
            {
                auto r = new root(the_region_.get(), &rng_, entity_manager_.get(), the_player_);
                entity_manager_->add_ptr(std::shared_ptr<plant>(r), {v.first, v.second});
            }
        }
    }

    const region &get_region() const { return *the_region_.get(); }
    const player &get_player() const { return *the_player_.get(); }

    void player_act(ssize_t dx, ssize_t dy, player::action act)
    {
        the_player_->perform({dx, dy}, act, on_did_);
    }

    void rest_act()
    {
        for (auto &p : *entity_manager_)
        {
            if (auto cast = std::dynamic_pointer_cast<plant>(p.second))
                cast->act(on_did_);
        }
        entity_manager_->del_ptrs();
        for (auto &p : *entity_manager_)
        {
            if (auto cast = std::dynamic_pointer_cast<plant>(p.second))
                cast->spawn(on_did_);
        }
        entity_manager_->add_ptrs();
    }

    std::pair<int, int> player_coord()
    {
        return entity_manager_->get_coord(the_player_);
    }

    std::weak_ptr<entity> get_entity_at(ssize_t x, ssize_t y) const { return entity_manager_->get_ptr({x, y}); }
    bool get_entity_coord(std::weak_ptr<entity> pl, std::pair<int, int> &v) const
    {
        if (entity_manager_->exists(pl.lock()))
        {
            v = entity_manager_->get_coord(pl.lock());
            return true;
        }
        return false;
    }

private:
    std::unique_ptr<region> the_region_;
    std::unique_ptr<sparse_2d_map<entity>> entity_manager_;
    std::shared_ptr<player> the_player_;
    std::function<void(std::weak_ptr<entity> src, entity::did did, std::weak_ptr<entity> targ)> on_did_;

    rng rng_;
    ssize_t level_;
};

#endif
