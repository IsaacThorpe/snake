#pragma once
#include <array>
#include <stdint.h>

// Rust's type names are so much better
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using usize = std::size_t;

enum Direction {
    UP = 0,
    DOWN,
    LEFT,
    RIGHT
};

// ========== GLOBAL VARIABLES ========== //

// Window
constexpr u16 WINDOW_WIDTH = 1600;
constexpr u16 WINDOW_HEIGHT = 900;
constexpr u16 UPDATES_PER_SECOND = 3;

// Game Tiles
constexpr u16 TILE_LENGTH = 30; // Should evenly divide the WINDOW_WIDTH and WINDOW_HEIGHT
constexpr u16 SQUARE_LENGTH = 30; // For a smaller square per the tile

constexpr u16 WIDTH = WINDOW_WIDTH / TILE_LENGTH; // Number of tile columns
constexpr u16 HEIGHT = WINDOW_HEIGHT / TILE_LENGTH; // Number of tile rows
constexpr u16 OFFSET = (TILE_LENGTH - SQUARE_LENGTH ) / 2;

// Snake
constexpr u32 START_LENGTH = 1;
constexpr Direction START_DIRECTION = RIGHT;
constexpr u8 START_GROW_TIMER = 2;
constexpr u16 GROW_RATE = 3;
constexpr std::array<u16, 2> START_COORDS[START_LENGTH] = {{WIDTH / 2, HEIGHT / 2}};

// ====================================== //
