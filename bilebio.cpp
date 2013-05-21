

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
    sf::Font gb;
    gb.loadFromFile("resources/Pokemon GB.ttf");
    sf::Texture title;
    title.loadFromFile("resources/title.png");
    sf::Texture vine;
    vine.loadFromFile("resources/vine.png");
    sf::Texture frame;
    frame.loadFromFile("resources/frame.png");
    sf::Texture dirt;
    dirt.loadFromFile("resources/dirt.png");
    sf::Texture rock;
    rock.loadFromFile("resources/rock.png");
    sf::Texture grass;
    grass.loadFromFile("resources/grass.png");
    sf::Texture tree;
    tree.loadFromFile("resources/tree.png");

    rm.manage<sf::Font>("Nouveau_IBM", font);
    rm.manage<sf::Font>("Pokemon GB", gb);
    rm.manage<sf::Texture>("title", title);
    rm.manage<sf::Texture>("vine", vine);
    rm.manage<sf::Texture>("frame", frame);
    rm.manage<sf::Texture>("dirt", dirt);
    rm.manage<sf::Texture>("rock", rock);
    rm.manage<sf::Texture>("grass", grass);
    rm.manage<sf::Texture>("tree", tree);

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
