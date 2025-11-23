#include <raylib.h>

int main() {
    InitWindow(800, 600, "C + nob.h + raylib");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        Vector2 center = {
            (float)(400 - MeasureText("C + nob.h + raylib", 60)/2),
            300. - 60.
        };
        DrawText( "C + nob.h + raylib", center.x, center.y, 60, RAYWHITE);

        EndDrawing();
    }

    CloseWindow();
}
