#pragma once
#include <array>
#include <SFML/Graphics.hpp>

#include "globals.h"
#include "random_utils.h"

Direction player_decide(Direction current_direction);

Direction snake_decide(std::array<u16, 2> apple_position, std::array<u16, 2> current_head_position, Direction current_direction, u8 tiles[HEIGHT][WIDTH]);
