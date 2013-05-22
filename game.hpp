
#ifndef GAME_HPP
#define GAME_HPP

#include <boost/noncopyable.hpp>

#include <SFML/Graphics.hpp>

#include <drunkard.h>
#include "resource_manager.hpp"
#include "simple_ui.hpp"
#include "screen_manager.hpp"

class animation : private boost::noncopyable
{
public:
    animation(sf::RenderWindow *win=nullptr, double width=0.0, double height=0.0, double duration=0.0, bool loop=false) :
        win_(win), width_(width), height_(height), duration_(duration), loops_(loop)
    {
        x_ = 0.0;
        y_ = 0.0;
        current_frame_ = -1;
        paused_ = true;
        done_ = true;
        time_accumulator_ = 0.0;
    }

    void set_render_window(sf::RenderWindow *win)
    {
        win_ = win;
    }

    double get_width() const { return width_; }
    double get_height() const { return height_; }

    void set_size(double width, double height)
    {
        width_ = width;
        height_ = height;
    }

    double get_x() const { return x_; }
    double get_y() const { return y_; }

    void set_position(double x, double y)
    {
        x_ = x;
        y_ = y;
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

    void add_frame(const sf::Texture &tex)
    {
        sf::RectangleShape sprite(sf::Vector2f(width_, height_));
        sprite.setTexture(&tex);
        frames_.push_back(std::move(sprite));
    }

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

    void render()
    {
        frames_[current_frame_].setPosition(x_, y_);
        win_->draw(frames_[current_frame_]);
    }

private:
    sf::RenderWindow *win_;
    double width_;
    double height_;
    double duration_;
    bool loops_;
    std::vector<sf::RectangleShape> frames_;
    double x_;
    double y_;
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

enum ground
{
    g_dirt,
    g_grass
};

enum object
{
    o_none,
    o_rock,
    o_tree
};

struct tile
{
    ground g;
    object o;
};

class forest : private boost::noncopyable
{
public:
    forest() { }
    forest(size_t width, size_t height) { generate(width, height); }
    ~forest() { }

    void generate(size_t width, size_t height)
    {
        tiles_.clear();

        tiles_.resize(width);
        for (size_t x = 0; x < width; ++x)
            tiles_[x].resize(height);

        unsigned *map = new unsigned[width * height];
        memset(map, 0, sizeof(unsigned) * width * height);
        drunkard *drunk = drunkard_create(map, width, height);
        drunkard_set_open_threshold(drunk, 1);
        drunkard_seed(drunk, time(NULL));

        // Carve seed.
        drunkard_start_fixed(drunk, width / 2, height / 2);
        drunkard_mark_1(drunk, 1);
        drunkard_flush_marks(drunk);

        // Carve.
        int tries = std::min(width, height);
        while (tries --> 0)
        {
            drunkard_start_random(drunk);
            drunkard_target_random_opened(drunk);

            while (!drunkard_is_on_opened(drunk))
            {
                drunkard_mark_plus(drunk, 1);
                drunkard_step_to_target(drunk, 0.90);
            }

            drunkard_flush_marks(drunk);
        }

        for (size_t y = 0; y < height; ++y)
        {
            for (size_t x = 0; x < width; ++x)
            {
                std::cout << map[y * width + x];
                if (map[y * width + x])
                    tiles_[x][y].o = o_none;
                else
                    tiles_[x][y].o = o_tree;
            }
            std::cout << std::endl;
        }

        width_ = width;
        height_ = height;
        drunkard_destroy(drunk);
        delete []map;
    }

    size_t get_width() const { return width_; }
    size_t get_height() const { return height_; }

    const tile &tile_at(int x, int y) const { return tiles_[x][y]; }
    tile &tile_at(int x, int y) { return tiles_[x][y]; }

private:
    size_t width_;
    size_t height_;
    std::vector<std::vector<tile>> tiles_;
};

class game : public screen, private boost::noncopyable
{
public:
    game(sf::RenderWindow *win, screen_manager *sm, resource_manager *rm) :
        win_(win), screen_manager_(sm), resource_manager_(rm)
    {
        game_view_ = sf::View(sf::FloatRect(0, 0, 1, 1));
        game_view_.setViewport(sf::FloatRect(0, 0, 1.0, 0.66));
        hud_view_ = sf::View(sf::FloatRect(0, 0, 1, 1));
        hud_view_.setViewport(sf::FloatRect(0, 0.66, 1.0, 1.0));
        grass_ = sf::RectangleShape(sf::Vector2f(0.2, 0.2));
        grass_.setTexture(&resource_manager_->acquire<sf::Texture>("grass"));
        frame_ = sf::RectangleShape(sf::Vector2f(1.0, 0.33));
        frame_.setTexture(&resource_manager_->acquire<sf::Texture>("frame"));

        water_anim_.set_render_window(win_);
        water_anim_.set_size(0.2, 0.2);
        water_anim_.set_duration(1.0);
        water_anim_.set_loops(true);
        water_anim_.add_frame(resource_manager_->acquire<sf::Texture>("water1"));
        water_anim_.add_frame(resource_manager_->acquire<sf::Texture>("water2"));
        water_anim_.start();

        player_anim_.set_render_window(win_);
        player_anim_.set_size(0.2, 0.2);
        player_anim_.set_duration(0.5);
        player_anim_.set_loops(true);
        player_anim_.add_frame(resource_manager_->acquire<sf::Texture>("player"));
        player_anim_.start();

        player_dx_ = 0.0;
        player_dy_ = 0.0;
        player_destx_ = 0.0;
        player_desty_ = 0.0;
        player_moving_ = false;
        player_timer_ = 0.0;
    }

    virtual ~game() { }

    virtual bool stops_events() const { return true; }
    virtual bool stops_updating() const { return true; }
    virtual bool stops_rendering() const { return true; }

    virtual void on_enter()
    {
        forest_.generate(50, 50);
        std::cout << "game::on_enter" << std::endl;
    }

    virtual void on_exit()
    {
        std::cout << "game::on_exit" << std::endl;
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
                player_destx_ = player_anim_.get_x();
                player_desty_ = player_anim_.get_y() - 0.2;
                player_dx_ = 0.0;
                player_dy_ = -0.2;
                player_moving_ = true;
                player_timer_ = 0.0;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            {
                player_destx_ = player_anim_.get_x() - 0.2;
                player_desty_ = player_anim_.get_y();
                player_dx_ = -0.2;
                player_dy_ = 0.0;
                player_moving_ = true;
                player_timer_ = 0.0;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            {
                player_destx_ = player_anim_.get_x();
                player_desty_ = player_anim_.get_y() + 0.2;
                player_dx_ = 0.0;
                player_dy_ = 0.2;
                player_moving_ = true;
                player_timer_ = 0.0;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            {
                player_destx_ = player_anim_.get_x() + 0.2;
                player_desty_ = player_anim_.get_y();
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
                player_anim_.set_position(player_destx_, player_desty_);
                player_moving_ = false;
            }
            else
            {
                player_anim_.set_position(
                    player_anim_.get_x() + player_dx_ * (dt / 0.5),
                    player_anim_.get_y() + player_dy_ * (dt / 0.5));
            }
        }
        game_view_.setCenter(
            player_anim_.get_x() + player_anim_.get_width() / 2,
            player_anim_.get_y() + player_anim_.get_height() / 2);
        water_anim_.update(dt);
        player_anim_.update(dt);
    }

    virtual void on_render()
    {
        win_->setView(game_view_);
        auto spr_size = grass_.getSize();

        for (size_t x = 0; x < forest_.get_width(); ++x)
        {
            for (size_t y = 0; y < forest_.get_height(); ++y)
            {
                grass_.setPosition(x * spr_size.x, y * spr_size.y);
                win_->draw(grass_);
                if (forest_.tile_at(x, y).o == o_tree)
                {
                    water_anim_.set_position(x * spr_size.x, y * spr_size.y);
                    water_anim_.render();
                }
            }
        }

        player_anim_.render();

        win_->setView(hud_view_);
        win_->draw(frame_);

    }

protected:
    sf::RenderWindow *win_;
    sf::View game_view_;
    sf::View hud_view_;
    screen_manager *screen_manager_;
    resource_manager *resource_manager_;
    forest forest_;

    sf::RectangleShape grass_;
    sf::RectangleShape frame_;
    animation water_anim_;
    animation player_anim_;
    bool player_moving_;
    double player_destx_;
    double player_desty_;
    double player_dx_;
    double player_dy_;
    double player_timer_;
};

#endif
