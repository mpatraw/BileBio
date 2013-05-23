
#ifndef MAIN_MENU_HPP
#define MAIN_MENU_HPP

#include <SFML/Graphics.hpp>

#include "game_screen.hpp"
#include "resource_manager.hpp"
#include "simple_ui.hpp"
#include "screen_manager.hpp"

/*
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

class configuration : private boost::noncopyable
{
public:
    configuration(std::string filename) :
        filename_(filename)
    {
        boost::property_tree::json_parser::read_json(filename_, pt_);
    }

    ~configuration()
    {
        boost::property_tree::json_parser::write_json(filename_, pt_);
    }

    template <typename T>
    const T &get(std::string key) const
    {
        return pt_.get<T>(key);
    }

    template <typename T>
    void put(std::string key, const T &val)
    {
        pt_.put(key, val);
    }

private:
    std::string filename_;
    boost::property_tree::ptree pt_;
};
*/

class main_menu_screen : public screen, private boost::noncopyable
{
public:
    main_menu_screen(sf::RenderWindow *win, screen_manager *sm, resource_manager *rm) :
        win_(win), screen_manager_ref_(sm), resource_manager_ref_(rm)
    {
        sf::View view(sf::FloatRect(0, 0, 1.0, 1.0));
        view.setViewport(sf::FloatRect(0, 0, 1.0, 1.0));
        win->setView(view);

        title_sprite_ = sf::RectangleShape(sf::Vector2f(0.5, 0.2));
        title_sprite_.setTexture(&resource_manager_ref_->acquire<sf::Texture>("title"));
        title_sprite_.setPosition(0.25, 0);

        sf::Font &font = resource_manager_ref_->acquire<sf::Font>("Pokemon GB");

        new_game_button_ = simple_button(win_, 0.2, 0.1, std::bind(&main_menu_screen::on_new_game_button_click, std::ref(*this)));
        new_game_button_.set_background(resource_manager_ref_->acquire<sf::Texture>("frame"));
        new_game_button_.set_text("New", font, 24);
        new_game_button_.set_center(0.5, 0.4);

        quit_button_ = simple_button(win_, 0.2, 0.1, std::bind(&main_menu_screen::on_quit_button_click, std::ref(*this)));
        quit_button_.set_background(resource_manager_ref_->acquire<sf::Texture>("frame"));
        quit_button_.set_text("Quit", font, 24);
        quit_button_.set_center(0.5, 0.6);
    }

    virtual ~main_menu_screen() { }

    virtual bool stops_events() const { return true; }
    virtual bool stops_updating() const { return true; }
    virtual bool stops_rendering() const { return true; }

    virtual void on_enter()
    {
        std::cout << "main_menu_screen::on_enter" << std::endl;
    }

    virtual void on_exit()
    {
        std::cout << "main_menu_screen::on_exit" << std::endl;
    }

    virtual void on_event(const sf::Event &event)
    {
        quit_button_.on_event(event);
        new_game_button_.on_event(event);
    }

    virtual void on_update(double dt)
    {
        quit_button_.on_update(dt);
        new_game_button_.on_update(dt);
    }

    virtual void on_render()
    {
        win_->draw(title_sprite_);
        quit_button_.on_render();
        new_game_button_.on_render();
    }

protected:
    sf::RenderWindow *win_;
    screen_manager *screen_manager_ref_;
    resource_manager *resource_manager_ref_;

    sf::RectangleShape title_sprite_;
    simple_button new_game_button_;
    simple_button quit_button_;

private:
    void on_new_game_button_click()
    {
        screen_manager_ref_->push_screen(new game_screen(win_, screen_manager_ref_, resource_manager_ref_));
    }

    void on_quit_button_click()
    {
        screen_manager_ref_->pop_screen();
    }
};

#endif
