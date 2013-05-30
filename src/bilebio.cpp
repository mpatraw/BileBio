

#include <iostream>

#include <boost/noncopyable.hpp>

#include <SFML/Graphics.hpp>

#include <main_menu_screen.hpp>
#include <screen_manager.hpp>
#include <utils/resource_manager.hpp>

// Yay for SFINAE
template <typename T>
struct has_load_from_file
{
    typedef char yes[1];
    typedef char no[2];

    template <typename U, bool (U::*)(const std::string &)> struct sfinae {};
    template <typename U> static yes &test(sfinae<U, &U::loadFromFile> *);
    template <typename U> static no &test(...);
    static const bool has = sizeof(test<T>(0)) == sizeof(yes);
};

template <typename T>
struct has_load_from_file_int_rect
{
    typedef char yes[1];
    typedef char no[2];

    template <typename U, bool (U::*)(const std::string &, const sf::IntRect &)> struct sfinae {};
    template <typename U> static yes &test(sfinae<U, &U::loadFromFile> *);
    template <typename U> static no &test(...);
    static const bool has = sizeof(test<T>(0)) == sizeof(yes);
};

template <class T, class = typename std::enable_if<has_load_from_file<T>::has>::type>
static inline bool load_resource(resource_manager &rm, const std::string &key, const std::string &filename)
{
    T t;
    if (!t.loadFromFile(filename))
        return false;
    rm.manage<T>(key, t);
    return true;
}

template <class T, class = typename std::enable_if<has_load_from_file_int_rect<T>::has>::type>
static inline bool load_resource(resource_manager &rm, const std::string &key, const std::string &filename, const sf::IntRect &rect=sf::IntRect())
{
    T t;
    if (!t.loadFromFile(filename, rect))
        return false;
    rm.manage<T>(key, t);
    return true;
}

int main()
{
    auto vm = sf::VideoMode(320, 480, sf::Style::Titlebar | sf::Style::Close);
    sf::RenderWindow window(vm, "BileBio");
    window.setVerticalSyncEnabled(true);
    //window.setFramerateLimit(60);

    resource_manager rm;

    load_resource<sf::Font>(rm, "Nouveau_IBM", "resources/Nouveau_IBM.ttf");
    load_resource<sf::Font>(rm, "Pokemon GB", "resources/Pokemon GB.ttf");
    load_resource<sf::Texture>(rm, "title", "resources/title.png");
    load_resource<sf::Texture>(rm, "frame", "resources/frame.png");

    load_resource<sf::Texture>(rm, "heart", "resources/heart.png");
    load_resource<sf::Texture>(rm, "energy", "resources/energy.png");
    load_resource<sf::Texture>(rm, "noheart", "resources/noheart.png");
    load_resource<sf::Texture>(rm, "noenergy", "resources/noenergy.png");

    load_resource<sf::Texture>(rm, "player", "resources/player.png", sf::IntRect(0, 0, 64, 64));
    load_resource<sf::Texture>(rm, "player_sw1", "resources/player.png", sf::IntRect(64, 0, 64, 64));
    load_resource<sf::Texture>(rm, "player_sw2", "resources/player.png", sf::IntRect(128, 0, 64, 64));
    load_resource<sf::Texture>(rm, "player_sa", "resources/player.png", sf::IntRect(192, 0, 64, 64));
    load_resource<sf::Texture>(rm, "root", "resources/root.png");
    load_resource<sf::Texture>(rm, "pod", "resources/pod.png");
    load_resource<sf::Texture>(rm, "vine", "resources/vine.png");

    load_resource<sf::Texture>(rm, "floor", "resources/jungle_floor.png");
    load_resource<sf::Texture>(rm, "rocks", "resources/rocks.png", sf::IntRect(0, 0, 64, 64));


    screen_manager sm;
    sm.push_screen(std::make_shared<main_menu_screen>(&window, &sm, &rm));

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
        sm.render();
        window.display();
    }

    return 0;
}
