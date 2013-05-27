
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

static constexpr double tiles_per_screen = 7.0;
static constexpr double tile_size = 1.0 / tiles_per_screen;

class player_controller
{
public:
    player_controller(the_game *tg, resource_manager *sm) :
        the_game_(tg), sprite_manager_(sm)
    {
        player_anim_.set_state("walking");
        player_anim_.get_animation().set_duration(0.5);
        player_anim_.get_animation().set_loops(true);
        player_anim_.get_animation().add_frame("player");
        player_anim_.get_animation().start();
        player_location_ = {the_game_->get_player().get_x() * tile_size, the_game_->get_player().get_y() * tile_size};
        player_destination_ = {the_game_->get_player().get_x() * tile_size, the_game_->get_player().get_y() * tile_size};
        player_moving_ = false;
    }

    virtual ~player_controller() { }

    const sf::Vector2f &get_location() const { return player_location_; }
    const state_animator &get_animator() const { return player_anim_; }

    virtual void update(double dt)
    {
        if (!player_moving_)
        {
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

        if (player_moving_)
        {
            player_timer_ += dt;
            if (player_timer_ >= 0.5)
            {
                player_location_ = player_destination_;
                player_moving_ = false;
                the_game_->rest_act();
            }
            else
            {
                player_location_ = sf::Vector2f(
                    player_location_.x + player_delta_.x * (dt / 0.5),
                    player_location_.y + player_delta_.y * (dt / 0.5));
            }
        }

        player_anim_.get_animation().update(dt);
    }

    virtual void player_did(entity::did did, entity *targ)
    {
        if (did == entity::did_move)
        {
            player_destination_ = {the_game_->get_player().get_x() * tile_size,
                the_game_->get_player().get_y() * tile_size};
            player_delta_ = player_destination_ - player_location_;
            player_moving_ = true;
            player_timer_ = 0.0;
        }
        else if (did == entity::did_attack || did == entity::did_miss)
        {
            std::printf("%p\n", targ);
            player_destination_ = {the_game_->get_player().get_x() * tile_size,
                the_game_->get_player().get_y() * tile_size};
            player_delta_ = player_destination_ - player_location_;
            player_moving_ = true;
            player_timer_ = 0.0;
        }
    }

protected:
    the_game *the_game_;
    resource_manager *sprite_manager_;
    state_animator player_anim_;

    // Smooth scrolling.
    bool player_moving_;
    double player_timer_;
    sf::Vector2f player_location_;
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
                auto pl = the_game_->get_plant_at(x, y);
                if (pl)
                {
                    std::string sprite = "";
                    switch (pl->get_type())
                    {
                    case plant::p_root:
                        sprite = "root";
                        break;
                    case plant::p_vine:
                        sprite = "vine";
                        break;
                    case plant::p_growing:
                        sprite = "growing";
                        break;
                    default:
                        sprite = "tree";
                        break;
                    }
                    win->draw(sprite_manager_->acquire<sf::RectangleShape>(sprite), t);
                }
            }
        }

        sf::Transform t;
        t.translate(controller_->get_location());
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
        manage_sprite(sprite_manager_, *resource_manager_, "growing", tile_size, tile_size);
        manage_sprite(sprite_manager_, *resource_manager_, "player", tile_size, tile_size);

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
            controller_->get_location().x + tile_size / 2,
            controller_->get_location().y + tile_size / 2);
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

    virtual void on_did(entity *src, entity::did did, entity *targ)
    {
        if (src == &the_game_->get_player())
        {
            controller_->player_did(did, targ);
        }
        else
        {
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
