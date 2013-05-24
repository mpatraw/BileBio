
#ifndef SCREEN_MANAGER_HPP
#define SCREEN_MANAGER_HPP

#include <vector>

#include <boost/noncopyable.hpp>

#include "resource_manager.hpp"

class screen_manager;

class screen
{
public:
    virtual bool stops_events() const = 0;
    virtual bool stops_updating() const = 0;
    virtual bool stops_rendering() const = 0;
    virtual void on_enter() = 0;
    virtual void on_exit() = 0;
    virtual void on_event(const sf::Event &event) = 0;
    virtual void on_update(double dt) = 0;
    virtual void on_render() = 0;
};

class screen_manager : private boost::noncopyable
{
public:
    screen_manager() { }
    ~screen_manager() { }

    size_t number_of_screens() const { return screen_stack_.size(); }

    void push_screen(std::shared_ptr<screen> scr)
    {
        screen_stack_.push_back(scr);
        scr->on_enter();
    }
    void pop_screen()
    {
        auto scr = std::move(screen_stack_.back());
        screen_stack_.pop_back();
        scr->on_exit();
    }

    void on_event(const sf::Event &event)
    {
        ssize_t top = (ssize_t)screen_stack_.size() - 1;
        for (; top >= 0; --top)
            if (screen_stack_[top]->stops_events())
                break;
        if (top < 0)
            top = 0;
        for (; top < (ssize_t)screen_stack_.size(); ++top)
            screen_stack_[top]->on_event(event);
    }

    void update(double dt)
    {
        ssize_t top = (ssize_t)screen_stack_.size() - 1;
        for (; top >= 0; --top)
            if (screen_stack_[top]->stops_updating())
                break;
        if (top < 0)
            top = 0;
        for (; top < (ssize_t)screen_stack_.size(); ++top)
            screen_stack_[top]->on_update(dt);
    }
    void render()
    {
        ssize_t top = (ssize_t)screen_stack_.size() - 1;
        for (; top >= 0; --top)
            if (screen_stack_[top]->stops_rendering())
                break;
        if (top < 0)
            top = 0;
        for (; top < (ssize_t)screen_stack_.size(); ++top)
            screen_stack_[top]->on_render();
    }

private:
    std::vector<std::shared_ptr<screen>> screen_stack_;
};

#endif
