#include "snakectl.h"

Direction snake_decide(std::array<u16, 2> apple_position, std::array<u16, 2> current_head_position, Direction current_direction, u8 tiles[HEIGHT][WIDTH]) {
    // I'm calling this Blind Snake Algorithm
    // It sees by smell...

    const u8 laziness = 2; // Tendency to stick to the same direction as previously when choosing between directions of equal precedence
    const u8 max_precedence = 3;
    u8 dir_precedence[4] = {2, 2, 2, 2};
    // Indexed by Direction enum
    // 3 is ideal
    // 2 is neutral
    // 1 avoid if possible
    // 0 avoid at all costs

    // ------------------------------ General Ranking -------------------------------- //

    // Get ideal X direction

    bool moving_incorrect_y = (current_head_position[1] < apple_position[1] && current_direction == UP) ||
        (current_head_position[1] > apple_position[1] && current_direction == DOWN);
    bool moving_incorrect_x = (current_head_position[0] < apple_position[0] && current_direction == LEFT) ||
        (current_head_position[0] > apple_position[0] && current_direction == RIGHT);

    if (current_head_position[0] < apple_position[0])
        dir_precedence[RIGHT] = 3;
    else if (current_head_position[0] > apple_position[0])
        dir_precedence[LEFT] = 3;
    else if (!moving_incorrect_y) {
        // Aligned with apple. Moving is counter-productive
        dir_precedence[LEFT] = 1;
        dir_precedence[RIGHT] = 1;
    }

    // Get ideal Y direction
    if (current_head_position[1] < apple_position[1])
        dir_precedence[UP] = 3;
    else if (current_head_position[1] > apple_position[1])
        dir_precedence[DOWN] = 3;
    else if (!moving_incorrect_x) {
        // Aligned with apple. Moving is counter-productive
        dir_precedence[UP] = 1;
        dir_precedence[DOWN] = 1;
    }

    // ------------------------------ Invalidate any Obstructed Directions -------------------------------- //

    auto is_obstructed = [&current_head_position, tiles](Direction dir) {
        std::array<u16, 2> next_position(current_head_position);
        switch (dir) {
            case RIGHT:
                next_position[0]++;
                break;
            case LEFT:
                next_position[0]--;
                break;
            case UP:
                next_position[1]++;
                break;
            case DOWN:
                next_position[1]--;
                break;
        }
        // Short-circuit bounds check and tile check
        if (next_position[0] >= WIDTH || 
            next_position[1] >= HEIGHT ||
            tiles[next_position[1]][next_position[0]] == 1)
        {
            return true;
        }
        return false;
    };

    for (u8 i = 0; i < 4; i++) {
        // Check if any direction is obstructed (This also prevents doing 180 turns)
        if (is_obstructed(static_cast<Direction>(i))) 
            dir_precedence[i] = 0;
    }

    // ------------------------------ Pick Random Best Precedence -------------------------------- //

    // More likely to keep current_direction
    if (dir_precedence[current_direction] == 3 && random_int(0,1) == 0)
        return current_direction;

    auto pick_random = [&current_direction, &laziness](u8 dir_precedence[4], u8 precedence) -> std::optional<Direction> {
        u8 choices[4];
        u8 choices_len = 0;

        for (u8 i = 0; i < 4; ++i) {
            if (dir_precedence[i] == precedence) {
                choices[choices_len++] = i;
                if (static_cast<Direction>(i) == current_direction &&
                    random_int(u8(0), laziness) > 0)
                {
                    return std::optional(current_direction);
                }
            }
        }

        if (choices_len == 0) {
            return std::nullopt;
        }

        u8 choice_idx = random_int(u8(0), u8(choices_len - 1));

        return std::optional(static_cast<Direction>(choices[choice_idx]));
    };

    Direction chosen_dir = UP; // To heaven

    for (u8 p = max_precedence; p >= 0; --p) {
        std::optional<Direction> dir = pick_random(dir_precedence, p);
        if (dir.has_value()) {
            chosen_dir = dir.value();
            break;
        }
    }

    return chosen_dir;
}

// --------------------------------------------------------------------------------------------------- // 

// Player input
Direction player_decide(Direction current_direction) {
    Direction dir; 

    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::K)) && current_direction != UP) {
        dir = DOWN;
    }
    else if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J)) && current_direction != DOWN) {
        dir = UP;
    }
    else if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::L)) && current_direction != LEFT) {
        dir = RIGHT;
    }
    else if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::H)) && current_direction != RIGHT) {
        dir = LEFT;
    }
    else {
        dir = current_direction;
    }

    return dir;
}
