
#ifndef SIMPLE_UI_HPP
#define SIMPLE_UI_HPP

#include <functional>

#include <SFML/Graphics.hpp>

sf::Texture draw_text(std::string txt, sf::Font font, size_t pt)
{
    sf::Text text(txt.c_str(), font, pt);
    sf::RenderTexture rend;
    rend.create(text.getLocalBounds().width, text.getLocalBounds().height);
    rend.clear(sf::Color(0, 0, 0, 0));
    rend.display();
    rend.draw(text);
    return std::move(rend.getTexture());
}

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
    simple_button(sf::RenderWindow *win, double width, double height, std::function<void()> on_click) :
        win_(win), width_(width), height_(height), on_click_(on_click)
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
            auto size = bg_sprite_.getSize();
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

    virtual void set_background(const sf::Texture &tex)
    {

        bg_sprite_ = sf::RectangleShape(sf::Vector2f(width_, height_));
        bg_sprite_.setTexture(&tex);
    }

    virtual void set_text(std::string str, sf::Font font, size_t pt)
    {
        text_texture_ = draw_text(str.c_str(), font, pt);
        text_ = sf::RectangleShape(sf::Vector2f(width_ / 2, height_ / 2));
        text_.setTexture(&text_texture_);
    }

    virtual void set_center(double x, double y)
    {
        auto spr_size = bg_sprite_.getSize();
        bg_sprite_.setPosition(x - spr_size.x / 2, y - spr_size.y / 2);

        auto txt_size = text_.getSize();
        text_.setPosition(x - txt_size.x / 2, y - txt_size.y / 2);
    }

private:
    sf::RenderWindow *win_;
    double width_;
    double height_;
    sf::RectangleShape bg_sprite_;
    sf::RectangleShape text_;
    sf::Texture text_texture_;
    std::function<void()> on_click_;
};

#endif
