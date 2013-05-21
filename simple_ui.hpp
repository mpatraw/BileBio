
#ifndef SIMPLE_UI_HPP
#define SIMPLE_UI_HPP

#include <functional>

#include <SFML/Graphics.hpp>

class simple_ui
{
public:
    virtual void on_event(const sf::Event &event) = 0;
    virtual void on_update(double dt) = 0;
    virtual void on_render(double dt) = 0;
};

class simple_button : public simple_ui
{
public:
    simple_button()
    {
    }
    simple_button(sf::RenderWindow *win, sf::Sprite bg, sf::Text text, std::function<void()> on_click) :
        win_(win), bg_sprite_(bg), text_(text), on_click_(on_click)
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
            auto pos = win_->mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
            auto x = pos.x;
            auto y = pos.y;
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

    virtual void on_render(double dt)
    {
        (void)dt;
        win_->draw(bg_sprite_);
        win_->draw(text_);
    }

    virtual void set_center(double x, double y)
    {
        auto spr_size = bg_sprite_.getTexture()->getSize();
        spr_size.x *= bg_sprite_.getScale().x;
        spr_size.y *= bg_sprite_.getScale().y;

        bg_sprite_.setPosition(static_cast<int>(x - spr_size.x / 2),
            static_cast<int>(y - spr_size.y / 2));

        auto txt_size = text_.getGlobalBounds();
        text_.setPosition(static_cast<int>(x - txt_size.width / 2),
            static_cast<int>(y - txt_size.height / 2));
    }

private:
    sf::RenderWindow *win_;
    sf::Sprite bg_sprite_;
    sf::Text text_;
    std::function<void()> on_click_;
};

#endif
