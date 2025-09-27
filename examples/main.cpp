#include <tsuki/tsuki.hpp>
#include <iostream>

// Game state
float player_x = 400.0f;
float player_y = 300.0f;
float player_speed = 200.0f;

// Colors
tsuki::Color red_color(1.0f, 0.0f, 0.0f, 1.0f);
tsuki::Color green_color(0.0f, 1.0f, 0.0f, 1.0f);
tsuki::Color blue_color(0.0f, 0.0f, 1.0f, 1.0f);

// Game callbacks
void load() {
    std::cout << "Game loaded! Use arrow keys to move the red square." << std::endl;

    // Set window title
    tsuki::window().setTitle("Tsuki Engine Demo");
}

void update(double dt) {
    // Handle input using the convenient global functions
    if (tsuki::keyboard().isDown(tsuki::KeyCode::Left)) {
        player_x -= player_speed * dt;
    }
    if (tsuki::keyboard().isDown(tsuki::KeyCode::Right)) {
        player_x += player_speed * dt;
    }
    if (tsuki::keyboard().isDown(tsuki::KeyCode::Up)) {
        player_y -= player_speed * dt;
    }
    if (tsuki::keyboard().isDown(tsuki::KeyCode::Down)) {
        player_y += player_speed * dt;
    }

    // Keep player on screen
    int window_width = tsuki::window().getWidth();
    int window_height = tsuki::window().getHeight();

    if (player_x < 0) player_x = 0;
    if (player_x > window_width - 50) player_x = window_width - 50;
    if (player_y < 0) player_y = 0;
    if (player_y > window_height - 50) player_y = window_height - 50;

    // Exit on escape
    if (tsuki::keyboard().isDown(tsuki::KeyCode::Escape)) {
        tsuki::Engine::getInstance().quit();
    }
}

void draw() {
    // Clear screen with dark blue background
    tsuki::graphics().clear(tsuki::Color(0.1f, 0.1f, 0.3f, 1.0f));

    // Draw some shapes to demonstrate the API

    // Draw background grid
    tsuki::graphics().setColor(tsuki::Color(0.2f, 0.2f, 0.4f, 1.0f));
    for (int x = 0; x < tsuki::window().getWidth(); x += 50) {
        tsuki::graphics().line(x, 0, x, tsuki::window().getHeight());
    }
    for (int y = 0; y < tsuki::window().getHeight(); y += 50) {
        tsuki::graphics().line(0, y, tsuki::window().getWidth(), y);
    }

    // Draw some decorative circles
    tsuki::graphics().setColor(green_color);
    tsuki::graphics().circle(tsuki::DrawMode::Fill, 100, 100, 30);

    tsuki::graphics().setColor(blue_color);
    tsuki::graphics().circle(tsuki::DrawMode::Line, 700, 100, 40);

    // Draw a triangle
    tsuki::graphics().setColor(tsuki::Color(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow
    std::vector<float> triangle_points = {400, 100, 450, 200, 350, 200};
    tsuki::graphics().polygon(tsuki::DrawMode::Fill, triangle_points);

    // Draw the player (red square)
    tsuki::graphics().setColor(red_color);
    tsuki::graphics().rectangle(tsuki::DrawMode::Fill, player_x, player_y, 50, 50);

    // Draw player border
    tsuki::graphics().setColor(tsuki::Color::white());
    tsuki::graphics().rectangle(tsuki::DrawMode::Line, player_x, player_y, 50, 50);

    // Draw some text information
    // Note: Text rendering would need TTF implementation, showing print calls for API completeness
    // tsuki::graphics().print("FPS: " + std::to_string(tsuki::timer().getFPS()), 10, 10);
    // tsuki::graphics().print("Time: " + std::to_string(tsuki::timer().getTime()), 10, 30);
    // tsuki::graphics().print("Use arrow keys to move", 10, 50);
}

int main() {
    // Initialize the engine
    if (!tsuki::Engine::getInstance().init()) {
        std::cerr << "Failed to initialize Tsuki engine!" << std::endl;
        return -1;
    }

    // Set up game callbacks
    tsuki::Engine::getInstance().setLoadCallback(load);
    tsuki::Engine::getInstance().setUpdateCallback(update);
    tsuki::Engine::getInstance().setDrawCallback(draw);

    // Run the game loop
    tsuki::Engine::getInstance().run();

    std::cout << "Game finished!" << std::endl;
    return 0;
}