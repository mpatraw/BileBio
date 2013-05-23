
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
        //sf::RectangleShape sprite(sf::Vector2f(width_, height_));
        //sprite.setTexture(&tex);
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

class renderer
{
public:
    virtual void update(double dt) = 0;
    virtual void render(sf::RenderWindow *win, const sf::Transform &trans=sf::Transform()) const = 0;
};

class animation_renderer : public renderer
{
public:
    animation_renderer(resource_manager *rm, animation *anim, double size=0.2) :
        resource_manager_ref_(rm), animation_ref_(anim), size_(size)
    {
        auto frames = anim->get_frames();
        for (size_t i = 0; i < frames.size(); ++i)
        {
            sf::RectangleShape sprite(sf::Vector2f(size_, size_));
            sprite.setTexture(&resource_manager_ref_->acquire<sf::Texture>(frames[i]));
            map_[frames[i]] = std::move(sprite);
        }
    }
    virtual ~animation_renderer() { }

    virtual void update(double dt)
    {
        animation_ref_->update(dt);
    }

    virtual void render(sf::RenderWindow *win, const sf::Transform &trans=sf::Transform()) const
    {
        win->draw(map_.at(animation_ref_->get_texture()), trans);
    }
protected:
    resource_manager *resource_manager_ref_;
    animation *animation_ref_;
    double size_;

    std::unordered_map<std::string, sf::RectangleShape> map_;
};

class region_renderer : public renderer
{
public:
    region_renderer(resource_manager *rm, region *reg, double tile_size=0.2) :
        resource_manager_ref_(rm), region_ref_(reg), tile_size_(tile_size)
    {
        grass_ = sf::RectangleShape(sf::Vector2f(tile_size_, tile_size_));
        grass_.setTexture(&resource_manager_ref_->acquire<sf::Texture>("grass"));

        water_anim_.set_duration(1.0);
        water_anim_.set_loops(true);
        water_anim_.add_frame("water1");
        water_anim_.add_frame("water2");
        water_anim_.start();
        water_anim_renderer_.reset(new animation_renderer(rm, &water_anim_));
    }
    virtual ~region_renderer() { }

    virtual void update(double dt)
    {
        water_anim_renderer_->update(dt);
    }

    virtual void render(sf::RenderWindow *win, const sf::Transform &trans=sf::Transform()) const
    {
        (void)trans;
        for (size_t x = 0; x < region_ref_->get_width(); ++x)
        {
            for (size_t y = 0; y < region_ref_->get_height(); ++y)
            {
                sf::Transform trans;
                trans.translate(x * tile_size_, y * tile_size_);
                win->draw(grass_, trans);
                if (region_ref_->tile_at(x, y).o == o_tree)
                {
                    water_anim_renderer_->render(win, trans);
                }
            }
        }
    }
protected:
    resource_manager *resource_manager_ref_;
    region *region_ref_;
    double tile_size_;

    sf::RectangleShape grass_;
    animation water_anim_;
    std::unique_ptr<animation_renderer> water_anim_renderer_;
};

class game_screen : public screen, private boost::noncopyable
{
public:
    game_screen(sf::RenderWindow *win, screen_manager *sm, resource_manager *rm) :
        win_ref_(win), screen_manager_ref_(sm), resource_manager_ref_(rm)
    {
        game_view_ = sf::View(sf::FloatRect(0, 0, 1, 1));
        game_view_.setViewport(sf::FloatRect(0, 0, 1.0, 0.66));
        hud_view_ = sf::View(sf::FloatRect(0, 0, 1, 1));
        hud_view_.setViewport(sf::FloatRect(0, 0.66, 1.0, 1.0));

        player_anim_.set_duration(0.5);
        player_anim_.set_loops(true);
        player_anim_.add_frame("player");
        player_anim_.start();
        player_anim_renderer_.reset(new animation_renderer(rm, &player_anim_));

        player_dx_ = 0.0;
        player_dy_ = 0.0;
        player_destx_ = 0.0;
        player_desty_ = 0.0;
        player_moving_ = false;
        player_timer_ = 0.0;

        region_.reset(new region());
        region_renderer_.reset(new region_renderer(rm, region_.get()));
    }

    virtual ~game_screen() { }

    virtual bool stops_events() const { return true; }
    virtual bool stops_updating() const { return true; }
    virtual bool stops_rendering() const { return true; }

    virtual void on_enter()
    {
        region_->generate(50, 50);
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
        if (!player_moving_)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            {
                player_destx_ = player_location_.x;
                player_desty_ = player_location_.y - 0.2;
                player_dx_ = 0.0;
                player_dy_ = -0.2;
                player_moving_ = true;
                player_timer_ = 0.0;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            {
                player_destx_ = player_location_.x - 0.2;
                player_desty_ = player_location_.y;
                player_dx_ = -0.2;
                player_dy_ = 0.0;
                player_moving_ = true;
                player_timer_ = 0.0;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            {
                player_destx_ = player_location_.x;
                player_desty_ = player_location_.y + 0.2;
                player_dx_ = 0.0;
                player_dy_ = 0.2;
                player_moving_ = true;
                player_timer_ = 0.0;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            {
                player_destx_ = player_location_.x + 0.2;
                player_desty_ = player_location_.y;
                player_dx_ = 0.2;
                player_dy_ = 0.0;
                player_moving_ = true;
                player_timer_ = 0.0;
            }
        }

        if (player_moving_)
        {
            player_timer_ += dt;
            if (player_timer_ >= 0.5)
            {
                player_location_ = sf::Vector2f(player_destx_, player_desty_);
                player_moving_ = false;
            }
            else
            {
                player_location_ = sf::Vector2f(
                    player_location_.x + player_dx_ * (dt / 0.5),
                    player_location_.y + player_dy_ * (dt / 0.5));
            }
        }
        game_view_.setCenter(
            player_location_.x + 0.2 / 2,
            player_location_.y + 0.2 / 2);
        region_renderer_->update(dt);
        player_anim_renderer_->update(dt);
    }

    virtual void on_render()
    {
        win_ref_->setView(game_view_);
        region_renderer_->render(win_ref_);
        sf::Transform trans;
        trans.translate(player_location_);
        player_anim_renderer_->render(win_ref_, trans);

        win_ref_->setView(hud_view_);
        //win_ref_->draw(frame_);
    }

protected:
    sf::RenderWindow *win_ref_;
    sf::View game_view_;
    sf::View hud_view_;
    screen_manager *screen_manager_ref_;
    resource_manager *resource_manager_ref_;

    std::unique_ptr<region> region_;
    animation player_anim_;
    sf::Vector2f player_location_;

    std::unique_ptr<region_renderer> region_renderer_;
    std::unique_ptr<animation_renderer> player_anim_renderer_;

    bool player_moving_;
    double player_destx_;
    double player_desty_;
    double player_dx_;
    double player_dy_;
    double player_timer_;
};

#endif
