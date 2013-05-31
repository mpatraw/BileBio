
#ifndef ANIMATION_MANAGER_HPP
#define ANIMATION_MANAGER_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include <boost/noncopyable.hpp>

class animation
{
public:
    animation(double duration=0.0, bool loop=false) :
        duration_(duration),
        loops_(loop)
    {
        current_frame_ = -1;
        paused_ = true;
        done_ = true;
        time_accumulator_ = 0.0;
    }
    animation(const animation &anim) = default;
    animation(animation &&anim) = default;
    animation &operator=(const animation &anim) = default;
    animation &operator=(animation &&anim) = default;

    void set_loops(bool loops=true) { loops_ = loops; }
    void set_duration(double d) { duration_ = d; }

    void start() { paused_ = false; current_frame_ = 0; done_ = false; time_accumulator_ = 0.0; }
    void pause() { paused_ = true; }
    void resume() { paused_ = false; }
    void stop() { paused_ = true; current_frame_ = 0; done_ = true; time_accumulator_ = 0.0; }

    bool is_done() const { return done_; }

    void clear_frames()
    {
        current_frame_ = -1;
        frames_.clear();
    }

    void add_frame(const std::string &str)
    {
        frames_.push_back(std::move(str));
    }

    const std::vector<std::string> &get_frames() const { return frames_; }

    void update(double dt)
    {
        if (!paused_ && (!done_ || loops_))
        {
            time_accumulator_ += dt;
            if (time_accumulator_ >= duration_ / frames_.size() * (current_frame_ + 1))
            {
                if (current_frame_ + 1 >= (ssize_t)frames_.size())
                {
                    done_ = true;
                    if (loops_)
                        current_frame_ = 0;
                    time_accumulator_ -= duration_;
                }
                else
                {
                    ++current_frame_;
                }
            }
        }
    }

    const std::string &get_texture() const
    {
        return frames_[current_frame_];
    }

private:
    double duration_;
    bool loops_;
    std::vector<std::string> frames_;
    ssize_t current_frame_;
    bool paused_;
    bool done_;
    double time_accumulator_;
};

class state_animator : private boost::noncopyable
{
public:
    state_animator() : current_state_("") { }
    state_animator(const state_animator &sm) = default;
    state_animator(state_animator &&sm) = default;
    state_animator &operator=(const state_animator &sm) = default;
    state_animator &operator=(state_animator &&sm) = default;

    void set_state(std::string state)
    {
        current_state_ = state;
        animations_[current_state_].stop();
    }
    const std::string &get_state() const { return current_state_; }
    const animation &get_animation() const { return animations_.at(current_state_); }
    animation &get_animation() { return animations_[current_state_]; }


private:
    std::unordered_map<std::string, animation> animations_;
    std::string current_state_;
};

#endif
