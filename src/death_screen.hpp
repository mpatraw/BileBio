
#ifndef DEATH_SCREEN_HPP
#define DEATH_SCREEN_HPP

#include <SFML/Graphics.hpp>

#include <game_screen.hpp>
#include <screen_manager.hpp>
#include <utils/resource_manager.hpp>
#include <ui/simple_ui.hpp>

class death_screen : public screen, private boost::noncopyable
{
public:
    death_screen(sf::RenderWindow *win, screen_manager *sm, const resource_manager *rm) :
        win_(win), screen_manager_(sm), resource_manager_(rm)
    {
        sf::View view(sf::FloatRect(0, 0, 1.0, 1.0));
        view.setViewport(sf::FloatRect(0, 0, 1.0, 1.0));
        win->setView(view);

        death_screen_sprite_ = sf::RectangleShape(sf::Vector2f(0.8, 0.4));
        death_screen_sprite_.setTexture(&resource_manager_->acquire<sf::Texture>("death_screen"));
        death_screen_sprite_.setPosition(0.1, 0.3);
    }

    virtual ~death_screen() { }

    virtual bool stops_events() const { return true; }
    virtual bool stops_updating() const { return true; }
    virtual bool stops_rendering() const { return true; }

    virtual void on_enter()
    {
        std::cout << "death_screen_screen::on_enter" << std::endl;
    }

    virtual void on_exit()
    {
        std::cout << "death_screen_screen::on_exit" << std::endl;
    }

    virtual void on_event(const sf::Event &event)
    {
        if (event.type == sf::Event::KeyPressed)
        {
            screen_manager_->pop_screen();
        }
    }

    virtual void on_update(double dt)
    {
        (void)dt;
    }

    virtual void on_render()
    {
        win_->draw(death_screen_sprite_);
    }

protected:
    sf::RenderWindow *win_;
    screen_manager *screen_manager_;
    const resource_manager *resource_manager_;

    sf::RectangleShape death_screen_sprite_;

private:
};

#endif
