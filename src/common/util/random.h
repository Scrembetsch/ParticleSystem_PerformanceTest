#pragma once

#include <random>

class Random
{
public:
    Random(uint32_t seed = std::default_random_engine::default_seed);

    float Rand01();
    float Rand(float min, float max);

    template<class T>
    T RandIntegral(T min, T max)
    {
        return min + (mRng() % (max - min));
    }

private:
    std::default_random_engine mRng;
    float mMax;
};
