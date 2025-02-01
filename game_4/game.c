#include <stdio.h>
#include <raylib.h>
#include <math.h> // Include math.h for fabs()
#include <time.h> // Include time.h for time()
#include <stdlib.h> // Include stdlib.h for rand() and srand()
#include <stdbool.h>
#include <string.h>

#define scale_factor 1.5
#define BRIGHT_RED (Color){ 255, 0, 0, 255 }

// Arrow Parameters
typedef struct {
    int x;
    int y;
    int width;
    int height;
    int speed;
    Texture2D texture;
} Arrow;

// Sword Parameters
typedef struct {
    int x;
    int y;
    int width;
    int height;
    int speed;
    Texture2D texture;
} Sword;

// Character Parameters
typedef struct {
    int x;
    int y;
    int width;
    int height_stand;
    int height_crouch;
    int velocity;
    int direction;
    int speed;
    bool jumping;
    bool walking;
    bool crouching; // Add crouching state
} Character;

// Platforms Differ
typedef enum {
    PLATFORM,
    FLOOR,
} PlatformType;

// Platform Parameters
typedef struct {
    int x;
    int y;
    int width;
    int height;
    PlatformType type;
} Platform;

typedef struct {
    Arrow *arrows;
    int count;
    int capacity;
} ArrowList;

typedef struct {
    Sword *swords;
    int count;
    int capacity;
} SwordList;

float rand_float() {
    return (float)((float)rand() / RAND_MAX);
}

// Arrow Creation Function
Arrow create_arrow(Texture2D arrow_texture, int x, int floor_y) {
    Arrow arrow;
    arrow.width = arrow_texture.width;
    arrow.height = arrow_texture.height;
    arrow.x = x; // Set the x position
    arrow.y = rand() % (floor_y - arrow.height); // Random y position above the floor
    arrow.speed = 5; // Speed of the arrow
    arrow.texture = arrow_texture;
    return arrow;
}

// Sword Creation Function
Sword create_sword(Texture2D sword_texture, int x, int floor_y) {
    Sword sword;
    sword.width = sword_texture.width;
    sword.height = sword_texture.height;
    sword.x = x; // Set the x position
    sword.y = rand() % (floor_y - sword.height); // Random y position above the floor
    sword.speed = 7; // Higher speed than the arrow
    sword.texture = sword_texture;
    return sword;
}

// Moving Arrow
void update_arrow(Arrow *arrow, int width, int camera_offset_x, int score) {
    arrow->x -= arrow->speed + score / 10; // Increase speed based on score
    if (arrow->x < camera_offset_x) {
        arrow->x = width + camera_offset_x; // Reset to the right edge if it goes off-screen
        arrow->y = rand() % (width - arrow->height); // Random y position
    }
}

// Moving Sword
void update_sword(Sword *sword, int width, int camera_offset_x, int score) {
    sword->x -= sword->speed + score / 10; // Increase speed based on score
    if (sword->x < camera_offset_x) {
        sword->x = width + camera_offset_x; // Reset to the right edge if it goes off-screen
        sword->y = rand() % (width - sword->height); // Random y position
    }
}

void init_arrow_list(ArrowList *arrow_list, int initial_capacity) {
    arrow_list->arrows = (Arrow *)malloc(initial_capacity * sizeof(Arrow));
    arrow_list->count = 0;
    arrow_list->capacity = initial_capacity;
}

void add_arrow(ArrowList *arrow_list, Arrow arrow) {
    if (arrow_list->count >= arrow_list->capacity) {
        arrow_list->capacity *= 2;
        arrow_list->arrows = (Arrow *)realloc(arrow_list->arrows, arrow_list->capacity * sizeof(Arrow));
    }
    arrow_list->arrows[arrow_list->count++] = arrow;
}

void update_arrows(ArrowList *arrow_list, int width, int camera_offset_x, int score) {
    for (int i = 0; i < arrow_list->count; i++) {
        update_arrow(&arrow_list->arrows[i], width, camera_offset_x, score);
    }
}

void draw_arrows(ArrowList *arrow_list) {
    for (int i = 0; i < arrow_list->count; i++) {
        DrawTexture(arrow_list->arrows[i].texture, arrow_list->arrows[i].x, arrow_list->arrows[i].y, WHITE);
    }
}

void init_sword_list(SwordList *sword_list, int initial_capacity) {
    sword_list->swords = (Sword *)malloc(initial_capacity * sizeof(Sword));
    sword_list->count = 0;
    sword_list->capacity = initial_capacity;
}

void add_sword(SwordList *sword_list, Sword sword) {
    if (sword_list->count >= sword_list->capacity) {
        sword_list->capacity *= 2;
        sword_list->swords = (Sword *)realloc(sword_list->swords, sword_list->capacity * sizeof(Sword));
    }
    sword_list->swords[sword_list->count++] = sword;
}

void update_swords(SwordList *sword_list, int width, int camera_offset_x, int score) {
    for (int i = 0; i < sword_list->count; i++) {
        update_sword(&sword_list->swords[i], width, camera_offset_x, score);
    }
}

void draw_swords(SwordList *sword_list) {
    for (int i = 0; i < sword_list->count; i++) {
        DrawTexture(sword_list->swords[i].texture, sword_list->swords[i].x, sword_list->swords[i].y, WHITE);
    }
}

// Function to check if the character is colliding with any platform
int character_on_platform(Character character, Platform platforms[], int platform_count) {
    for (int i = 0; i < platform_count; i++) {
        Platform platform = platforms[i];
        Rectangle platform_rec = {.x = platform.x, .y = platform.y, .width = platform.width, .height = platform.height};
        Rectangle character_rec = {.x = character.x + 10, .y = character.y + character.height_stand - character.height_stand * 0.2,
                                   .width = character.width - 15, .height = character.height_stand* 0.2 + 1};

        // Check if the character is colliding with the platform
        if (CheckCollisionRecs(character_rec, platform_rec)) {
            return i;
        }
    }
    return -1;
}

bool check_character_collision(Character character, ArrowList *arrow_list, SwordList *sword_list, bool crouching) {
    Rectangle character_rect;
    if (crouching) {
        character_rect = (Rectangle){
            .x = character.x + character.width * 0.2f,
            .y = character.y + character.height_stand - character.height_crouch + character.height_crouch * 0.1f,
            .width = character.width * 0.6f,
            .height = character.height_crouch * 0.8f
        };
    } else {
        character_rect = (Rectangle){
            .x = character.x + character.width * 0.2f,
            .y = character.y + character.height_stand * 0.1f,
            .width = character.width * 0.6f,
            .height = character.height_stand * 0.8f
        };
    }

    // Check collision with arrows
    for (int i = 0; i < arrow_list->count; i++) {
        Rectangle arrow_rect = {
            .x = arrow_list->arrows[i].x + arrow_list->arrows[i].width * 0.2f,
            .y = arrow_list->arrows[i].y + arrow_list->arrows[i].height * 0.2f,
            .width = arrow_list->arrows[i].width * 0.6f,
            .height = arrow_list->arrows[i].height * 0.6f
        };
        if (CheckCollisionRecs(character_rect, arrow_rect)) {
            return true;
        }
    }

    // Check collision with swords
    for (int i = 0; i < sword_list->count; i++) {
        Rectangle sword_rect = {
            .x = sword_list->swords[i].x + sword_list->swords[i].width * 0.2f,
            .y = sword_list->swords[i].y + sword_list->swords[i].height * 0.2f,
            .width = sword_list->swords[i].width * 0.6f,
            .height = sword_list->swords[i].height * 0.6f
        };
        if (CheckCollisionRecs(character_rect, sword_rect)) {
            return true;
        }
    }

    return false;
}

int main() {
    srand(time(NULL));
    int width = 800 * scale_factor;
    int height = 600 * scale_factor;

    InitWindow(width, height, "Survive The Jungle");
    SetTargetFPS(60);

    // Initialize audio device
    InitAudioDevice();

    // Initialize character properties
    Character character = {.x = width / 2,
                           .y = height / 2,
                           .width = 101,
                           .height_stand = 260,
                           .height_crouch = 130,
                           .velocity = 4 * scale_factor,
                           .speed = 7,
                           .jumping = false,
                           .walking = false,};

    // Initialize gravity
    float gravity = 2.3 * scale_factor;

    // Initialize camera
    Vector2 cam_offset = {.x = 0, .y = 0};
    Vector2 cam_target = {.x = 0, .y = 0};

    Camera2D camera = {
        .offset = cam_offset,
        .target = cam_target,
        .rotation = 0.0,
        .zoom = 1.0
    };

    float platform_spacing = 0.01;
    int platform1_whitespace = 45;
    int platform2_whitespace = 20;

    int world_width = width * 10;
    int platform_width = 201;
    int platform_count = world_width / (platform_width + platform_spacing * width);

    int floor_piece_width = 490;
    int floor_piece_height = 190;
    int floor_piece_count = ceil((float)world_width / (float)floor_piece_width);
    int platform_height = 50;

    int platform_min_y = (int)(height * 0.2);
    int platform_max_y = (int)(height - floor_piece_height - platform_height - platform_min_y);
    int floor_spacing = 33;

    // Background Image
    Texture2D background_texture = LoadTexture("img/background.png");
    int background_width = 1792;
    int backgroung_overflow = background_width - width;
    float background_ratio = 1 / ((float)(world_width - width) / (float)(backgroung_overflow));
    int background_x = 0;

    platform_count += floor_piece_count;

    // Load the arrow texture
    Texture2D arrow_texture = LoadTexture("img/arrow4.png");

    // Load the sword texture
    Texture2D sword_texture = LoadTexture("img/sward.png");

    // Initialize the arrow list
    ArrowList arrow_list;
    init_arrow_list(&arrow_list, 10);

    // Initialize the sword list
    SwordList sword_list;
    init_sword_list(&sword_list, 10);

    // Initialize platforms
    Platform platforms[platform_count + 1];

    // Platforms1
    Image platform1_img = LoadImage("img/platform1.png");
    Texture2D platform1_texture = LoadTextureFromImage(platform1_img);
    UnloadImage(platform1_img);

    // Platforms2
    Image platform2_img = LoadImage("img/platform2.png");
    Texture2D platform2_texture = LoadTextureFromImage(platform2_img);
    UnloadImage(platform2_img);

    // Floor (Ground)
    Image floor_img = LoadImage("img/floor.png");
    Texture2D floor_piece_texture = LoadTextureFromImage(floor_img);
    UnloadImage(floor_img);

    int i = 0;
    int floor_x = 0;
    for (; i < floor_piece_count; i++) {
        platforms[i].x = floor_x;
        platforms[i].y = height - floor_piece_height + floor_spacing;
        platforms[i].width = floor_piece_width;
        platforms[i].height = floor_piece_height;
        platforms[i].type = FLOOR;
        floor_x += platforms[i].width; // Increment floor_x
    }

    int platform_x = (int)(width * 0.1);

    for (; i <= platform_count; i++) {
        int random_number = rand_float() * platform_max_y;
        platforms[i].x = platform_x;
        platforms[i].y = rand_float() * platform_max_y + platform_min_y;
        platforms[i].width = platform_width;
        platforms[i].height = platform_height;
        platforms[i].type = PLATFORM;
        platform_x += platforms[i].width + (int)(platform_width * platform_spacing);
    }

    // Character standing
    Image character_standing = LoadImage("img/standing.png");
    Texture2D character_standing_right = LoadTextureFromImage(character_standing);
    ImageFlipHorizontal(&character_standing);
    Texture2D character_standing_left = LoadTextureFromImage(character_standing);
    UnloadImage(character_standing);

    // Character Jumping
    Image character_jumping = LoadImage("img/jumping.png");
    Texture2D character_jumping_right = LoadTextureFromImage(character_jumping);
    ImageFlipHorizontal(&character_jumping);
    Texture2D character_jumping_left = LoadTextureFromImage(character_jumping);
    UnloadImage(character_jumping);

    // Character walking
    // Walk 1
    Image character_walk1 = LoadImage("img/walk1.png");
    Texture2D character_walk1_right = LoadTextureFromImage(character_walk1);
    ImageFlipHorizontal(&character_walk1);
    Texture2D character_walk1_left = LoadTextureFromImage(character_walk1);
    UnloadImage(character_walk1);

    // Walk 2
    Image character_walk2 = LoadImage("img/walk2.png");
    Texture2D character_walk2_right = LoadTextureFromImage(character_walk2);
    ImageFlipHorizontal(&character_walk2);
    Texture2D character_walk2_left = LoadTextureFromImage(character_walk2);
    UnloadImage(character_walk2);

    // Character crouching
    Image character_crouching_left_img = LoadImage("img/crouch1.png");
    Texture2D character_crouching_left = LoadTextureFromImage(character_crouching_left_img);
    UnloadImage(character_crouching_left_img);

    Image character_crouching_right_img = LoadImage("img/crouch2.png");
    Texture2D character_crouching_right = LoadTextureFromImage(character_crouching_right_img);
    UnloadImage(character_crouching_right_img);

    float arrow_timer = 0.0f;
    const float arrow_interval = 2.3f; // Interval in seconds

    float sword_timer = 0.0f;
    const float sword_interval = 10.0f; // Interval in seconds

    const char *game_over_text = "Game Over!";

    // Load the game over texture
    Texture2D game_over_texture = LoadTexture("img/Game Over.png");

    //New variables for score
    int score = 0;
    float score_timer = 0.0f;
    const float score_interval = 1.3f; // Interval in seconds

    // Load and play background music
    Music background_music = LoadMusicStream("img/background_music.mp3");
    Music gameover_music = LoadMusicStream("img/gameover1.mp3");
    PlayMusicStream(background_music);
    SetMusicVolume(background_music, 0.5f);  // Set volume to 50%, adjust as needed
    SetMusicVolume(gameover_music, 0.5f);  // Set volume for game over music

    bool game_over = false;
    bool gameover_music_started = false;


    // Game loop
    while (!WindowShouldClose()) {
        //Updating Background Music
        if (!game_over) {
            UpdateMusicStream(background_music);
        } else {
            if (!gameover_music_started) {
                StopMusicStream(background_music);
                PlayMusicStream(gameover_music);
                gameover_music_started = true;
            }
            UpdateMusicStream(gameover_music);
        }


        float delta_time = GetFrameTime(); // Get the time elapsed since the last frame
        arrow_timer += delta_time; // Update the arrow timer
        sword_timer += delta_time; // Update the sword timer

        BeginDrawing();

        character.walking = false;
        character.crouching = false; // Reset crouching state

        if (character.x > width * 0.6) {
            camera.offset.x = -(character.x - width * 0.6);
        } else if (character.x < width * 0.4) {
            camera.offset.x = -(character.x - width * 0.6);
        }

        if (camera.offset.x > 0) {
            camera.offset.x = 0;
        }
        if (camera.offset.x < -(world_width - width)) {
            camera.offset.x = -(world_width - width);
        }

        BeginMode2D(camera);

        background_x = -(camera.offset.x);
        background_x -= background_x * background_ratio;

        if (!game_over) {
            // Update character's position and velocity
            character.y += character.velocity;
            character.velocity += gravity;

            // Check if character is on any platform
            int current_platform = character_on_platform(character, platforms, platform_count);
            if (current_platform != -1) {
                if (character.velocity > 0) {
                    character.velocity = 0;
                    character.y = platforms[current_platform].y - character.height_stand;
                    character.jumping = false;
                }
                // Jump if space is pressed
                if (IsKeyPressed(KEY_SPACE)) {
                    character.velocity = -30 * scale_factor; // Negative velocity to make the character jump
                    character.jumping = true;
                }
            }

            // Move the character
            if (IsKeyDown(KEY_LEFT)) {
                character.walking = true;
                character.x -= character.speed * scale_factor;
                character.direction = -1;
            }

            if (IsKeyDown(KEY_RIGHT)) {
                character.walking = true;
                character.x += character.speed * scale_factor;
                character.direction = 1;
            }

            // Crouch the character
            if (IsKeyDown(KEY_DOWN)) {
                character.crouching = true; 
            }

            // Generate new arrow at the rightmost x coordinate of the screen every 2.5 seconds
            if (arrow_timer >= arrow_interval) {
                int rightmost_x = width - camera.offset.x;
                int floor_y = height - floor_piece_height + floor_spacing;
                Arrow new_arrow = create_arrow(arrow_texture, rightmost_x, floor_y);
                add_arrow(&arrow_list, new_arrow);
                arrow_timer = 0.0f; // Reset the timer
            }

            // Generate new sword at the rightmost x coordinate of the screen every 10 seconds
            if (sword_timer >= sword_interval) {
                int rightmost_x = width - camera.offset.x;
                int floor_y = height - floor_piece_height + floor_spacing;
                Sword new_sword = create_sword(sword_texture, rightmost_x, floor_y);
                add_sword(&sword_list, new_sword);
                sword_timer = 0.0f; // Reset the timer
            }

            // Update game logic
            update_arrows(&arrow_list, width, camera.offset.x, score);
            update_swords(&sword_list, width, camera.offset.x, score);

            // Check for collision
            game_over = check_character_collision(character, &arrow_list, &sword_list,character.crouching);

            // Update score
            score_timer += delta_time;
            if (score_timer >= score_interval) {
                score++;
                score_timer = 0.0f; // Reset the timer
            }
        }

        // Clear the background
        ClearBackground(WHITE);

        // Draw background
        DrawTexture(background_texture, background_x, 0, WHITE);

        // Draw character platforms
        for (int i = 0; i < platform_count; i++) {
            if (platforms[i].type == FLOOR) {
                DrawTexture(floor_piece_texture, platforms[i].x - 20, platforms[i].y - floor_spacing, WHITE);
            } else {
                Texture2D platform_texture = platform1_texture;
                int whitespace = platform1_whitespace;
                if (i % 2 == 0) {
                    platform_texture = platform2_texture;
                    whitespace = platform2_whitespace;
                }
                DrawTexture(platform_texture, platforms[i].x, platforms[i].y - whitespace, WHITE);
            }
        }

        // Draw the character
        Texture2D character_texture;
        if (character.crouching) {
            character_texture = (character.direction == -1) ? character_crouching_left : character_crouching_right;
        } else if (character.jumping) {
            character_texture = (character.direction == -1) ? character_jumping_left : character_jumping_right;
        } else if (character.walking) {
            double time = GetTime() * 10 / 2;
            character_texture = (character.direction == -1) ? ((int)time % 2 == 0 ? character_walk1_left : character_walk2_left) : ((int)time % 2 == 0 ? character_walk1_right : character_walk2_right);
        } else {
            character_texture = (character.direction == -1) ? character_standing_left : character_standing_right;
        }

        if (character.x < 0) {
            character.x = 0;
        }
        if (character.x > world_width - character.width) {
            character.x = world_width - character.width;
        }

        if(character.crouching==true){
            DrawTexture(character_texture, character.x, character.y+135, WHITE);
        }else{
            DrawTexture(character_texture, character.x, character.y, WHITE);
        }
        // Draw all arrows
        draw_arrows(&arrow_list);

        // Draw all swords
        draw_swords(&sword_list);

        EndMode2D();

        // Draw score
        char score_text[20];
        sprintf(score_text, "Score: %d", score);
        DrawText(score_text, 10, 10, 30, RED);

        // Draw game over screen
        if (game_over) {
            DrawTexture(game_over_texture, width/2 - game_over_texture.width/2, height/2 - game_over_texture.height/2, WHITE);
            char score_text[20];
            sprintf(score_text, "Score: %d", score);
            DrawText(score_text, width/3+30, 30, 80, BRIGHT_RED);
        }

        EndDrawing();
    }

    UnloadTexture(character_standing_right);
    UnloadTexture(character_standing_left);
    UnloadTexture(character_jumping_right);
    UnloadTexture(character_jumping_left);
    UnloadTexture(character_walk1_right);
    UnloadTexture(character_walk1_left);
    UnloadTexture(character_walk2_right);
    UnloadTexture(character_walk2_left);
    UnloadTexture(character_crouching_right);
    UnloadTexture(character_crouching_left);
    UnloadTexture(floor_piece_texture);
    UnloadTexture(platform1_texture);
    UnloadTexture(platform2_texture);
    UnloadTexture(background_texture);
    UnloadTexture(arrow_texture);
    UnloadTexture(sword_texture);
    UnloadTexture(game_over_texture);
    free(arrow_list.arrows);
    free(sword_list.swords);

    // Cleanup
    StopMusicStream(background_music);
    StopMusicStream(gameover_music);
    UnloadMusicStream(background_music);
    UnloadMusicStream(gameover_music);
    CloseAudioDevice();

    CloseWindow();
    return 0;
}

