#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <string.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCALE_FACTOR 4
#define MAX_STARS 150
#define MAX_DEPTH 32
#define TRAIL_THRESHOLD 0.8f
#define MAX_SPEED_MULTIPLIER 5.0f

// Flipper Zero memory constraints
#define FLIPPER_RAM_SIZE (20 * 1024)  // 20 KB of RAM
#define FLIPPER_STACK_SIZE (4 * 1024)  // 4 KB stack

typedef struct {
    float x;
    float y;
    float z;
    float speed;
} Star;

// Simulated Flipper Zero RAM
static uint8_t flipper_ram[FLIPPER_RAM_SIZE];
static size_t flipper_ram_used = 0;

Star stars[MAX_STARS];
float view_offset_x = 0.0f;
float view_offset_y = 0.0f;
float target_offset_x = 0.0f;
float target_offset_y = 0.0f;
bool inverted = true;
float speed_multiplier = 1.0f;
int scale_factor = 4;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

// Simulated heap allocation
void* flipper_malloc(size_t size) {
    if (flipper_ram_used + size > FLIPPER_RAM_SIZE) {
        SDL_Log("Memory allocation failed: Out of memory");
        return NULL;
    }
    void* ptr = &flipper_ram[flipper_ram_used];
    flipper_ram_used += size;
    return ptr;
}

void flipper_free(void* ptr) {
    // In this simple simulation, we don't actually free memory
    // In a more complex simulation, you could implement proper memory management
}

void init_sdl() {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Flipper Zero Starfield Simulator",
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH * scale_factor, SCREEN_HEIGHT * scale_factor, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetScale(renderer, scale_factor, scale_factor);
}


void cleanup_sdl() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void init_stars() {
    srand(time(NULL));
    for(int i = 0; i < MAX_STARS; i++) {
        Star* star = (Star*)flipper_malloc(sizeof(Star));
        if (star == NULL) {
            SDL_Log("Failed to allocate memory for star");
            return;
        }
        star->x = (float)((rand() % SCREEN_WIDTH) - SCREEN_WIDTH / 2);
        star->y = (float)((rand() % SCREEN_HEIGHT) - SCREEN_HEIGHT / 2);
        star->z = (float)(rand() % MAX_DEPTH);
        star->speed = 0.1f + ((float)rand() / RAND_MAX) * 0.3f;
        memcpy(&stars[i], star, sizeof(Star));
        flipper_free(star);
    }
}

void update_stars() {
    for(int i = 0; i < MAX_STARS; i++) {
        stars[i].z -= stars[i].speed * speed_multiplier;
        if(stars[i].z <= 0) {
            stars[i].x = (float)((rand() % SCREEN_WIDTH) - SCREEN_WIDTH / 2);
            stars[i].y = (float)((rand() % SCREEN_HEIGHT) - SCREEN_HEIGHT / 2);
            stars[i].z = (float)MAX_DEPTH;
        }
    }

    view_offset_x += (target_offset_x - view_offset_x) * 0.1f;
    view_offset_y += (target_offset_y - view_offset_y) * 0.1f;
}

void draw_stars() {
    SDL_SetRenderDrawColor(renderer, inverted ? 255 : 0, inverted ? 255 : 0, inverted ? 255 : 0, 255);
    SDL_RenderClear(renderer);
    
    SDL_SetRenderDrawColor(renderer, inverted ? 0 : 255, inverted ? 0 : 255, inverted ? 0 : 255, 255);

    for(int i = 0; i < MAX_STARS; i++) {
        float depth = stars[i].z / MAX_DEPTH;
        int x = (int)(((stars[i].x + view_offset_x) / stars[i].z) * 64 + SCREEN_WIDTH / 2);
        int y = (int)(((stars[i].y + view_offset_y) / stars[i].z) * 32 + SCREEN_HEIGHT / 2);

        if(x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
            if(speed_multiplier > MAX_SPEED_MULTIPLIER * TRAIL_THRESHOLD) {
                int trail_length = (int)((speed_multiplier - MAX_SPEED_MULTIPLIER * TRAIL_THRESHOLD) * 20);

                for(int t = 1; t <= trail_length; t++) {
                    int fade_x = (int)(((stars[i].x + view_offset_x) / (stars[i].z + stars[i].speed * speed_multiplier * t)) * 64 + SCREEN_WIDTH / 2);
                    int fade_y = (int)(((stars[i].y + view_offset_y) / (stars[i].z + stars[i].speed * speed_multiplier * t)) * 32 + SCREEN_HEIGHT / 2);

                    SDL_RenderDrawPoint(renderer, fade_x, fade_y);
                }
            }

            if(depth < 0.3) {
                SDL_Rect rect = {x - 1, y - 1, 3, 3};
                SDL_RenderFillRect(renderer, &rect);
            } else {
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }

    SDL_RenderPresent(renderer);
}

void update_scale_factor(int new_scale) {
    scale_factor = new_scale;  // Update the scale factor
    SDL_SetWindowSize(window, SCREEN_WIDTH * scale_factor, SCREEN_HEIGHT * scale_factor);
    SDL_RenderSetScale(renderer, scale_factor, scale_factor);
}

int main(int argc, char* argv[]) {
    // Simulate stack size limitation
    char stack_usage[FLIPPER_STACK_SIZE];
    memset(stack_usage, 0, FLIPPER_STACK_SIZE);

    init_sdl();
    init_stars();

    bool running = true;
    SDL_Event event;

    while(running) {
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                running = false;
            } else if(event.type == SDL_KEYDOWN) {
                switch(event.key.keysym.sym) {
                    case SDLK_ESCAPE: running = false; break;
                    case SDLK_LEFT: target_offset_x += 2.0f; break;
                    case SDLK_RIGHT: target_offset_x -= 2.0f; break;
                    case SDLK_UP:
                        speed_multiplier += 0.1f;
                        if(speed_multiplier > MAX_SPEED_MULTIPLIER) speed_multiplier = MAX_SPEED_MULTIPLIER;
                        break;
                    case SDLK_DOWN:
                        speed_multiplier -= 0.1f;
                        if(speed_multiplier < 0.1f) speed_multiplier = 0.1f;
                        break;
                    case SDLK_RETURN: inverted = !inverted; break;

                    // Handle scale factor adjustments
                    case SDLK_1: update_scale_factor(1); break;
                    case SDLK_2: update_scale_factor(2); break;
                    case SDLK_3: update_scale_factor(3); break;
                    case SDLK_4: update_scale_factor(4); break;
                }
            }
        }

        update_stars();
        draw_stars();

        SDL_Delay(16);  // Approx. 60 FPS
    }

    cleanup_sdl();

    SDL_Log("Total RAM used: %zu bytes", flipper_ram_used);
    return 0;
}