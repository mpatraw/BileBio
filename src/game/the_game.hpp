
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
    {5, 0.8, 0.0, 0.1},
};

static constexpr ssize_t max_levels = 1;

class the_game : private boost::noncopyable
{
public:
    the_game(std::function<void(entity *src, entity::did did)> on_did) :
        on_did_(on_did)
    {
        the_region_.reset(new region());
        plant_manager_.reset(new plant_manager());
        the_player_.reset(new player(the_region_.get(), &rng_, plant_manager_.get()));
        level_ = 0;
        reset();
    }

    void reset()
    {
        the_region_->generate(20, 20);
        auto loc = the_region_->get_random_empty_location();
        the_player_->move_to(::get_x(loc), ::get_y(loc));

        auto set = settings[level_];
        for (ssize_t i = 0; i < set.number_of_roots; ++i)
        {
            auto v = the_region_->get_random_empty_location();
            plant_manager_->add_plant(std::shared_ptr<plant>(new root(the_region_.get(), &rng_, plant_manager_.get())), ::get_x(v), ::get_y(v));
        }
    }

    const region &get_region() const { return *the_region_.get(); }
    const player &get_player() const { return *the_player_.get(); }

    void player_act(ssize_t dx, ssize_t dy, player::action act)
    {
        if (act == player::act_move)
            if (the_player_->move_by(dx, dy))
                on_did_(the_player_.get(), entity::did_move);
    }

    void rest_act()
    {
        for (auto &p : plant_manager_->get_plant_list())
            p->act(on_did_);
        plant_manager_->add_plants();
    }

    const plant *get_plant_at(ssize_t x, ssize_t y) const { return plant_manager_->get_plant(x, y).get(); }

private:
    std::unique_ptr<region> the_region_;
    std::unique_ptr<plant_manager> plant_manager_;
    std::unique_ptr<player> the_player_;
    std::function<void(entity*src, entity::did did)> on_did_;

    rng rng_;
    ssize_t level_;
};

#endif
