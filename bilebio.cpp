

#include <iostream>

#include <boost/noncopyable.hpp>

#include <SFML/Graphics.hpp>

#include "main_menu.hpp"
#include "resource_manager.hpp"
#include "screen_manager.hpp"

int main()
{
    auto vm = sf::VideoMode(320, 480, sf::Style::Titlebar | sf::Style::Close);
    sf::RenderWindow window(vm, "BileBio");
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(30);

    resource_manager rm;

    sf::Font font;
    font.loadFromFile("resources/Nouveau_IBM.ttf");
    rm.manage<sf::Font>("Nouveau_IBM", font);
    font.loadFromFile("resources/Pokemon GB.ttf");
    rm.manage<sf::Font>("Pokemon GB", font);
    sf::Texture sprite;
    sprite.loadFromFile("resources/title.png");
    rm.manage<sf::Texture>("title", sprite);
    sprite.loadFromFile("resources/vine.png");
    rm.manage<sf::Texture>("vine", sprite);
    sprite.loadFromFile("resources/frame.png");
    rm.manage<sf::Texture>("frame", sprite);
    sprite.loadFromFile("resources/dirt.png");
    rm.manage<sf::Texture>("dirt", sprite);
    sprite.loadFromFile("resources/rock.png");
    rm.manage<sf::Texture>("rock", sprite);
    sprite.loadFromFile("resources/grass.png");
    rm.manage<sf::Texture>("grass", sprite);
    sprite.loadFromFile("resources/tree.png");
    rm.manage<sf::Texture>("tree", sprite);
    sprite.loadFromFile("resources/player.png");
    rm.manage<sf::Texture>("player", sprite);


    screen_manager sm;
    sm.push_screen(new main_menu(&window, &sm, &rm));

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
        sm.render(dt.asSeconds());
        window.display();
    }

    return 0;
}
