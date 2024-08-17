#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include <math.h>
#include <starfield_icons.h>

typedef struct {
    float x;
    float y;
    float z;
    float speed;
} Star;

#define MAX_STARS 150
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define MAX_DEPTH 32
#define TRAIL_THRESHOLD 0.8f // Top 20% of speed range

static Star stars[MAX_STARS];
static float view_offset_x = 0.0f;
static float view_offset_y = 0.0f;
static float target_offset_x = 0.0f;
static float target_offset_y = 0.0f;
static bool inverted = true;
static float speed_multiplier = 1.0f;
static const float MAX_SPEED_MULTIPLIER = 5.0f; // Increased max speed

// Timer to track button press duration
static uint32_t up_button_hold_time = 0;
static uint32_t down_button_hold_time = 0;
static const uint32_t hold_threshold = 10; // Adjust this value to change the acceleration speed

static void init_stars() {
    for(int i = 0; i < MAX_STARS; i++) {
        stars[i].x = (float)((rand() % SCREEN_WIDTH) - SCREEN_WIDTH / 2);
        stars[i].y = (float)((rand() % SCREEN_HEIGHT) - SCREEN_HEIGHT / 2);
        stars[i].z = (float)(rand() % MAX_DEPTH);
        stars[i].speed = 0.1f + ((float)rand() / RAND_MAX) * 0.3f;
    }
}

static void update_stars() {
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

static void draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    canvas_clear(canvas);

    if(inverted) {
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_box(canvas, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        canvas_set_color(canvas, ColorWhite);
    } else {
        canvas_set_color(canvas, ColorBlack);
    }

    for(int i = 0; i < MAX_STARS; i++) {
        float depth = stars[i].z / MAX_DEPTH;
        int x = (int)(((stars[i].x + view_offset_x) / stars[i].z) * 64 + SCREEN_WIDTH / 2);
        int y = (int)(((stars[i].y + view_offset_y) / stars[i].z) * 32 + SCREEN_HEIGHT / 2);

        if(x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
            if(speed_multiplier > MAX_SPEED_MULTIPLIER * TRAIL_THRESHOLD) {
                // Draw longer light trails
                int trail_length = (int)((speed_multiplier - MAX_SPEED_MULTIPLIER * TRAIL_THRESHOLD) * 20); // Increased trail length

                for(int t = 1; t <= trail_length; t++) {
                    int fade_x = (int)(((stars[i].x + view_offset_x) / (stars[i].z + stars[i].speed * speed_multiplier * t)) * 64 + SCREEN_WIDTH / 2);
                    int fade_y = (int)(((stars[i].y + view_offset_y) / (stars[i].z + stars[i].speed * speed_multiplier * t)) * 32 + SCREEN_HEIGHT / 2);

                    // Gradually reduce size to simulate fading
                    canvas_draw_dot(canvas, fade_x, fade_y);
                }
            }

            if(depth < 0.3) {
                canvas_draw_disc(canvas, x, y, 1);
            } else {
                canvas_draw_dot(canvas, x, y);
            }
        }
    }
}

static void input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

int32_t starfield_app(void* p) {
    UNUSED(p);
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, NULL);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    init_stars();

    InputEvent event;
    bool running = true;
    while(running) {
        if(furi_message_queue_get(event_queue, &event, 50) == FuriStatusOk) {
            if(event.type == InputTypePress || event.type == InputTypeRepeat) {
                if(event.key == InputKeyBack) {
                    running = false;
                } else if(event.key == InputKeyLeft) {
                    target_offset_x += 2.0f;
                } else if(event.key == InputKeyRight) {
                    target_offset_x -= 2.0f;
                } else if(event.key == InputKeyUp) {
                    up_button_hold_time++;
                    if(up_button_hold_time > hold_threshold) {
                        speed_multiplier += 0.1f * (up_button_hold_time / hold_threshold);
                    } else {
                        speed_multiplier += 0.1f;
                    }
                    if(speed_multiplier > MAX_SPEED_MULTIPLIER) {
                        speed_multiplier = MAX_SPEED_MULTIPLIER;
                    }
                } else if(event.key == InputKeyDown) {
                    down_button_hold_time++;
                    if(down_button_hold_time > hold_threshold) {
                        speed_multiplier -= 0.1f * (down_button_hold_time / hold_threshold);
                    } else {
                        speed_multiplier -= 0.1f;
                    }
                    if(speed_multiplier < 0.1f) {
                        speed_multiplier = 0.1f;
                    }
                } else if(event.key == InputKeyOk) {
                    inverted = !inverted;
                }
            } else if(event.type == InputTypeRelease) {
                if(event.key == InputKeyUp) {
                    up_button_hold_time = 0;
                } else if(event.key == InputKeyDown) {
                    down_button_hold_time = 0;
                }
            }
        }
        update_stars();
        view_port_update(view_port);
    }

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_record_close(RECORD_GUI);

    return 0;
}
