

#include <iostream>

#include <boost/noncopyable.hpp>

#include <SFML/Graphics.hpp>

#include "resource_manager.hpp"
#include "screen_manager.hpp"
#include "simple_ui.hpp"

class main_menu : public screen, private boost::noncopyable
{
public:
    main_menu(screen_manager *sm, resource_manager *rm) :
        screen_manager_(sm), resource_manager_(rm)
    {
        title_sprite_ = sf::Sprite(resource_manager_->acquire<sf::Texture>("title"));
        title_sprite_.setPosition(320/2 - 192/2, 0);

        sf::Text text;
        text.setFont(resource_manager_->acquire<sf::Font>("Nouveau_IBM"));
        text.setCharacterSize(14);
        text.setColor(sf::Color::White);
        text.setString("Quit");
        quit_button_ = simple_button(sf::Sprite(resource_manager_->acquire<sf::Texture>("frame")),
            std::move(text), std::bind(&main_menu::on_quit_button_click, std::ref(*this)));
        quit_button_.set_center(160, 400);
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
    }

    virtual void on_update(double dt)
    {
        quit_button_.on_update(dt);
    }

    virtual void on_render(sf::RenderWindow &win, double dt)
    {
        (void)dt;
        win.draw(title_sprite_);
        quit_button_.on_render(win, dt);
        /*
        resource_manager_->acquire<sf::Sprite>("frame").setPosition(320/2 - 64/2, 128);
        win.draw(resource_manager_->acquire<sf::Sprite>("frame"));
        resource_manager_->acquire<sf::Sprite>("frame").setPosition(320/2 - 64/2, 192);
        win.draw(resource_manager_->acquire<sf::Sprite>("frame"));
        resource_manager_->acquire<sf::Sprite>("frame").setPosition(320/2 - 64/2, 256);
        win.draw(resource_manager_->acquire<sf::Sprite>("frame"));
        resource_manager_->acquire<sf::Sprite>("frame").setPosition(320/2 - 64/2, 384);
        win.draw(resource_manager_->acquire<sf::Sprite>("frame"));
        */
    }

protected:
    screen_manager *screen_manager_;
    resource_manager *resource_manager_;

    sf::Sprite title_sprite_;
    simple_button quit_button_;

private:
    void on_quit_button_click()
    {
        screen_manager_->pop_screen();
    }
};

int main()
{
    auto vm = sf::VideoMode(320, 480, sf::Style::Titlebar | sf::Style::Close);
    sf::RenderWindow window(vm, "BileBio");
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(30);

    resource_manager rm;

    sf::Font font;
    font.loadFromFile("resources/Nouveau_IBM.ttf");
    sf::Texture title;
    title.loadFromFile("resources/title.png");
    sf::Texture vine;
    vine.loadFromFile("resources/vine.png");
    sf::Texture frame;
    frame.loadFromFile("resources/frame.png");

    rm.manage<sf::Font>("Nouveau_IBM", font);
    rm.manage<sf::Texture>("title", title);
    rm.manage<sf::Texture>("vine", vine);
    rm.manage<sf::Texture>("frame", frame);

    screen_manager sm;
    sm.push_screen(new main_menu(&sm, &rm));

    sf::Clock delta_clock;

    while (window.isOpen() && sm.number_of_screens() > 0)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else
                sm.on_event(event);
        }

        sf::Time dt = delta_clock.restart();

        sm.update(dt.asSeconds());

        window.clear();
        sm.render(window, dt.asSeconds());
        window.display();
    }

    return 0;
}
