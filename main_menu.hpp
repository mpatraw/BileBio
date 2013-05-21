
#ifndef MAIN_MENU_HPP
#define MAIN_MENU_HPP

#include <SFML/Graphics.hpp>

#include "game.hpp"
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

class main_menu : public screen, private boost::noncopyable
{
public:
    main_menu(sf::RenderWindow *win, screen_manager *sm, resource_manager *rm) :
        win_(win), screen_manager_(sm), resource_manager_(rm)
    {
        sf::View view(sf::FloatRect(0, 0, 320, 320));
        view.setViewport(sf::FloatRect(0, 0, 1.0, 1.0));
        win->setView(view);

        title_sprite_ = sf::Sprite(resource_manager_->acquire<sf::Texture>("title"));
        title_sprite_.setPosition(320 / 2 - 192 / 2, 0);

        sf::Font &font = resource_manager_->acquire<sf::Font>("Pokemon GB");

        sf::Text qtext("Quit", font, 12);
        quit_button_ = simple_button(win_, sf::Sprite(resource_manager_->acquire<sf::Texture>("frame")),
            std::move(qtext), std::bind(&main_menu::on_quit_button_click, std::ref(*this)));
        quit_button_.set_center(160, 240);

        sf::Text ntext("New", font, 12);
        new_game_button_ = simple_button(win_, sf::Sprite(resource_manager_->acquire<sf::Texture>("frame")),
            std::move(ntext), std::bind(&main_menu::on_new_game_button_click, std::ref(*this)));
        new_game_button_.set_center(160, 160);
    }

    virtual ~main_menu() { }

    virtual bool stops_events() const { return true; }
    virtual bool stops_updating() const { return true; }
    virtual bool stops_rendering() const { return true; }

    virtual void on_enter()
    {
        std::cout << "main_menu::on_enter" << std::endl;
    }

    virtual void on_exit()
    {
        std::cout << "main_menu::on_exit" << std::endl;
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

    virtual void on_render(double dt)
    {
        (void)dt;
        win_->draw(title_sprite_);
        quit_button_.on_render(dt);
        new_game_button_.on_render(dt);
    }

protected:
    sf::RenderWindow *win_;
    screen_manager *screen_manager_;
    resource_manager *resource_manager_;

    sf::Sprite title_sprite_;
    simple_button new_game_button_;
    simple_button quit_button_;

private:
    void on_new_game_button_click()
    {
        screen_manager_->push_screen(new game(win_, screen_manager_, resource_manager_));
    }

    void on_quit_button_click()
    {
        screen_manager_->pop_screen();
    }
};

#endif
