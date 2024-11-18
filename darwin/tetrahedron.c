#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>

#define WIDTH 128
#define HEIGHT 64
#define PI 3.14159265358979323846

typedef struct {
    float x, y, z;
} Point3D;

typedef struct {
    int x, y;
} Point2D;

// Function to project 3D points to 2D
Point2D project(Point3D p, float fov, float viewer_distance) {
    Point2D projected;
    projected.x = (int)(p.x * fov / (viewer_distance + p.z)) + WIDTH / 2;
    projected.y = (int)(p.y * fov / (viewer_distance + p.z)) + HEIGHT / 2;
    return projected;
}

// Function to rotate points around the X-axis
Point3D rotateX(Point3D p, float angle) {
    Point3D rotated;
    rotated.x = p.x;
    rotated.y = p.y * cos(angle) - p.z * sin(angle);
    rotated.z = p.y * sin(angle) + p.z * cos(angle);
    return rotated;
}

// Function to rotate points around the Y-axis
Point3D rotateY(Point3D p, float angle) {
    Point3D rotated;
    rotated.x = p.x * cos(angle) + p.z * sin(angle);
    rotated.y = p.y;
    rotated.z = -p.x * sin(angle) + p.z * cos(angle);
    return rotated;
}

// Function to rotate points around the Z-axis
Point3D rotateZ(Point3D p, float angle) {
    Point3D rotated;
    rotated.x = p.x * cos(angle) - p.y * sin(angle);
    rotated.y = p.x * sin(angle) + p.y * cos(angle);
    rotated.z = p.z;
    return rotated;
}

// Function to draw a line on the screen
void drawLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2) {
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

int main(int argc, char *argv[]) {
    // Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("Rotating Tetrahedron",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Define the vertices of a tetrahedron
    Point3D vertices[4] = {
        { 1, 1, 1 },
        { -1, -1, 1 },
        { -1, 1, -1 },
        { 1, -1, -1 }
    };

    // Define the edges between the vertices
    int edges[6][2] = {
        {0, 1}, {0, 2}, {0, 3},
        {1, 2}, {1, 3}, {2, 3}
    };

    float angleX = 0.0f;
    float angleY = 0.0f;
    float angleZ = 0.0f;

    float fov = 128; // Adjust field of view to fit within Flipper Zero dimensions
    float viewer_distance = 3; // Adjusted for better fit

    int running = 1;
    SDL_Event e;

    // Main loop
    while (running) {
        // Handle events
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
        }

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Rotate and project the vertices
        Point2D projected_vertices[4];
        for (int i = 0; i < 4; i++) {
            Point3D rotated = rotateX(vertices[i], angleX);
            rotated = rotateY(rotated, angleY);
            rotated = rotateZ(rotated, angleZ);

            projected_vertices[i] = project(rotated, fov, viewer_distance);
        }

        // Draw the edges
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int i = 0; i < 6; i++) {
            drawLine(renderer,
                     projected_vertices[edges[i][0]].x, projected_vertices[edges[i][0]].y,
                     projected_vertices[edges[i][1]].x, projected_vertices[edges[i][1]].y);
        }

        // Update rotation angles (slowed down)
        angleX += 0.01f;
        angleY += 0.015f;
        angleZ += 0.02f;

        // Update the screen
        SDL_RenderPresent(renderer);

        // Delay to control the frame rate
        SDL_Delay(33); // ~30 FPS
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

