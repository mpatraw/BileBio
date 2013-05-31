
#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <ctime>
#include <random>

class rng
{
public:
    rng(size_t seed=0)
        : gen_(rd_())
    {
        set_seed(seed);
    }

    size_t get_seed() const { return seed_; }
    void set_seed(size_t seed)
    {
        if (!seed)
            seed = std::time(nullptr);
        gen_.seed(seed);
    }

    double get_uniform()
    {
        std::uniform_real_distribution<double> dis(0, 1);
        return dis(gen_);
    }

    ssize_t get_range(ssize_t min, ssize_t max)
    {
        std::uniform_int_distribution<ssize_t> dis(min, max);
        return dis(gen_);
    }

    template <class I>
    void shuffle(I first, I last) { std::shuffle(first, last, gen_); }
private:
    size_t seed_;
    std::random_device rd_;
    std::mt19937_64 gen_;
};

#endif
