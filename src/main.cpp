#include <SFML/Graphics.hpp>
#include <deque>
#include <iostream>

#include "globals.h"
#include "random_utils.h"
#include "snakectl.h"
#include "highscore.h"

struct Snake {
    Direction direction;
    u8 grow_timer;
    std::deque<std::array<u16, 2>> deque; // Keeps track of head and tail with O(1) insertion + deletion on front and back
};

// Set a square (6 vertices) to a color
void set_square_vertices_color(sf::Vertex* vertices, sf::Color color);

// Set the positions of each vertex (2 triangles, 6 vertices) to make a square
void set_square_vertices_positions(sf::Vertex* vertices, u16 x, u16 y);

// Initialize all triangle positions and colors (colored according to tiles array values)
void generate_vertex_buffer(sf::VertexBuffer& vertex_buffer, u8 tiles[HEIGHT][WIDTH]);

// Overwrites a single square in the vertex_buffer to change it's color
void update_vertex_buffer_square_color(sf::VertexBuffer& vertex_buffer, u16 x, u16 y, sf::Color color);

// Sets new apple position and draws it to vertex_buffer
// Does not remove previous apple vertices. This should not be necessary because snake head should overwrite it
void update_apple(std::array<u16, 2>& apple_position, sf::VertexBuffer& vertex_buffer, u8 tiles[HEIGHT][WIDTH]);

void game_over(bool highscore_viable, u32 snake_length, bool* died_bool = nullptr) {
    if (died_bool != nullptr)
        *died_bool = true;

    if (highscore_viable && snake_length > get_highscore()) {
        save_highscore(snake_length);
    }
}

void end_program(sf::RenderWindow& window, bool display_results, bool highscore_viable, u32 snake_length) {
    game_over(highscore_viable, snake_length);
    window.close();
}

int main(int argc, char* argv[])
{
    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Snake", sf::State::Windowed);
    // Default. Will be overwritten if -f argument given
    window.setFramerateLimit(UPDATES_PER_SECOND);
    window.setVerticalSyncEnabled(false);
    bool highscore_viable = true;

    // Argument handling
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "-h" || arg == "highscore" || arg == "h") {
            std::cout << "The current record length is " << get_highscore() << std::endl;
            return 0;
        }
        else if (arg == "-f") {
            i++;
            try {
                u32 framerate_arg = std::stoi(argv[i]);
                window.setFramerateLimit(framerate_arg);
                if (framerate_arg > 20) {
                    highscore_viable = false;
                    std::cout << "Framerate is greater than 20. Highscore will not be updated." << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid framerate value provided for -f." << std::endl;
            }
        }
    }

    bool replay = true;
    while (replay) {
        // Initialize tiles (for collision detection and initializing vertex_buffer)
        u8 tiles[HEIGHT][WIDTH] = {0};
        for (const auto& p : START_COORDS) {
            tiles[p[1]][p[0]] = 1;
        }

        // Initialize snake data
        Snake snake;
        snake.direction = START_DIRECTION;
        snake.grow_timer = START_GROW_TIMER;
        for (const auto& p : START_COORDS) {
            snake.deque.push_back(p);
        }

        // Initialize vertex_buffer
        sf::VertexBuffer vertex_buffer(sf::PrimitiveType::Triangles, sf::VertexBuffer::Usage::Stream);
        generate_vertex_buffer(vertex_buffer, tiles); 

        // Apple
        std::array<u16, 2> apple_position;
        update_apple(apple_position, vertex_buffer, tiles);

        bool died = false;
        while (window.isOpen() && !died)
        {
            while (const std::optional event = window.pollEvent())
            {
                if (event->is<sf::Event::Closed>())
                    end_program(window, false, highscore_viable, static_cast<u32>(snake.deque.size()));

                else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                    switch (keyPressed->scancode) {
                        case sf::Keyboard::Scancode::Escape:
                        case sf::Keyboard::Scancode::Q:
                        case sf::Keyboard::Scancode::C:
                            end_program(window, true, highscore_viable, static_cast<u32>(snake.deque.size()));
                            break;
                        default:
                            break;
                    }
                }
            }

            // ======================== Game logic ======================== //

            // snake.direction = snake_decide(apple_position, snake.deque.back(), snake.direction, tiles);
            snake.direction = snake_decide(apple_position, snake.deque.back(), snake.direction, tiles);

            // Get the next head position and check if out of bounds
            std::array<u16, 2> next_head_position(snake.deque.back());

            switch (snake.direction) {
                case UP:    next_head_position[1]++; break;
                case DOWN:  next_head_position[1]--; break;
                case RIGHT: next_head_position[0]++; break;
                case LEFT:  next_head_position[0]--; break;
            }
            if (next_head_position[0] >= WIDTH || next_head_position[1] >= HEIGHT)
                game_over(highscore_viable, static_cast<u32>(snake.deque.size()), &died);
            // Would check for collision here, but tail must be popped first just in case head ends up at the previous tail position 

            // Tail 
            // Check if growing. If not, pop front (tail) of snake and update vertex_buffer
            if (snake.grow_timer != 0) 
                snake.grow_timer--;
            else {
                update_vertex_buffer_square_color(vertex_buffer, snake.deque.front()[0], snake.deque.front()[1], sf::Color::Black);
                tiles[snake.deque.front()[1]][snake.deque.front()[0]] = 0;
                snake.deque.pop_front();
            }

            // Head
            // Check for collisions. If none, push the next head position and update vertex_buffer
            if (tiles[next_head_position[1]][next_head_position[0]] == 1)
                game_over(highscore_viable, static_cast<u32>(snake.deque.size()), &died);
            else {
                update_vertex_buffer_square_color(vertex_buffer, next_head_position[0], next_head_position[1], sf::Color::White);
                snake.deque.push_back(next_head_position);
                tiles[next_head_position[1]][next_head_position[0]] = 1;
            }

            // Apple!
            if (snake.deque.back() == apple_position) {
                snake.grow_timer += GROW_RATE;
                update_apple(apple_position, vertex_buffer, tiles);
            }

            // ======================== Rendering ======================== //

            // clear the window with black color
            window.clear(sf::Color::Black);

            // draw everything here...
            window.draw(vertex_buffer);

            // end the current frame
            window.display();
        }
    }
}

void set_square_vertices_color(sf::Vertex* vertices, sf::Color color) {
    for (u8 i = 0; i < 6; ++i) {
        vertices[i].color = color;
    }
}

void set_square_vertices_positions(sf::Vertex* vertices, u16 x, u16 y) {
    float screen_x = x * TILE_LENGTH + OFFSET;
    float screen_y = y * TILE_LENGTH + OFFSET;

    // Four corners of the square
    sf::Vector2f TL = { screen_x, screen_y };
    sf::Vector2f TR = { screen_x + SQUARE_LENGTH, screen_y };
    sf::Vector2f BL = { screen_x, screen_y + SQUARE_LENGTH };
    sf::Vector2f BR = { screen_x + SQUARE_LENGTH, screen_y + SQUARE_LENGTH };

    vertices[0].position = TL;
    vertices[1].position = BL;
    vertices[2].position = BR;

    vertices[3].position = TL;
    vertices[4].position = BR;
    vertices[5].position = TR;
}

void generate_vertex_buffer(sf::VertexBuffer& vertex_buffer, u8 tiles[HEIGHT][WIDTH]) {
    sf::VertexArray vertices(sf::PrimitiveType::Triangles, WIDTH * HEIGHT * 6);

    for (usize i = 0; i < WIDTH * HEIGHT; ++i) {
        u16 x = i % WIDTH;
        u16 y = i / WIDTH;

        set_square_vertices_positions(&vertices[i * 6], x, y);

        if (tiles[y][x] == 1) 
            set_square_vertices_color(&vertices[i * 6], sf::Color::White);
        else 
            set_square_vertices_color(&vertices[i * 6], sf::Color::Black);
    }

    // Create and copy vertices to vertex_buffer
    if (!vertex_buffer.create(vertices.getVertexCount()))
        throw(std::runtime_error("Failed to create vertex_buffer of size " + std::to_string(vertices.getVertexCount())));

    if (!vertex_buffer.update(&vertices[0]))
        throw(std::runtime_error("Failed to copy initial vertex data to vertex_buffer"));
}

void update_vertex_buffer_square_color(sf::VertexBuffer& vertex_buffer, u16 x, u16 y, sf::Color color) {
    sf::Vertex vertices[6];
    set_square_vertices_positions(vertices, x, y);
    set_square_vertices_color(vertices, color);

    if (!vertex_buffer.update(vertices, 6, (y * WIDTH + x) * 6)) 
        throw(std::runtime_error("Failed to update vertex_buffer"));
}

void update_apple(std::array<u16, 2>& apple_position, sf::VertexBuffer& vertex_buffer, u8 tiles[HEIGHT][WIDTH]) {
    // TODO: Ensure new apple position cannot be set to a snake tile
    do {
        apple_position[0] = random_int(0, WIDTH - 1);
        apple_position[1] = random_int(0, HEIGHT - 1);
    } while (tiles[apple_position[1]][apple_position[0]] == 1);
    update_vertex_buffer_square_color(vertex_buffer, apple_position[0], apple_position[1], sf::Color::Red);
}
