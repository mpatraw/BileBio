
#ifndef GAME_HPP
#define GAME_HPP

#include <boost/noncopyable.hpp>

#include <SFML/Graphics.hpp>

#include <drunkard.h>
#include "resource_manager.hpp"
#include "simple_ui.hpp"
#include "screen_manager.hpp"

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
        game_view_ = sf::View(sf::FloatRect(0, 0, 320, 320));
        game_view_.setViewport(sf::FloatRect(0, 0, 1.0, 0.66));
        hud_view_ = sf::View(sf::FloatRect(0, 0, 320, 320));
        hud_view_.setViewport(sf::FloatRect(0, 0.66, 1.0, 1.0));
        grass_ = sf::Sprite(resource_manager_->acquire<sf::Texture>("grass"));
        tree_ = sf::Sprite(resource_manager_->acquire<sf::Texture>("tree"));
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
        if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::W)
            {
                game_view_.move(0, -10);
            }
            if (event.key.code == sf::Keyboard::A)
            {
                game_view_.move(-10, 0);
            }
            if (event.key.code == sf::Keyboard::S)
            {
                game_view_.move(0, 10);
            }
            if (event.key.code == sf::Keyboard::D)
            {
                game_view_.move(10, 0);
            }
        }
    }

    virtual void on_update(double dt)
    {
        (void)dt;
    }

    virtual void on_render(double dt)
    {
        (void)dt;

        win_->setView(game_view_);
        auto spr_size = grass_.getTexture()->getSize();
        spr_size.x *= grass_.getScale().x;
        spr_size.y *= grass_.getScale().y;

        for (size_t x = 0; x < forest_.get_width(); ++x)
        {
            for (size_t y = 0; y < forest_.get_height(); ++y)
            {
                grass_.setPosition(x * spr_size.x, y * spr_size.y);
                win_->draw(grass_);
                if (forest_.tile_at(x, y).o == o_tree)
                {
                    tree_.setPosition(x * spr_size.x, y * spr_size.y);
                    win_->draw(tree_);
                }
            }
        }
    }

protected:
    sf::RenderWindow *win_;
    sf::View game_view_;
    sf::View hud_view_;
    screen_manager *screen_manager_;
    resource_manager *resource_manager_;
    forest forest_;

    sf::Sprite grass_;
    sf::Sprite tree_;
};

#endif
