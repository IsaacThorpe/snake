#include "random_utils.h"

static constexpr uint32_t fnv1a(const char* s, uint32_t h = 0x811C9DC5) {
    return (*s == 0) ? h : fnv1a(s + 1, (h ^ uint32_t(*s)) * 0x01000193);
}

static constexpr uint32_t compile_time_seed =
    fnv1a(__DATE__ __TIME__);

// ============================================================ //

std::mt19937& get_rng() {
    static std::mt19937 rng(std::random_device{}());
    return rng;
}

std::mt19937& get_rng_seeded() {
    static std::mt19937 rng(compile_time_seed);
    return rng;
}
