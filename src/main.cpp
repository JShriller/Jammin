#include "raylib.h"

int main() {
    // 1. Initialize the window
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(60); // Set game to run at 60 frames-per-second

    // 2. Main game loop
    while (!WindowShouldClose()) { // Detect window close button or ESC key
        // Update variables / game logic here
        
        // 3. Render logic
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Congrats! You created your first raylib window!", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }

    // 4. De-Initialization
    CloseWindow(); // Close window and OpenGL context

    return 0;
}
