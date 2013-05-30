
#ifndef GAME_HPP
#define GAME_HPP

#include <boost/noncopyable.hpp>
#include <unordered_map>
#include <vector>

#include <SFML/Graphics.hpp>

#include <screen_manager.hpp>
#include <game/the_game.hpp>
#include <utils/animation_manager.hpp>
#include <utils/resource_manager.hpp>
#include <ui/simple_ui.hpp>

static constexpr double tiles_per_screen = 5.0;
static constexpr double tile_size = 1.0 / tiles_per_screen;
static constexpr double turn_length_s = 0.5;

class player_controller
{
private:
    using weak_ptr = std::weak_ptr<entity>;
    using weak_const_ptr = std::weak_ptr<const entity>;
    using shared_ptr = std::shared_ptr<entity>;
    using shared_const_ptr = std::shared_ptr<const entity>;
public:
    player_controller(the_game *tg, resource_manager *sm) :
        the_game_(tg), sprite_manager_(sm)
    {
        player_anim_.set_state("walking_s");
        player_anim_.get_animation().set_duration(turn_length_s);
        player_anim_.get_animation().set_loops(true);
        player_anim_.get_animation().add_frame("player_sw1");
        player_anim_.get_animation().add_frame("player_sw2");
        player_anim_.set_state("attacking_s");
        player_anim_.get_animation().set_duration(turn_length_s);
        player_anim_.get_animation().set_loops(false);
        player_anim_.get_animation().add_frame("player_sa");
        player_anim_.get_animation().add_frame("player");
        player_anim_.set_state("standing");
        player_anim_.get_animation().set_duration(turn_length_s);
        player_anim_.get_animation().set_loops(true);
        player_anim_.get_animation().add_frame("player");
        player_anim_.get_animation().start();
        auto loc = the_game_->player_coord();
        player_coord_ = {loc.first * tile_size, loc.second * tile_size};
        player_destination_ = {loc.first * tile_size, loc.second * tile_size};
        player_moving_ = false;
    }

    virtual ~player_controller() { }

    const sf::Vector2f &get_coord() const { return player_coord_; }
    const state_animator &get_animator() const { return player_anim_; }

    virtual void update(double dt)
    {
        // Player turn over. Take events.
        if (!player_moving_)
        {
            player_anim_.set_state("standing");
            player_anim_.get_animation().start();
            ssize_t dx = 0, dy = 0;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            {
                dx = 0;
                dy = -1;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            {
                dx = -1;
                dy = 0;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            {
                dx = 0;
                dy = 1;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            {
                dx = 1;
                dy = 0;
            }
            if (dx != 0 || dy != 0)
                the_game_->player_act(dx, dy, player::act_move);
        }

        // Player turn happening, update shit.
        if (player_moving_)
        {
            player_timer_ += dt;
            if (player_timer_ >= turn_length_s / 2.0)
            {
                attacking_ = weak_ptr();
                missing_ = weak_ptr();
            }

            if (player_timer_ >= turn_length_s)
            {
                player_coord_ = player_destination_;
                player_moving_ = false;
                the_game_->rest_act();
            }
            else
            {
                player_coord_ = sf::Vector2f(
                    player_coord_.x + player_delta_.x * (dt / turn_length_s),
                    player_coord_.y + player_delta_.y * (dt / turn_length_s));
            }
        }

        player_anim_.get_animation().update(dt);
    }

    virtual void player_did(entity::did did, weak_ptr targ)
    {
        auto loc = the_game_->player_coord();
        if (did == entity::did_move)
        {
            player_destination_ = {loc.first * tile_size, loc.second * tile_size};
            player_delta_ = player_destination_ - player_coord_;
            player_moving_ = true;
            player_timer_ = 0.0;

            player_anim_.set_state("walking_s");
            player_anim_.get_animation().start();
        }
        else if (did == entity::did_attack || did == entity::did_miss)
        {
            if (did == entity::did_attack)
                attacking_ = targ;
            else
                missing_ = targ;
            player_destination_ = {loc.first * tile_size, loc.second * tile_size};
            player_delta_ = player_destination_ - player_coord_;
            player_moving_ = true;
            player_timer_ = 0.0;
            player_anim_.set_state("attacking_s");
            player_anim_.get_animation().start();
        }
    }

    weak_const_ptr get_attacking() const { return attacking_; }
    weak_const_ptr get_missing() const { return missing_; }

protected:
    the_game *the_game_;
    resource_manager *sprite_manager_;
    state_animator player_anim_;
    weak_ptr attacking_;
    weak_ptr missing_;

    // Smooth scrolling.
    bool player_moving_;
    double player_timer_;
    sf::Vector2f player_coord_;
    sf::Vector2f player_destination_;
    sf::Vector2f player_delta_;
};

class the_game_renderer
{
public:
    the_game_renderer(const resource_manager *sm, const the_game *tg, const player_controller *controller) :
        sprite_manager_(sm), the_game_(tg), controller_(controller)
    {
    }
    virtual ~the_game_renderer() { }

    virtual void update(double dt)
    {
        (void)dt;
    }

    virtual void render(sf::RenderWindow *win, const sf::Transform &trans=sf::Transform()) const
    {
        const region &reg = the_game_->get_region();

        for (size_t x = 0; x < reg.get_width(); ++x)
        {
            for (size_t y = 0; y < reg.get_height(); ++y)
            {
                sf::Transform t;
                t.translate(x * tile_size, y * tile_size);
                t.combine(trans);
                win->draw(sprite_manager_->acquire<sf::RectangleShape>("floor"), t);
                switch (reg.tile_at(x, y))
                {
                    case t_rocks:
                        win->draw(sprite_manager_->acquire<sf::RectangleShape>("rocks"), t);
                        break;

                    default:
                        break;
                }

                // Render plants.
                auto pl = the_game_->get_entity_at(x, y);
                if (pl.lock())
                {
                    std::string sprite = "";
                    if (auto sptr = std::dynamic_pointer_cast<plant>(pl.lock()))
                    {
                        sprite = sptr->get_type();
                    }

                    if (sprite != "")
                    {
                        auto temp = sprite_manager_->acquire<sf::RectangleShape>(sprite);
                        if (pl.lock() == controller_->get_attacking().lock())
                            temp.setFillColor(sf::Color(255, 0, 0, 255));
                        else if (pl.lock() == controller_->get_missing().lock())
                            temp.setFillColor(sf::Color(255, 255, 255, 255));

                        win->draw(temp, t);
                    }
                }
            }
        }

        sf::Transform t;
        t.translate(controller_->get_coord());
        t.combine(trans);
        win->draw(sprite_manager_->acquire<sf::RectangleShape>(controller_->get_animator().get_animation().get_texture()), t);
    }
protected:
    const resource_manager *sprite_manager_;
    const the_game *the_game_;
    const player_controller *controller_;
};

static inline void manage_sprite(resource_manager &sm, const resource_manager &rm, std::string key, double width, double height)
{
    sf::RectangleShape sprite({width, height});
    sprite.setTexture(&rm.acquire<sf::Texture>(key));
    sm.manage<sf::RectangleShape>(key, sprite);
}

class game_screen : public screen, private boost::noncopyable
{
public:
    game_screen(sf::RenderWindow *win, screen_manager *sm, const resource_manager *rm) :
        win_(win), screen_manager_(sm), resource_manager_(rm)
    {
        game_view_ = sf::View(sf::FloatRect(0, 0, 1, 1));
        game_view_.setViewport(sf::FloatRect(0, 0, 1.0, 0.66));
        hud_view_ = sf::View(sf::FloatRect(0, 0, 1, 1));
        hud_view_.setViewport(sf::FloatRect(0, 0.66, 1.0, 1.0));

        manage_sprite(sprite_manager_, *resource_manager_, "heart", 0.1, 0.1);
        manage_sprite(sprite_manager_, *resource_manager_, "energy", 0.1, 0.1);
        manage_sprite(sprite_manager_, *resource_manager_, "noheart", 0.1, 0.1);
        manage_sprite(sprite_manager_, *resource_manager_, "noenergy", 0.1, 0.1);

        manage_sprite(sprite_manager_, *resource_manager_, "root", tile_size, tile_size);
        manage_sprite(sprite_manager_, *resource_manager_, "vine", tile_size, tile_size);
        manage_sprite(sprite_manager_, *resource_manager_, "pod", tile_size, tile_size);
        manage_sprite(sprite_manager_, *resource_manager_, "player", tile_size, tile_size);
        manage_sprite(sprite_manager_, *resource_manager_, "player_sw1", tile_size, tile_size);
        manage_sprite(sprite_manager_, *resource_manager_, "player_sw2", tile_size, tile_size);
        manage_sprite(sprite_manager_, *resource_manager_, "player_sa", tile_size, tile_size);

        manage_sprite(sprite_manager_, *resource_manager_, "floor", tile_size, tile_size);
        manage_sprite(sprite_manager_, *resource_manager_, "rocks", tile_size, tile_size);

        using std::placeholders::_1;
        using std::placeholders::_2;
        using std::placeholders::_3;
        the_game_.reset(new the_game(std::bind(&game_screen::on_did, std::ref(*this), _1, _2, _3)));
        controller_.reset(new player_controller(the_game_.get(), &sprite_manager_));
        the_game_renderer_.reset(new the_game_renderer(&sprite_manager_, the_game_.get(), controller_.get()));
    }

    virtual ~game_screen() { }

    const player_controller &get_player_controller() const { return *controller_.get(); }

    virtual bool stops_events() const { return true; }
    virtual bool stops_updating() const { return true; }
    virtual bool stops_rendering() const { return true; }

    virtual void on_enter()
    {
        std::cout << "game_screen::on_enter" << std::endl;
    }

    virtual void on_exit()
    {
        std::cout << "game_screen::on_exit" << std::endl;
    }

    virtual void on_event(const sf::Event &event)
    {
        (void)event;
    }

    virtual void on_update(double dt)
    {
        controller_->update(dt);
        game_view_.setCenter(
            controller_->get_coord().x + tile_size / 2,
            controller_->get_coord().y + tile_size / 2);
        the_game_renderer_->update(dt);
    }

    virtual void on_render()
    {
        win_->setView(game_view_);
        the_game_renderer_->render(win_);
        win_->setView(hud_view_);

        auto vitals = the_game_->get_player().get_vitals();
        for (ssize_t i = 0; i < vitals.max_hearts; ++i)
        {
            sf::Transform trans;
            trans.translate(0.1 * i, 0);
            if (i > vitals.max_hearts - vitals.hearts)
                win_->draw(sprite_manager_.acquire<sf::RectangleShape>("noheart"), trans);
            else
                win_->draw(sprite_manager_.acquire<sf::RectangleShape>("heart"), trans);
        }
        auto atts = the_game_->get_player().get_attributes();
        for (ssize_t i = 0; i < atts.max_energy; ++i)
        {
            sf::Transform trans;
            trans.translate(0.1 * i, 0.1);
            if (i > atts.max_energy - atts.energy)
                win_->draw(sprite_manager_.acquire<sf::RectangleShape>("noenergy"), trans);
            else
                win_->draw(sprite_manager_.acquire<sf::RectangleShape>("energy"), trans);
        }
    }

    virtual void on_did(std::weak_ptr<entity> src, entity::did did, std::weak_ptr<entity> targ)
    {
        if (auto sptr = src.lock())
        {
            if (sptr.get() == &the_game_->get_player())
                controller_->player_did(did, targ);
        }
    }

protected:
    sf::RenderWindow *win_;
    sf::View game_view_;
    sf::View hud_view_;
    screen_manager *screen_manager_;
    resource_manager sprite_manager_;
    const resource_manager *resource_manager_;

    std::unique_ptr<the_game> the_game_;
    std::unique_ptr<the_game_renderer> the_game_renderer_;
    std::unique_ptr<player_controller> controller_;
};

#endif
