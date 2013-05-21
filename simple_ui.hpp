
#ifndef SIMPLE_UI_HPP
#define SIMPLE_UI_HPP

#include <functional>

#include <SFML/Graphics.hpp>

class simple_ui
{
public:
    virtual void on_event(const sf::Event &event) = 0;
    virtual void on_update(double dt) = 0;
    virtual void on_render(sf::RenderWindow &win, double dt) = 0;
};

class simple_button : public simple_ui
{
public:
    simple_button()
    {
    }
    simple_button(sf::Sprite bg, sf::Text text, std::function<void()> on_click) :
        bg_sprite_(bg), text_(text), on_click_(on_click)
    {
    }
    virtual ~simple_button() { }

    simple_button(const simple_button &but) = default;
    simple_button(simple_button &&but) = default;
    simple_button &operator=(const simple_button &but) = default;
    simple_button &operator=(simple_button &&but) = default;

    virtual void on_event(const sf::Event &event)
    {
        if (event.type == sf::Event::MouseButtonReleased)
        {
            auto position = bg_sprite_.getPosition();
            auto size = bg_sprite_.getTexture()->getSize();
            auto x = event.mouseButton.x;
            auto y = event.mouseButton.y;
            if (x >= position.x && x < position.x + size.x &&
                y >= position.y && y < position.y + size.y)
            {
                on_click_();
            }
        }
    }

    virtual void on_update(double dt)
    {
        (void)dt;
    }

    virtual void on_render(sf::RenderWindow &win, double dt)
    {
        (void)dt;
        win.draw(bg_sprite_);
        win.draw(text_);
    }

    virtual void set_center(double x, double y)
    {
        auto spr_size = bg_sprite_.getTexture()->getSize();
        spr_size.x *= bg_sprite_.getScale().x;
        spr_size.y *= bg_sprite_.getScale().y;

        bg_sprite_.setPosition(x - spr_size.x / 2, y - spr_size.y / 2);

        auto txt_size = text_.getGlobalBounds();
        text_.setPosition(x - txt_size.width / 2, y - txt_size.height / 2);
    }

private:
    sf::Sprite bg_sprite_;
    sf::Text text_;
    std::function<void()> on_click_;
};

#endif
