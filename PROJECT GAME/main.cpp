#include "Src/ui/FadeEffect.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>

// Điều chỉnh FRAME_DELAY để hoạt ảnh nhanh hơn (giảm từ 100 xuống 80)
const int FRAME_DELAY = 80;
int currentFrame = 0;
Uint32 lastFrameTime = 0;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
int sun_Count = 500;

// Biến cooldown và trạng thái kéo thả
bool isDragging = false;
bool isDraggingPeaShooter = false;
bool onCooldownSunflower = false;  // Đổi tên biến cooldown để rõ ràng hơn
bool onCooldownPeaShooter = false; // Thêm biến cooldown riêng cho Peashooter
int cooldownTimerSunflower = 0;
int cooldownTimerPeaShooter = 0;
const int cooldownDuration = 300;

SDL_Rect draggingRect;
SDL_Texture* sunflowerPreview = nullptr;
bool previewingSunflower = false;
SDL_Rect previewRect;
SDL_Texture* SunFlowerTexture = nullptr;
SDL_Texture* peaShooterPreview = nullptr;
bool previewingPeaShooter = false;
SDL_Rect draggingPeaShooterRect;
std::vector<SDL_Texture*> peaShooterFrames;
std::vector<SDL_Rect> placedPeaShooters;
int peaShooterCurrentFrame = 0;
Uint32 peaShooterLastFrameTime = 0;

struct Sunflower {
    SDL_Rect rect;
    Uint32 lastProducedTime;
    int gridRow;
    int gridCol;
    float baseSize = 75;
};
std::vector<Sunflower> placedSunflowers;

struct Cell {
    int x, y;
};

Cell grid[5][9] = {
    {{210, 123}, {269, 127}, {328, 120}, {387, 116}, {446, 120}, {504, 120}, {564, 123}, {618, 120}, {677, 120}},
    {{210, 229}, {268, 226}, {326, 225}, {384, 228}, {444, 225}, {504, 226}, {561, 229}, {616, 227}, {680, 230}},
    {{211, 323}, {268, 325}, {327, 329}, {389, 330}, {443, 334}, {503, 331}, {560, 335}, {617, 332}, {684, 336}},
    {{210, 422}, {269, 424}, {331, 429}, {388, 433}, {444, 427}, {505, 431}, {561, 430}, {617, 430}, {683, 429}},
    {{208, 518}, {272, 526}, {330, 524}, {389, 522}, {446, 523}, {504, 523}, {561, 522}, {620, 524}, {683, 523}}
};

void updateGridCoordinates(int windowWidth, int windowHeight) {
    int startX = windowWidth * 0.26;
    int startY = windowHeight * 0.2;
    int cellWidth = windowWidth * 0.07;
    int cellHeight = windowHeight * 0.15;

    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 9; col++) {
            grid[row][col].x = startX + col * cellWidth;
            grid[row][col].y = startY + row * cellHeight;
        }
    }
}

void updatePlacedPlants(int oldWidth, int oldHeight, int newWidth, int newHeight) {
    float xRatio = (float)newWidth / oldWidth;
    float yRatio = (float)newHeight / oldHeight;

    for (auto& sunflower : placedSunflowers) {
        sunflower.rect.x *= xRatio;
        sunflower.rect.y *= yRatio;
        sunflower.rect.w = 75 * xRatio;
        sunflower.rect.h = 75 * yRatio;
    }

    for (auto& peaShooter : placedPeaShooters) {
        peaShooter.x *= xRatio;
        peaShooter.y *= yRatio;
        peaShooter.w = 75 * xRatio;
        peaShooter.h = 75 * yRatio;
    }
}

bool canPlacePlant(int x, int y, int cost) {
    // Kiểm tra đủ Sun không
    if (sun_Count < cost || y <= 100) return false;

    // Kiểm tra ô trống
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 9; col++) {
            int gridX = grid[row][col].x;
            int gridY = grid[row][col].y;
            if (abs(x - gridX) <= 30 && abs(y - gridY) <= 30) {
                // Kiểm tra Sunflower
                for (const auto& sunflower : placedSunflowers) {
                    if (abs(sunflower.rect.x - gridX) <= 5 && abs(sunflower.rect.y - gridY) <= 5) {
                        return false;
                    }
                }
                // Kiểm tra Peashooter
                for (const auto& peaShooter : placedPeaShooters) {
                    if (abs(peaShooter.x - gridX) <= 5 && abs(peaShooter.y - gridY) <= 5) {
                        return false;
                    }
                }
                return true;
            }
        }
    }
    return false;
}

struct Sun {
    SDL_Texture* texture;
    SDL_Rect rect;
    int targetY;
    bool collected;
    bool movingToSunBar;
    int targetX, targetFinalY;
    bool soundPlayed;
};

std::vector<Sun> suns;
SDL_Texture* sunTexture;
Mix_Chunk* pointSound;
int sunSpawnTimer = 0;

// Hàm mới: Tạo Sun từ Sunflower
void produceSunFromSunflowers(SDL_Renderer* renderer) {
    Uint32 currentTime = SDL_GetTicks();

    for (auto& sunflower : placedSunflowers) {
        if (currentTime - sunflower.lastProducedTime > 10000) {
            Sun sun;
            sun.texture = sunTexture;
            sun.rect.w = 62;
            sun.rect.h = 62;

            // Xuất hiện ngay chính giữa phía dưới cây
            sun.rect.x = sunflower.rect.x + (sunflower.rect.w - sun.rect.w) / 2;
            sun.rect.y = sunflower.rect.y + sunflower.rect.h;

            // Rơi xuống ngay tại chỗ (không di chuyển xuống thêm)
            sun.targetY = sunflower.rect.y + sunflower.rect.h;

            sun.collected = false;
            sun.movingToSunBar = false;
            sun.targetX = 10;
            sun.targetFinalY = 0;
            sun.soundPlayed = false;
            suns.push_back(sun);

            sunflower.lastProducedTime = currentTime;

            // Có thể thêm âm thanh ở đây
            // Mix_PlayChannel(-1, sunSound, 0);
        }
    }
}

void spawnSun(SDL_Renderer* renderer, int windowWidth, int windowHeight) {
    if (sunSpawnTimer >= 3600) {
        Sun sun;
        sun.texture = sunTexture;
        sun.rect.w = 62;
        sun.rect.h = 62;
        sun.rect.x = rand() % (windowWidth - sun.rect.w);
        sun.rect.y = 0;
        sun.targetY = 50 + rand() % (windowHeight - 100);
        sun.collected = false;
        sun.movingToSunBar = false;
        sun.targetX = 10;
        sun.targetFinalY = 0;
        sun.soundPlayed = false;
        suns.push_back(sun);
        sunSpawnTimer = 0;
    }
    sunSpawnTimer++;
}

void updateSuns() {
    for (size_t i = 0; i < suns.size(); i++) {
        Sun& sun = suns[i];
        if (!sun.collected && sun.rect.y < sun.targetY) {
            sun.rect.y += 2;
        }

        if (sun.movingToSunBar) {
            int speed = 25;
            if (sun.rect.x < sun.targetX) sun.rect.x += speed;
            if (sun.rect.x > sun.targetX) sun.rect.x -= speed;
            if (sun.rect.y > sun.targetFinalY) sun.rect.y -= speed;

            if (abs(sun.rect.x - sun.targetX) < speed && abs(sun.rect.y - sun.targetFinalY) < speed) {
                if (!sun.collected) {
                    sun_Count += 25;
                    sun.collected = true;
                }
            }

            if (!sun.soundPlayed) {
                Mix_PlayChannel(-1, pointSound, 0);
                sun.soundPlayed = true;
            }
        }
    }

    suns.erase(std::remove_if(suns.begin(), suns.end(), [](const Sun& sun) {
        return sun.collected;
        }), suns.end());
}

void renderSuns(SDL_Renderer* renderer) {
    for (const auto& sun : suns) {
        if (!sun.collected) {
            SDL_RenderCopy(renderer, sun.texture, nullptr, &sun.rect);
        }
    }
}

bool collectSun(int x, int y) {
    for (auto& sun : suns) {
        if (!sun.collected && !sun.movingToSunBar &&
            x >= sun.rect.x && x <= sun.rect.x + sun.rect.w &&
            y >= sun.rect.y && y <= sun.rect.y + sun.rect.h) {
            sun.movingToSunBar = true;
            sun.targetFinalY = 20;
            return true;
        }
    }
    return false;
}

TTF_Font* font;
SDL_Color textColor = { 0, 0, 0 };

void renderSunCount(SDL_Renderer* renderer) {
    std::string sunText = std::to_string(sun_Count);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, sunText.c_str(), textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = { 50, 10, textSurface->w, textSurface->h };
    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void handleMouseEvent(SDL_Event& e, SDL_Rect& SunFlowerCardRect, SDL_Rect& PeaShooterCardRect) {
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        SDL_Point mousePos = { e.button.x, e.button.y };

        if (SDL_PointInRect(&mousePos, &SunFlowerCardRect) && !onCooldownSunflower) {
            isDragging = true;
            draggingRect = SunFlowerCardRect;
            previewingSunflower = true;
        }
        else if (SDL_PointInRect(&mousePos, &PeaShooterCardRect) && !onCooldownPeaShooter) {
            isDraggingPeaShooter = true;
            draggingPeaShooterRect = PeaShooterCardRect;
            previewingPeaShooter = true;
        }
        else {
            collectSun(e.button.x, e.button.y);
        }
    }

    if (e.type == SDL_MOUSEBUTTONUP) {
        bool placedSuccessfully = false;

        if (isDragging) {
            for (int row = 0; row < 5; row++) {
                for (int col = 0; col < 9; col++) {
                    int gridX = grid[row][col].x;
                    int gridY = grid[row][col].y;
                    if (abs(e.button.x - gridX) <= 30 && abs(e.button.y - gridY) <= 30) {
                        if (canPlacePlant(gridX, gridY, 50) && !onCooldownSunflower) {
                            sun_Count -= 50;
                            Sunflower newSunflower;
                            newSunflower.rect = { gridX - 37, gridY - 37, 75, 75 };
                            newSunflower.gridRow = row;
                            newSunflower.gridCol = col;
                            newSunflower.lastProducedTime = SDL_GetTicks();
                            placedSunflowers.push_back(newSunflower);
                            onCooldownSunflower = true;
                            cooldownTimerSunflower = cooldownDuration;
                            placedSuccessfully = true;
                        }
                        break;
                    }
                }
            }
            if (!placedSuccessfully) {
                previewingSunflower = false; // Ẩn preview nếu không đặt được
            }
            isDragging = false;
        }
        else if (isDraggingPeaShooter) {
            for (int row = 0; row < 5; row++) {
                for (int col = 0; col < 9; col++) {
                    int gridX = grid[row][col].x;
                    int gridY = grid[row][col].y;
                    if (abs(e.button.x - gridX) <= 30 && abs(e.button.y - gridY) <= 30) {
                        if (canPlacePlant(gridX, gridY, 100) && !onCooldownPeaShooter) {
                            sun_Count -= 100;
                            SDL_Rect newPeaShooter = { gridX - 37, gridY - 37, 75, 75 };
                            placedPeaShooters.push_back(newPeaShooter);
                            onCooldownPeaShooter = true;
                            cooldownTimerPeaShooter = cooldownDuration;
                            placedSuccessfully = true;
                        }
                        break;
                    }
                }
            }
            if (!placedSuccessfully) {
                previewingPeaShooter = false; // Ẩn preview nếu không đặt được
            }
            isDraggingPeaShooter = false;
        }
    }

    if (e.type == SDL_MOUSEMOTION) {
        if (isDragging) {
            draggingRect.x = e.motion.x - draggingRect.w / 2;
            draggingRect.y = e.motion.y - draggingRect.h / 2;
        }
        else if (isDraggingPeaShooter) {
            draggingPeaShooterRect.x = e.motion.x - draggingPeaShooterRect.w / 2;
            draggingPeaShooterRect.y = e.motion.y - draggingPeaShooterRect.h / 2;
        }
    }
}

void updateCooldown() {
    // Cập nhật cooldown cho Sunflower
    if (onCooldownSunflower) {
        cooldownTimerSunflower--;
        if (cooldownTimerSunflower <= 0) {
            onCooldownSunflower = false;
            cooldownTimerSunflower = 0;
        }
    }

    // Cập nhật cooldown cho Peashooter
    if (onCooldownPeaShooter) {
        cooldownTimerPeaShooter--;
        if (cooldownTimerPeaShooter <= 0) {
            onCooldownPeaShooter = false;
            cooldownTimerPeaShooter = 0;
        }
    }
}

void renderSunflowerCard(SDL_Renderer* renderer, SDL_Texture* SunFlowerCardTexture, SDL_Rect& SunFlowerCardRect) {
    if (isDragging) {
        SDL_RenderCopy(renderer, sunflowerPreview, nullptr, &draggingRect);
    }
    else {
        if (onCooldownSunflower) {
            SDL_SetTextureColorMod(SunFlowerCardTexture, 128, 128, 128);
        }
        SDL_RenderCopy(renderer, SunFlowerCardTexture, nullptr, &SunFlowerCardRect);
        if (onCooldownSunflower) {
            SDL_SetTextureColorMod(SunFlowerCardTexture, 255, 255, 255);
        }
    }
}

void renderPeaShooterCard(SDL_Renderer* renderer, SDL_Texture* PeaShooterCardTexture, SDL_Rect& PeaShooterCardRect) {
    if (isDraggingPeaShooter) {
        SDL_RenderCopy(renderer, peaShooterPreview, nullptr, &draggingPeaShooterRect);
    }
    else {
        if (onCooldownPeaShooter) {
            SDL_SetTextureColorMod(PeaShooterCardTexture, 128, 128, 128);
        }
        SDL_RenderCopy(renderer, PeaShooterCardTexture, nullptr, &PeaShooterCardRect);
        if (onCooldownPeaShooter) {
            SDL_SetTextureColorMod(PeaShooterCardTexture, 255, 255, 255);
        }
    }
}

void renderSunflowerPreview(SDL_Renderer* renderer) {
    if (previewingSunflower && isDragging) { // Chỉ hiển thị khi đang kéo
        SDL_RenderCopy(renderer, sunflowerPreview, nullptr, &draggingRect);
    }
}

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
std::vector<SDL_Texture*> frames;

void loadFrames(SDL_Renderer* renderer) {
    frames.clear();
    for (int i = 0; i < 54; ++i) {
        std::string path = "Assets/Images/Plants/SunFlower/sunflower_frames/frame_" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            if (texture) {
                frames.push_back(texture);
            }
        }
    }
}

void loadPeaShooterFrames(SDL_Renderer* renderer) {
    peaShooterFrames.clear();
    for (int i = 0; i < 60; ++i) { // Giả sử Peashooter có 30 frame
        std::string path = "Assets/Images/Plants/Peashooter/peashooter_frames/frame_" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            if (texture) {
                peaShooterFrames.push_back(texture);
            }
        }
    }
}

void renderSunflowers(SDL_Renderer* renderer) {
    if (placedSunflowers.empty() || frames.empty()) return;

    Uint32 currentTime = SDL_GetTicks();
    if (currentTime > lastFrameTime + FRAME_DELAY) {
        currentFrame = (currentFrame + 1) % frames.size();
        lastFrameTime = currentTime;
    }

    for (const auto& sunflower : placedSunflowers) {
        SDL_RenderCopy(renderer, frames[currentFrame], nullptr, &sunflower.rect);
    }
}

void renderPeaShooters(SDL_Renderer* renderer) {
    if (placedPeaShooters.empty() || peaShooterFrames.empty()) return;

    Uint32 currentTime = SDL_GetTicks();
    if (currentTime > peaShooterLastFrameTime + FRAME_DELAY) {
        peaShooterCurrentFrame = (peaShooterCurrentFrame + 1) % peaShooterFrames.size();
        peaShooterLastFrameTime = currentTime;
    }

    for (const auto& peaShooter : placedPeaShooters) {
        SDL_RenderCopy(renderer, peaShooterFrames[peaShooterCurrentFrame], nullptr, &peaShooter);
    }
}

void renderPeaShooterPreview(SDL_Renderer* renderer) {
    if (previewingPeaShooter && isDraggingPeaShooter) { // Chỉ hiển thị khi đang kéo
        SDL_RenderCopy(renderer, peaShooterPreview, nullptr, &draggingPeaShooterRect);
    }
}

void close() {
    for (auto tex : frames) SDL_DestroyTexture(tex);
    SDL_DestroyTexture(sunflowerPreview);
    SDL_DestroyTexture(SunFlowerTexture);
    SDL_DestroyTexture(peaShooterPreview);
    for (auto tex : peaShooterFrames) SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    Mix_CloseAudio();
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    srand(static_cast<unsigned>(time(0)));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer initialization failed: " << Mix_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf initialization failed: " << TTF_GetError() << std::endl;
        return -1;
    }

    font = TTF_OpenFont("Assets/Fonts/Roboto-Bold.ttf", 24);
    if (!font) {
        std::cerr << "Font loading failed: " << TTF_GetError() << std::endl;
        return -1;
    }

    window = SDL_CreateWindow("Sunflower Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    IMG_Init(IMG_INIT_PNG);
    SDL_Texture* mainMenuTexture = FadeEffect::loadTexture("Assets/Images/Background/background1a.png", renderer);
    SDL_Texture* itemBarTexture = FadeEffect::loadTexture("Assets/Images/Background/Item_Bar.png", renderer);
    SDL_Texture* sunBarTexture = FadeEffect::loadTexture("Assets/Images/Background/sun_bar.png", renderer);
    sunTexture = FadeEffect::loadTexture("Assets/Images/Background/Sun.png", renderer);
    SDL_Texture* SunFlowerCardTexture = FadeEffect::loadTexture("Assets/Images/Plants/SunFlower/1.png", renderer);
	SDL_Texture* PeaShooterCardTexture = FadeEffect::loadTexture("Assets/Images/Plants/Peashooter/1.png", renderer);    
    sunflowerPreview = FadeEffect::loadTexture("Assets/Images/Plants/SunFlower/Sunflower.png", renderer);
    SunFlowerTexture = FadeEffect::loadTexture("Assets/Images/Plants/SunFlower/Sunflower.png", renderer);
    peaShooterPreview = FadeEffect::loadTexture("Assets/Images/Plants/Peashooter/Peashooter.png", renderer);
    loadPeaShooterFrames(renderer);
    loadFrames(renderer);

    pointSound = Mix_LoadWAV("Assets/Sound/Points.wav");
    Mix_Music* mainMenuMusic = Mix_LoadMUS("Assets/Sound/mainmenu.mp3");
    if (mainMenuMusic) Mix_PlayMusic(mainMenuMusic, -1);

    bool running = true;
    SDL_Event e;
    int windowWidth = SCREEN_WIDTH;
    int windowHeight = SCREEN_HEIGHT;
    updateGridCoordinates(windowWidth, windowHeight);

    SDL_Rect menuRect = { 0, 0, windowWidth, windowHeight };
    SDL_Rect itemBarRect = { 10, 50, 85, 400 };
    SDL_Rect sunBarRect = { 10, 0, 85, 40 };
    SDL_Rect SunFlowerCardRect = { 15, 55, 75, 50 };
	SDL_Rect PeaShooterCardRect = { 15, 115, 75, 50 };
    previewRect.w = 75;
    previewRect.h = 75;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
            else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) {
                int oldWidth = windowWidth;
                int oldHeight = windowHeight;
                windowWidth = e.window.data1;
                windowHeight = e.window.data2;
                menuRect.w = windowWidth;
                menuRect.h = windowHeight;
                updateGridCoordinates(windowWidth, windowHeight);
                updatePlacedPlants(oldWidth, oldHeight, windowWidth, windowHeight);
                itemBarRect.w = static_cast<int>(windowWidth * 0.6 * 85.0 / SCREEN_WIDTH);
                itemBarRect.h = static_cast<int>(windowHeight * 400.0 / SCREEN_HEIGHT);
                sunBarRect.w = static_cast<int>(windowWidth * 0.6 * 85.0 / SCREEN_WIDTH);
                sunBarRect.h = static_cast<int>(windowHeight * 40.0 / SCREEN_HEIGHT);
                SunFlowerCardRect.w = static_cast<int>(windowWidth * 0.6 * 75.0 / SCREEN_WIDTH);
                SunFlowerCardRect.h = static_cast<int>(windowHeight * 0.95 * 50.0 / SCREEN_HEIGHT);
				PeaShooterCardRect.w = static_cast<int>(windowWidth * 0.6 * 75.0 / SCREEN_WIDTH);
				PeaShooterCardRect.h = static_cast<int>(windowHeight * 50.0 / SCREEN_HEIGHT);
            }
            handleMouseEvent(e, SunFlowerCardRect, PeaShooterCardRect);
        }

        updateCooldown();
        spawnSun(renderer, windowWidth, windowHeight);
        produceSunFromSunflowers(renderer);
        updateSuns();

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, mainMenuTexture, nullptr, &menuRect);
        SDL_RenderCopy(renderer, itemBarTexture, nullptr, &itemBarRect);
        SDL_RenderCopy(renderer, sunBarTexture, nullptr, &sunBarRect);
        renderSunflowerCard(renderer, SunFlowerCardTexture, SunFlowerCardRect);
        renderPeaShooterCard(renderer, PeaShooterCardTexture, PeaShooterCardRect); // Sử dụng hàm render mới
        renderSunflowers(renderer);
        renderSunflowerPreview(renderer);
        renderPeaShooters(renderer);
        renderPeaShooterPreview(renderer);
        renderSuns(renderer);
        renderSunCount(renderer);
        SDL_RenderPresent(renderer);
    }

    Mix_FreeMusic(mainMenuMusic);
    Mix_FreeChunk(pointSound);
    close();
    return 0;
}


/*
// Code xác định tọa độ 
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL không thể khởi tạo!" << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Check Click", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cout << "Không thể tạo cửa sổ!" << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cout << "Không thể tạo renderer!" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Load ảnh
    IMG_Init(IMG_INIT_PNG);
    SDL_Surface* imageSurface = IMG_Load("Assets/Images/Background/background1a.png");
    if (!imageSurface) {
        std::cout << "Không thể load ảnh!" << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, imageSurface);
    SDL_FreeSurface(imageSurface);

    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = e.button.x;
                int mouseY = e.button.y;
                std::cout << "Bạn đã click vào tọa độ: (" << mouseX << ", " << mouseY << ")" << std::endl;
            }
        }

        // Render ảnh lên màn hình
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    // Dọn dẹp
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}

#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <string>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int FRAME_DELAY = 100; // Thời gian delay giữa các frame (ms)

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
std::vector<SDL_Texture*> frames;

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;
    window = SDL_CreateWindow("Sunflower Animation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) return false;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) return false;
    IMG_Init(IMG_INIT_PNG);
    return true;
}

void loadFrames() {
    for (int i = 0; i < 54; ++i) { // 54 frames
        std::string path = "Assets/Images/Plants/SunFlower/sunflower_frames/frame_" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (surface) {
            frames.push_back(SDL_CreateTextureFromSurface(renderer, surface));
            SDL_FreeSurface(surface);
        }
    }
}

void close() {
    for (auto tex : frames) SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    if (!init()) return -1;
    loadFrames();

    bool running = true;
    SDL_Event e;
    int frame = 0;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
        }
        SDL_RenderClear(renderer);
        SDL_Rect dst = { 200, 150, 200, 200 };
        SDL_RenderCopy(renderer, frames[frame], nullptr, &dst);
        SDL_RenderPresent(renderer);
        SDL_Delay(FRAME_DELAY);
        frame = (frame + 1) % frames.size();
    }
    close();
    return 0;
}
*/




