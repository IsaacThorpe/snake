#pragma once
#include <random>

extern std::mt19937& get_rng();
extern std::mt19937& get_rng_seeded();

template<typename T>
T random_int(T min, T max) {
    std::uniform_int_distribution<T> dist(min, max);
    return dist(get_rng());
}

template<typename T>
T random_int_compile_time_seeded(T min, T max) {
    std::uniform_int_distribution<T> dist(min, max);
    return dist(get_rng_seeded());
}

