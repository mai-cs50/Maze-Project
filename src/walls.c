#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h> // For errno

#define WIDTH 800
#define HEIGHT 600
#define MAP_WIDTH 10
#define MAP_HEIGHT 10

int map[MAP_WIDTH][MAP_HEIGHT];
SDL_Texture *wallTexture; // Texture for walls
SDL_Texture *groundTexture; // Texture for ground
SDL_Texture *ceilingTexture; // Texture for ceiling
int drawMapEnabled = 0; // Variable to toggle map display

float cameraX = 22.0f;
float cameraY = 12.0f;
float cameraAngle = 0.0f;
float rotationSpeed = 0.05f;
float moveSpeed = 5.0f;

int isCollision(float newX, float newY) {
    int mapX = (int)newX;
    int mapY = (int)newY;

    if (mapX < 0 || mapX >= MAP_WIDTH || mapY < 0 || mapY >= MAP_HEIGHT) {
        return 1;
    }

    return map[mapX][mapY] > 0;
}

SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path) {
    SDL_Surface* surface = SDL_LoadBMP(path);
    if (!surface) {
        fprintf(stderr, "Unable to load image %s: %s\n", path, SDL_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void loadMap(const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Unable to open map file %s: %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            fscanf(file, "%1d", &map[x][y]);
        }
    }

    fclose(file);
}

void drawMap(SDL_Renderer *renderer) {
    // Implement drawing of the map here, for example:
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White color
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (map[x][y] > 0) {
                SDL_Rect rect;
                rect.x = x * (WIDTH / MAP_WIDTH);
                rect.y = y * (HEIGHT / MAP_HEIGHT);
                rect.w = WIDTH / MAP_WIDTH;
                rect.h = HEIGHT / MAP_HEIGHT;
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}

void renderWalls(SDL_Renderer *renderer) {
    int x;
    for (x = 0; x < WIDTH; x++) {
        float rayAngle = (cameraAngle - M_PI / 6) + ((float)x / (float)WIDTH) * (M_PI / 3);
        float rayX = cameraX;
        float rayY = cameraY;
        float stepSize = 0.1f;
        float distance = 0.0f;
        int hit = 0;
        int wallOrientation = 0; // 0 = None, 1 = Vertical, 2 = Horizontal

        while (!hit) {
            rayX += cos(rayAngle) * stepSize;
            rayY += sin(rayAngle) * stepSize;
            distance += stepSize;

            int mapX = (int)rayX;
            int mapY = (int)rayY;

            if (mapX < 0 || mapX >= MAP_WIDTH || mapY < 0 || mapY >= MAP_HEIGHT) {
                break;
            }

            if (map[mapX][mapY] > 0) {
                if (fabs(fmod(rayX, 1.0f)) < 0.1f) {
                    wallOrientation = 1; // Vertical wall
                } else if (fabs(fmod(rayY, 1.0f)) < 0.1f) {
                    wallOrientation = 2; // Horizontal wall
                }
                hit = 1;
            }
        }

        int lineHeight = (int)(HEIGHT / (distance * cos(cameraAngle - rayAngle)));
        int drawStart = -lineHeight / 2 + HEIGHT / 2;
        int drawEnd = lineHeight / 2 + HEIGHT / 2;

        if (drawStart < 0) drawStart = 0;
        if (drawEnd >= HEIGHT) drawEnd = HEIGHT - 1;

        SDL_Rect srcRect;
        SDL_Rect destRect;
        srcRect.x = 0;
        srcRect.y = 0;
        srcRect.w = 64; // Assuming texture width
        srcRect.h = 64; // Assuming texture height
        destRect.x = x;
        destRect.y = drawStart;
        destRect.w = 1;
        destRect.h = drawEnd - drawStart;

        // Draw textured wall
        if (wallOrientation == 1) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red for vertical walls
        } else if (wallOrientation == 2) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Blue for horizontal walls
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White for empty or default
        }
        SDL_RenderCopy(renderer, wallTexture, &srcRect, &destRect);
    }
}

void renderGroundAndCeiling(SDL_Renderer *renderer) {
    // Ground
    SDL_Rect groundRect = { 0, HEIGHT / 2, WIDTH, HEIGHT / 2 };
    SDL_RenderCopy(renderer, groundTexture, NULL, &groundRect);

    // Ceiling
    SDL_Rect ceilingRect = { 0, 0, WIDTH, HEIGHT / 2 };
    SDL_RenderCopy(renderer, ceilingTexture, NULL, &ceilingRect);
}

void updatePlayerPosition(const Uint8 *state) {
    float newX = cameraX;
    float newY = cameraY;

    if (state[SDL_SCANCODE_W]) {
        newX += cos(cameraAngle) * moveSpeed;
        newY += sin(cameraAngle) * moveSpeed;
    }
    if (state[SDL_SCANCODE_S]) {
        newX -= cos(cameraAngle) * moveSpeed;
        newY -= sin(cameraAngle) * moveSpeed;
    }
    if (state[SDL_SCANCODE_A]) {
        newX -= cos(cameraAngle + M_PI / 2) * moveSpeed;
        newY -= sin(cameraAngle + M_PI / 2) * moveSpeed;
    }
    if (state[SDL_SCANCODE_D]) {
        newX += cos(cameraAngle - M_PI / 2) * moveSpeed;
        newY += sin(cameraAngle - M_PI / 2) * moveSpeed;
    }

    // Check collisions
    if (!isCollision(newX, newY)) {
        cameraX = newX;
        cameraY = newY;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <mapfile>\n", argv[0]);
        return EXIT_FAILURE;
    }

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Raycasting", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    wallTexture = loadTexture(renderer, "images/wood.png"); // Load your wall texture
    groundTexture = loadTexture(renderer, "images/mossystone.png"); // Load your ground texture
    ceilingTexture = loadTexture(renderer, "images/mossystone.png"); // Load your ceiling texture

    if (!wallTexture || !groundTexture || !ceilingTexture) {
        fprintf(stderr, "Failed to load one or more textures.\n");
        return EXIT_FAILURE;
    }

    // Load map
    loadMap(argv[1]);

    int running = 1;
    SDL_Event event;
    const Uint8 *keyState = SDL_GetKeyboardState(NULL);

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_LEFT) {
                    cameraAngle -= rotationSpeed;
                } else if (event.key.keysym.sym == SDLK_RIGHT) {
                    cameraAngle += rotationSpeed;
                } else if (event.key.keysym.sym == SDLK_m) {
                    drawMapEnabled = !drawMapEnabled; // Toggle map display
                }
            }
        }

        updatePlayerPosition(keyState);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
        SDL_RenderClear(renderer);

        renderGroundAndCeiling(renderer);
        renderWalls(renderer);

        if (drawMapEnabled) {
            drawMap(renderer); // Draw the map if enabled
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // Roughly 60 FPS
    }

    SDL_DestroyTexture(wallTexture);
    SDL_DestroyTexture(groundTexture);
    SDL_DestroyTexture(ceilingTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
