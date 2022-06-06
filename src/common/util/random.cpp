#include "random.h"

Random::Random()
    : mRng(std::default_random_engine::default_seed)
    , mMax(static_cast<float>(mRng.max()))
{
}

float Random::Rand01()
{
    return static_cast<float>(mRng()) / mMax;
}

float Random::Rand(float min, float max)
{
    float rand = Rand01();
    return min + (max - min) * rand;
}