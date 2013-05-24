
#ifndef GAME_HPP
#define GAME_HPP

#include <boost/noncopyable.hpp>
#include <unordered_map>
#include <vector>

#include <SFML/Graphics.hpp>

#include "resource_manager.hpp"
#include "simple_ui.hpp"
#include "screen_manager.hpp"
#include "the_game.hpp"

class animation : private boost::noncopyable
{
public:
    animation(double duration=0.0, bool loop=false) :
        duration_(duration),
        loops_(loop)
    {
        current_frame_ = -1;
        paused_ = true;
        done_ = true;
        time_accumulator_ = 0.0;
    }

    void set_loops(bool loops=true) { loops_ = loops; }
    void set_duration(double d) { duration_ = d; }

    void start() { paused_ = false; current_frame_ = 0; done_ = false; time_accumulator_ = 0.0; }
    void pause() { paused_ = true; }
    void resume() { paused_ = false; }
    void stop() { paused_ = true; current_frame_ = 0; done_ = true; time_accumulator_ = 0.0; }

    bool is_done() const { return done_; }

    void clear_frames()
    {
        current_frame_ = -1;
        frames_.clear();
    }

    void add_frame(const std::string &str)
    {
        frames_.push_back(std::move(str));
    }

    const std::vector<std::string> &get_frames() const { return frames_; }

    void update(double dt)
    {
        if (!paused_ && (!done_ || loops_))
        {
            time_accumulator_ += dt;
            if (time_accumulator_ >= duration_ / frames_.size() * current_frame_)
            {
                ++current_frame_;
                if (current_frame_ >= (ssize_t)frames_.size())
                {
                    done_ = true;
                    current_frame_ = 0;
                    time_accumulator_ -= duration_;
                }
            }
        }
    }

    const std::string &get_texture() const
    {
        return frames_[current_frame_];
    }

private:
    double duration_;
    bool loops_;
    std::vector<std::string> frames_;
    ssize_t current_frame_;
    bool paused_;
    bool done_;
    double time_accumulator_;
};

class state_animator : private boost::noncopyable
{
public:

private:

};

class the_game_renderer
{
public:
    the_game_renderer(const resource_manager *sm, const the_game *tg, double tile_size=0.2) :
        sprite_manager_(sm), the_game_(tg), tile_size_(tile_size)
    {
        water_anim_.set_duration(1.0);
        water_anim_.set_loops(true);
        water_anim_.add_frame("water1");
        water_anim_.add_frame("water2");
        water_anim_.start();
    }
    virtual ~the_game_renderer() { }

    virtual void update(double dt)
    {
        water_anim_.update(dt);
    }

    virtual void render(sf::RenderWindow *win, const sf::Transform &trans=sf::Transform()) const
    {
        const region &reg = the_game_->get_region();

        for (size_t x = 0; x < reg.get_width(); ++x)
        {
            for (size_t y = 0; y < reg.get_height(); ++y)
            {
                sf::Transform t;
                t.translate(x * tile_size_, y * tile_size_);
                t.combine(trans);
                win->draw(sprite_manager_->acquire<sf::RectangleShape>("grass"), t);
                switch (reg.tile_at(x, y))
                {
                    case t_water:
                        win->draw(sprite_manager_->acquire<sf::RectangleShape>(water_anim_.get_texture()), t);
                        break;

                    case t_tree:
                        win->draw(sprite_manager_->acquire<sf::RectangleShape>("tree"), t);
                        break;

                    case t_rock:
                        win->draw(sprite_manager_->acquire<sf::RectangleShape>("rock"), t);
                        break;

                    case t_dirt:
                        win->draw(sprite_manager_->acquire<sf::RectangleShape>("dirt"), t);
                        break;

                    case t_grass:
                        win->draw(sprite_manager_->acquire<sf::RectangleShape>("grass"), t);
                        break;

                    default:
                        break;
                }
            }
        }
    }
protected:
    const resource_manager *sprite_manager_;
    const the_game *the_game_;
    double tile_size_;

    animation water_anim_;
};

enum direction
{
    d_up, d_down, d_left, d_right
};

class player_controller
{
public:
    player_controller(the_game *tg, resource_manager *sm) :
        the_game_(tg), sprite_manager_(sm)
    {
        player_anim_.set_duration(0.5);
        player_anim_.set_loops(true);
        player_anim_.add_frame("player");
        player_anim_.start();
        player_location_ = {the_game_->get_player().get_x() * 0.2, the_game_->get_player().get_y() * 0.2};
    }

    virtual ~player_controller() { }

    const sf::Vector2f &get_location() const { return player_location_; }

    virtual void update(double dt)
    {
        ssize_t dx, dy;
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

        if (!player_moving_ && the_game_->move_player_by(dx, dy))
        {
            player_delta_ = {dx * 0.2, dy * 0.2};
            player_destination_ = player_location_ + player_delta_;
            player_moving_ = true;
            player_timer_ = 0.0;
        }

        if (player_moving_)
        {
            player_timer_ += dt;
            if (player_timer_ >= 0.5)
            {
                player_location_ = player_destination_;
                player_moving_ = false;
            }
            else
            {
                player_location_ = sf::Vector2f(
                    player_location_.x + player_delta_.x * (dt / 0.5),
                    player_location_.y + player_delta_.y * (dt / 0.5));
            }
        }
        player_anim_.update(dt);
    }

    virtual void render(sf::RenderWindow *win)
    {
        sf::Transform trans;
        trans.translate(player_location_);
        win->draw(sprite_manager_->acquire<sf::RectangleShape>(player_anim_.get_texture()), trans);
    }

protected:
    the_game *the_game_;
    resource_manager *sprite_manager_;
    animation player_anim_;

    bool player_moving_;
    double player_timer_;
    sf::Vector2f player_location_;
    sf::Vector2f player_destination_;
    sf::Vector2f player_delta_;
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

        manage_sprite(sprite_manager_, *resource_manager_, "grass", 0.2, 0.2);
        manage_sprite(sprite_manager_, *resource_manager_, "dirt", 0.2, 0.2);
        manage_sprite(sprite_manager_, *resource_manager_, "water1", 0.2, 0.2);
        manage_sprite(sprite_manager_, *resource_manager_, "water2", 0.2, 0.2);
        manage_sprite(sprite_manager_, *resource_manager_, "player", 0.2, 0.2);

        the_game_.reset(new the_game());
        the_game_renderer_.reset(new the_game_renderer(&sprite_manager_, the_game_.get()));

        controller_.reset(new player_controller(the_game_.get(), &sprite_manager_));
    }

    virtual ~game_screen() { }

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
            controller_->get_location().x + 0.2 / 2,
            controller_->get_location().y + 0.2 / 2);
        the_game_renderer_->update(dt);
    }

    virtual void on_render()
    {
        win_->setView(game_view_);
        the_game_renderer_->render(win_);
        controller_->render(win_);
        win_->setView(hud_view_);
        // Draw HUD here.
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
