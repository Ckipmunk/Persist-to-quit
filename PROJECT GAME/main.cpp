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
#include <map>
#include <algorithm>

// ==============================================
// Constants and Global Variables
// ==============================================
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Animation settings
const int SUNFLOWER_FRAME_DELAY = 80;
const int PEASHOOTER_FRAME_DELAY = 60;
const int WALNUT_FRAME_DELAY = 100;
const int ZOMBIE_FRAME_DELAY = 60;
const int ZOMBIE_SPAWN_TIME = 10000;
const int ZOMBIE_SPEED = 1;
const int LOSE_DISPLAY_TIME = 3000;

// Game settings
const int WALNUT_COST = 50;
const int WALNUT_HEALTH = 400;
const int CHERRYBOMB_COST = 150;
const int CHERRYBOMB_COOLDOWN_DURATION = 3000;
const int cooldownDuration = 300;
const int PEA_DAMAGE = 10;
const int PEA_SPEED = 5;
const int PEA_SPAWN_DELAY = 1000;
const int ZOMBIE_EAT_DAMAGE = 10;
const int ZOMBIE_EAT_DELAY = 1000;

// Game state
int sun_Count = 500;
bool gameLost = false;
Uint32 gameStartTime = 0;
Uint32 lastZombieSpawnTime = 0;
Uint32 loseTime = 0;

// SDL resources
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;
SDL_Color textColor = { 0, 0, 0 };

// ==============================================
// Structures and Enums
// ==============================================
enum ZombieState {
    ZOMBIE_WALKING,
    ZOMBIE_EATING
};

enum ZombieType {
    ZOMBIE_NORMAL,
    ZOMBIE_CONE,
    ZOMBIE_BOSS
};

struct Plant {
    SDL_Rect rect;
    int gridRow;
    int gridCol;
    int health = 100;
    int level = 1;
    virtual ~Plant() {}
};

struct Sunflower : public Plant {
    Uint32 lastProducedTime;
    float baseSize = 75;
    int level = 1;
};

struct Walnut : public Plant {
    int currentHealthStage = 3;
};

struct Pea {
    SDL_Rect rect;
    int speed;
    int damage;
    int row;
    bool active;
};

struct CherryBomb {
    SDL_Rect rect;
    int gridRow;
    int gridCol;
    Uint32 plantTime;
    bool exploded = false;
};

struct Zombie {
    SDL_Rect rect;
    int health = 100;
    int currentFrame;
    Uint32 lastFrameTime;
    bool active;
    int gridRow;
    SDL_Rect originalSize;
    ZombieState state = ZOMBIE_WALKING;
    Uint32 lastEatTime = 0;
    int targetPlantIndex = -1;
    bool targetIsSunflower = false;
    ZombieType type = ZOMBIE_NORMAL;
};

struct Sun {
    SDL_Texture* texture;
    SDL_Rect rect;
    int targetY;
    bool collected;
    bool movingToSunBar;
    int targetX, targetFinalY;
    bool soundPlayed;
};

struct Grid {
    SDL_Rect area;
    int rows = 5;
    int cols = 9;
    int cellWidth;
    int cellHeight;
};

// ==============================================
// Game Objects
// ==============================================
Grid gameGrid;
std::vector<Sunflower> placedSunflowers;
std::vector<Plant> placedPeaShooters;
std::vector<Walnut> placedWalnuts;
std::vector<CherryBomb> placedCherryBombs;
std::vector<Zombie> zombies;
std::vector<Pea> peas;
std::vector<Sun> suns;

// ==============================================
// Resources
// ==============================================
// Textures
SDL_Texture* loseTexture = nullptr;
SDL_Texture* sunTexture = nullptr;
SDL_Texture* SunFlowerTexture = nullptr;
SDL_Texture* sunflowerPreview = nullptr;
SDL_Texture* peaShooterPreview = nullptr;
SDL_Texture* shovelPreview = nullptr;
SDL_Texture* peaTexture = nullptr;
SDL_Texture* walnutPreview = nullptr;
SDL_Texture* cherryBombPreview = nullptr;
SDL_Texture* CherryBombCardTexture = nullptr;

// Animation frames
std::vector<SDL_Texture*> frames;
std::vector<SDL_Texture*> sunflowerFramesLv2;
std::vector<SDL_Texture*> peaShooterFrames;
std::vector<SDL_Texture*> peaShooterFramesLv2;
std::vector<SDL_Texture*> peaShooterFramesLv3;
std::vector<SDL_Texture*> zombieWalkFrames;
std::vector<SDL_Texture*> zombieEatFrames;
std::vector<SDL_Texture*> coneZombieWalkFrames;
std::vector<SDL_Texture*> coneZombieEatFrames;
std::vector<SDL_Texture*> bossZombieWalkFrames;
std::vector<SDL_Texture*> bossZombieEatFrames;
std::vector<SDL_Texture*> walnutFrames;
std::vector<SDL_Texture*> walnutCrackedFrames1;
std::vector<SDL_Texture*> walnutCrackedFrames2;
std::vector<SDL_Texture*> walnutCrackedFrames3;
std::vector<SDL_Texture*> cherryBombFrames;

// Sounds
Mix_Chunk* pointSound = nullptr;
Mix_Chunk* zombieEatSound = nullptr;
Mix_Chunk* shootSound = nullptr;
Mix_Chunk* loseSound = nullptr;
Mix_Chunk* explosionSound = nullptr;

// ==============================================
// Game State Variables
// ==============================================
// Animation state
int currentFrame = 0;
Uint32 lastFrameTime = 0;
int peaShooterCurrentFrame = 0;
Uint32 peaShooterLastFrameTime = 0;
int sunSpawnTimer = 0;

// Drag and drop state
bool isDragging = false;
bool isDraggingPeaShooter = false;
bool isDraggingWalnut = false;
bool isDraggingCherryBomb = false;
bool isDraggingShovel = false;

// Cooldown state
bool onCooldownSunflower = false;
bool onCooldownPeaShooter = false;
bool onCooldownWalnut = false;
bool onCooldownCherryBomb = false;
int cooldownTimerSunflower = 0;
int cooldownTimerPeaShooter = 0;
int cooldownTimerWalnut = 0;
int cooldownTimerCherryBomb = 0;

// Preview state
bool previewingSunflower = false;
bool previewingPeaShooter = false;
bool previewingWalnut = false;
bool previewingCherryBomb = false;
bool previewingShovel = false;

// Drag rectangles
SDL_Rect draggingRect;
SDL_Rect draggingPeaShooterRect;
SDL_Rect draggingWalnutRect;
SDL_Rect draggingCherryBombRect;
SDL_Rect draggingShovelRect;
SDL_Rect previewRect;

// ==============================================
// Initialization Functions
// ==============================================
void initGrid(int windowWidth, int windowHeight) {
    gameGrid.area.x = windowWidth * 0.22;
    gameGrid.area.y = windowHeight * 0.15;
    gameGrid.area.w = windowWidth * 0.66;
    gameGrid.area.h = windowHeight * 0.8;

    gameGrid.cellWidth = gameGrid.area.w / gameGrid.cols;
    gameGrid.cellHeight = gameGrid.area.h / gameGrid.rows;
}

void loadFrames(SDL_Renderer* renderer) {
    frames.clear();
    sunflowerFramesLv2.clear();
    for (int i = 0; i < 54; ++i) {
        std::string path = "Assets/Images/Plants/SunFlower/sunflower_frames/frame_" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            if (texture) frames.push_back(texture);
        }
    }

    for (int i = 1; i <= 19; ++i) {
        std::string path = "Assets/Images/Plants/SunFlower/sunflower2_frames/frame_" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            if (texture) sunflowerFramesLv2.push_back(texture);
        }
    }
}

void loadPeaShooterFrames(SDL_Renderer* renderer) {
    peaShooterFrames.clear();
    peaShooterFramesLv2.clear();
    peaShooterFramesLv3.clear();
    for (int i = 0; i < 60; ++i) {
        std::string path = "Assets/Images/Plants/Peashooter/peashooter_frames/frame_" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            if (texture) peaShooterFrames.push_back(texture);
        }
    }

    for (int i = 5; i < 50; ++i) {
        std::string path = "Assets/Images/Plants/Peashooter/peashooter2_frames/frame_" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            if (texture) peaShooterFramesLv2.push_back(texture);
        }
    }

    for (int i = 1; i < 13; ++i) {
        std::string path = "Assets/Images/Plants/Peashooter/peashooter3_frames/frame_" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            if (texture) peaShooterFramesLv3.push_back(texture);
        }
    }
}

void loadZombieFrames(SDL_Renderer* renderer) {
    zombieWalkFrames.clear();
    zombieEatFrames.clear();
    coneZombieWalkFrames.clear();
    coneZombieEatFrames.clear();
    bossZombieWalkFrames.clear();
    bossZombieEatFrames.clear();

    // Load normal zombie frames
    for (int i = 1; i < 46; ++i) {
        std::string path = "Assets/Images/Zombies/ZombieNormal/walk_frames/frame_" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            if (texture) zombieWalkFrames.push_back(texture);
        }
    }

    for (int i = 1; i < 40; ++i) {
        std::string path = "Assets/Images/Zombies/ZombieNormal/eat_frames/frame_" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            if (texture) zombieEatFrames.push_back(texture);
        }
    }

    // Load cone zombie frames
    for (int i = 1; i < 52; ++i) {
        std::string path = "Assets/Images/Zombies/ZombieCone/walk_frames/frame_" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            if (texture) coneZombieWalkFrames.push_back(texture);
        }
    }

    for (int i = 1; i < 62; ++i) {
        std::string path = "Assets/Images/Zombies/ZombieCone/eat_frames/frame_" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            if (texture) coneZombieEatFrames.push_back(texture);
        }
    }

    // Load boss zombie frames
    for (int i = 3; i <= 7; ++i) {
        std::string path = "Assets/Images/Zombies/BossZombie/walk_frames/frame_" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            if (texture) bossZombieWalkFrames.push_back(texture);
        }
    }

    for (int i = 3; i < 15; ++i) {
        std::string path = "Assets/Images/Zombies/BossZombie/attack_frames/frame_" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            if (texture) bossZombieEatFrames.push_back(texture);
        }
    }
}

void loadWalnutFrames(SDL_Renderer* renderer) {
    walnutFrames.clear();
    walnutCrackedFrames1.clear();
    walnutCrackedFrames2.clear();
    walnutCrackedFrames3.clear();

    for (int i = 1; i < 32; ++i) {
        std::string path = "Assets/Images/Plants/Wallnut/normal_frames/frame_" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            if (texture) walnutFrames.push_back(texture);
        }
    }

    for (int i = 1; i < 40; ++i) {
        std::string path = "Assets/Images/Plants/Wallnut/cracked1_frames/frame_" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            if (texture) walnutCrackedFrames1.push_back(texture);
        }
    }

    for (int i = 1; i < 39; ++i) {
        std::string path = "Assets/Images/Plants/Wallnut/cracked2_frames/frame_" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            if (texture) walnutCrackedFrames2.push_back(texture);
        }
    }
}

void loadCherryBombFrames(SDL_Renderer* renderer) {
    cherryBombFrames.clear();
    for (int i = 0; i <= 30; ++i) {
        std::string path = "Assets/Images/Plants/Cherrybomb/cherrybomb_frames/frame_" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            if (texture) cherryBombFrames.push_back(texture);
        }
    }
}

void loadResources(SDL_Renderer* renderer) {
    // Load textures
    loseTexture = FadeEffect::loadTexture("Assets/Images/Background/ZombiesWon.png", renderer);
    sunTexture = FadeEffect::loadTexture("Assets/Images/Background/Sun.png", renderer);
    sunflowerPreview = FadeEffect::loadTexture("Assets/Images/Plants/SunFlower/Sunflower.png", renderer);
    SunFlowerTexture = FadeEffect::loadTexture("Assets/Images/Plants/SunFlower/Sunflower.png", renderer);
    peaShooterPreview = FadeEffect::loadTexture("Assets/Images/Plants/Peashooter/Peashooter.png", renderer);
    shovelPreview = FadeEffect::loadTexture("Assets/Images/Background/Shovel2.png", renderer);
    peaTexture = FadeEffect::loadTexture("Assets/Images/Plants/Peashooter/pea.png", renderer);
    walnutPreview = FadeEffect::loadTexture("Assets/Images/Plants/Wallnut/Walnut.png", renderer);
    cherryBombPreview = FadeEffect::loadTexture("Assets/Images/Plants/Cherrybomb/Cherrybomb.png", renderer);
    CherryBombCardTexture = FadeEffect::loadTexture("Assets/Images/Plants/Cherrybomb/1.png", renderer);

    // Load animation frames
    loadFrames(renderer);
    loadPeaShooterFrames(renderer);
    loadZombieFrames(renderer);
    loadWalnutFrames(renderer);
    loadCherryBombFrames(renderer);

    // Load sounds
    pointSound = Mix_LoadWAV("Assets/Sound/Points.wav");
    zombieEatSound = Mix_LoadWAV("Assets/Sound/ZombieBite.wav");
    shootSound = Mix_LoadWAV("Assets/Sound/shoot.wav");
    loseSound = Mix_LoadWAV("Assets/Sound/lose_sound.wav");
    explosionSound = Mix_LoadWAV("Assets/Sound/explode.wav");
}

void close() {
    // Destroy textures
    for (auto tex : frames) SDL_DestroyTexture(tex);
    for (auto tex : peaShooterFrames) SDL_DestroyTexture(tex);
    for (auto tex : zombieWalkFrames) SDL_DestroyTexture(tex);
    for (auto tex : zombieEatFrames) SDL_DestroyTexture(tex);
    for (auto tex : coneZombieWalkFrames) SDL_DestroyTexture(tex);
    for (auto tex : coneZombieEatFrames) SDL_DestroyTexture(tex);
    for (auto tex : bossZombieWalkFrames) SDL_DestroyTexture(tex);
    for (auto tex : bossZombieEatFrames) SDL_DestroyTexture(tex);
    for (auto tex : cherryBombFrames) SDL_DestroyTexture(tex);
    
    SDL_DestroyTexture(sunflowerPreview);
    SDL_DestroyTexture(SunFlowerTexture);
    SDL_DestroyTexture(peaShooterPreview);
    SDL_DestroyTexture(shovelPreview);
    SDL_DestroyTexture(peaTexture);
    SDL_DestroyTexture(walnutPreview);
    SDL_DestroyTexture(cherryBombPreview);
    SDL_DestroyTexture(CherryBombCardTexture);
    SDL_DestroyTexture(loseTexture);
    SDL_DestroyTexture(sunTexture);

    // Free sounds
    Mix_FreeChunk(pointSound);
    Mix_FreeChunk(zombieEatSound);
    Mix_FreeChunk(shootSound);
    Mix_FreeChunk(loseSound);
    Mix_FreeChunk(explosionSound);

    // Close systems
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    
    // Quit subsystems
    IMG_Quit();
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
}

// ==============================================
// Game Logic Functions
// ==============================================
bool canPlacePlant(int x, int y, int cost, bool isUpgrade = false, bool isSunflower = false) {
    if (!isUpgrade && sun_Count < cost) return false;

    if (x < gameGrid.area.x || x > gameGrid.area.x + gameGrid.area.w ||
        y < gameGrid.area.y || y > gameGrid.area.y + gameGrid.area.h) {
        return false;
    }

    float relativeX = (x - gameGrid.area.x) / static_cast<float>(gameGrid.cellWidth);
    float relativeY = (y - gameGrid.area.y) / static_cast<float>(gameGrid.cellHeight);
    int col = static_cast<int>(relativeX);
    int row = static_cast<int>(relativeY);

    if (col < 0 || col >= gameGrid.cols || row < 0 || row >= gameGrid.rows) {
        return false;
    }
    
    if (isUpgrade) {
        if (isSunflower) {
            for (auto& sunflower : placedSunflowers) {
                if (sunflower.gridRow == row && sunflower.gridCol == col) {
                    return sunflower.level < 2;
                }
            }
        }
        else {
            for (auto& peaShooter : placedPeaShooters) {
                if (peaShooter.gridRow == row && peaShooter.gridCol == col) {
                    return peaShooter.level < 3;
                }
            }
            return false;
        }
    }

    SDL_Rect cellRect = {
        gameGrid.area.x + col * gameGrid.cellWidth + 2,
        gameGrid.area.y + row * gameGrid.cellHeight + 2,
        gameGrid.cellWidth - 4,
        gameGrid.cellHeight - 4
    };

    for (const auto& sunflower : placedSunflowers) {
        if (SDL_HasIntersection(&sunflower.rect, &cellRect)) {
            return false;
        }
    }

    for (const auto& peaShooter : placedPeaShooters) {
        if (SDL_HasIntersection(&peaShooter.rect, &cellRect)) {
            return false;
        }
    }

    for (const auto& walnut : placedWalnuts) {
        if (SDL_HasIntersection(&walnut.rect, &cellRect)) {
            return false;
        }
    }

    return true;
}

void produceSunFromSunflowers(SDL_Renderer* renderer) {
    Uint32 currentTime = SDL_GetTicks();

    for (auto& sunflower : placedSunflowers) {
        if (currentTime - sunflower.lastProducedTime > 10000) {
            Sun sun1;
            sun1.texture = sunTexture;
            sun1.rect.w = 62;
            sun1.rect.h = 62;
            sun1.rect.x = sunflower.rect.x + (sunflower.rect.w - sun1.rect.w) / 2;
            sun1.rect.y = sunflower.rect.y + sunflower.rect.h;
            sun1.targetY = sunflower.rect.y + sunflower.rect.h;
            sun1.collected = false;
            sun1.movingToSunBar = false;
            sun1.targetX = 10;
            sun1.targetFinalY = 0;
            sun1.soundPlayed = false;
            suns.push_back(sun1);
            
            if (sunflower.level == 2) {
                Sun sun2;
                sun2.texture = sunTexture;
                sun2.rect.w = 62;
                sun2.rect.h = 62;
                sun2.rect.x = sunflower.rect.x + (sunflower.rect.w - sun2.rect.w) / 2 + 50;
                sun2.rect.y = sunflower.rect.y + sunflower.rect.h;
                sun2.targetY = sunflower.rect.y + sunflower.rect.h;
                sun2.collected = false;
                sun2.movingToSunBar = false;
                sun2.targetX = 10;
                sun2.targetFinalY = 0;
                sun2.soundPlayed = false;
                suns.push_back(sun1);
            }
            sunflower.lastProducedTime = currentTime;
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
                    sun_Count += 50;
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

void updateCooldown() {
    if (onCooldownSunflower) {
        cooldownTimerSunflower--;
        if (cooldownTimerSunflower <= 0) {
            onCooldownSunflower = false;
            cooldownTimerSunflower = 0;
        }
    }

    if (onCooldownPeaShooter) {
        cooldownTimerPeaShooter--;
        if (cooldownTimerPeaShooter <= 0) {
            onCooldownPeaShooter = false;
            cooldownTimerPeaShooter = 0;
        }
    }

    if (onCooldownCherryBomb) {
        cooldownTimerCherryBomb--;
        if (cooldownTimerCherryBomb <= 0) {
            onCooldownCherryBomb = false;
            cooldownTimerCherryBomb = 0;
        }
    }

    if (onCooldownWalnut) {
        cooldownTimerWalnut--;
        if (cooldownTimerWalnut <= 0) {
            onCooldownWalnut = false;
            cooldownTimerWalnut = 0;
        }
    }
}

void spawnZombie(int windowWidth, int windowHeight, ZombieType type = ZOMBIE_NORMAL) {
    Zombie newZombie;
    int row = rand() % gameGrid.rows;

    newZombie.rect = {
        windowWidth,
        gameGrid.area.y + row * gameGrid.cellHeight - 30,
        static_cast<int>(gameGrid.cellWidth * 1.8f),
        static_cast<int>(gameGrid.cellHeight * 1.6f)
    };

    newZombie.originalSize = newZombie.rect;
    newZombie.gridRow = row;
    newZombie.type = type;

    if (type == ZOMBIE_NORMAL) {
        newZombie.health = 100;
    }
    else if (type == ZOMBIE_CONE) {
        newZombie.health = 200;
    }

    newZombie.currentFrame = 0;
    newZombie.lastFrameTime = SDL_GetTicks();
    newZombie.active = true;
    newZombie.state = ZOMBIE_WALKING;
    newZombie.lastEatTime = 0;
    newZombie.targetPlantIndex = -1;
    newZombie.targetIsSunflower = false;
    zombies.push_back(newZombie);
}

void spawnBossZombie(int windowWidth, int windowHeight) {
    Zombie boss;
    int row = rand() % gameGrid.rows;

    boss.rect = {
        windowWidth,
        gameGrid.area.y + row * gameGrid.cellHeight - 30,
        static_cast<int>(gameGrid.cellWidth * 2.0f),
        static_cast<int>(gameGrid.cellHeight * 2.0f)
    };

    boss.originalSize = boss.rect;
    boss.gridRow = row;
    boss.type = ZOMBIE_BOSS;
    boss.health = 9999;
    boss.currentFrame = 0;
    boss.lastFrameTime = SDL_GetTicks();
    boss.active = true;
    boss.state = ZOMBIE_WALKING;
    boss.lastEatTime = 0;
    boss.targetPlantIndex = -1;
    boss.targetIsSunflower = false;

    zombies.push_back(boss);
}

bool isZombieCollidingWithPlant(const Zombie& zombie, const SDL_Rect& plantRect) {
    SDL_Rect zombieCollider = zombie.rect;
    zombieCollider.x += 100;
    zombieCollider.w -= 20;

    return SDL_HasIntersection(&zombieCollider, &plantRect);
}

void updateZombies(int windowWidth, int windowHeight) {
    Uint32 currentTime = SDL_GetTicks();

    if (gameLost) return;

    if (currentTime - gameStartTime > 60000 &&
        std::none_of(zombies.begin(), zombies.end(),
            [](const Zombie& z) { return z.type == ZOMBIE_BOSS; })) {
        spawnBossZombie(windowWidth, windowHeight);
    }

    if (currentTime - gameStartTime > ZOMBIE_SPAWN_TIME &&
        (currentTime - lastZombieSpawnTime > 10000 || zombies.empty())) {
        for (int i = 0; i < 3; i++) {
            if (rand() % 10 < 3) {
                spawnZombie(windowWidth, windowHeight, ZOMBIE_CONE);
            }
            else {
                spawnZombie(windowWidth, windowHeight, ZOMBIE_NORMAL);
            }
        }
        lastZombieSpawnTime = currentTime;
    }

    for (auto& zombie : zombies) {
        if (!zombie.active) continue;

        zombie.rect.y = gameGrid.area.y + zombie.gridRow * gameGrid.cellHeight - 50;
        zombie.rect.w = zombie.originalSize.w;
        zombie.rect.h = zombie.originalSize.h;
        int speed = (zombie.type == ZOMBIE_BOSS) ? ZOMBIE_SPEED / 10 : ZOMBIE_SPEED;

        if (zombie.state == ZOMBIE_WALKING) {
            if (currentTime > zombie.lastFrameTime + ZOMBIE_FRAME_DELAY) {
                zombie.rect.x -= ZOMBIE_SPEED;
                zombie.currentFrame = (zombie.currentFrame + 1) % zombieWalkFrames.size();
                zombie.lastFrameTime = currentTime;
            }

            // Check for Wallnut first
            for (size_t i = 0; i < placedWalnuts.size(); i++) {
                auto& walnut = placedWalnuts[i];
                if (walnut.gridRow == zombie.gridRow && isZombieCollidingWithPlant(zombie, walnut.rect)) {
                    zombie.state = ZOMBIE_EATING;
                    zombie.targetPlantIndex = static_cast<int>(i);
                    zombie.targetIsSunflower = false;
                    break;
                }
            }

            // Then check PeaShooters
            if (zombie.state == ZOMBIE_WALKING) {
                for (size_t i = 0; i < placedPeaShooters.size(); i++) {
                    auto& plant = placedPeaShooters[i];
                    if (plant.gridRow == zombie.gridRow && isZombieCollidingWithPlant(zombie, plant.rect)) {
                        zombie.state = ZOMBIE_EATING;
                        zombie.targetPlantIndex = static_cast<int>(i);
                        zombie.targetIsSunflower = false;
                        break;
                    }
                }
            }

            // Finally check Sunflowers
            if (zombie.state == ZOMBIE_WALKING) {
                for (size_t i = 0; i < placedSunflowers.size(); i++) {
                    auto& sunflower = placedSunflowers[i];
                    if (sunflower.gridRow == zombie.gridRow && isZombieCollidingWithPlant(zombie, sunflower.rect)) {
                        zombie.state = ZOMBIE_EATING;
                        zombie.targetPlantIndex = static_cast<int>(i);
                        zombie.targetIsSunflower = true;
                        break;
                    }
                }
            }
        }
        else if (zombie.state == ZOMBIE_EATING) {
            if (currentTime > zombie.lastFrameTime + ZOMBIE_FRAME_DELAY) {
                zombie.currentFrame = (zombie.currentFrame + 1) % zombieEatFrames.size();
                zombie.lastFrameTime = currentTime;

                // Kiểm tra xem cây mục tiêu có còn tồn tại và zombie có còn va chạm với nó không
                bool shouldContinueEating = false;
                SDL_Rect targetRect;

                if (!zombie.targetIsSunflower) {
                    if (zombie.targetPlantIndex < placedWalnuts.size()) {
                        auto& target = placedWalnuts[zombie.targetPlantIndex];
                        targetRect = target.rect;
                        if (target.gridRow == zombie.gridRow && isZombieCollidingWithPlant(zombie, targetRect)) {
                            shouldContinueEating = true;
                        }
                    }
                    else if (zombie.targetPlantIndex < placedPeaShooters.size()) {
                        auto& target = placedPeaShooters[zombie.targetPlantIndex];
                        targetRect = target.rect;
                        if (target.gridRow == zombie.gridRow && isZombieCollidingWithPlant(zombie, targetRect)) {
                            shouldContinueEating = true;
                        }
                    }
                }
                else {
                    if (zombie.targetPlantIndex < placedSunflowers.size()) {
                        auto& target = placedSunflowers[zombie.targetPlantIndex];
                        targetRect = target.rect;
                        if (target.gridRow == zombie.gridRow && isZombieCollidingWithPlant(zombie, targetRect)) {
                            shouldContinueEating = true;
                        }
                    }
                }

                if (!shouldContinueEating) {
                    zombie.state = ZOMBIE_WALKING;
                    zombie.targetPlantIndex = -1;
                    continue; // Chuyển sang WALKING và bỏ qua phần ăn cây
                }

                // Tiếp tục ăn cây nếu vẫn còn va chạm
                if (currentTime - zombie.lastEatTime > 1000) {
                    int damage = 10;
                    if (zombie.type == ZOMBIE_CONE) {
                        damage = 20;
                    }
                    else if (zombie.type == ZOMBIE_BOSS) {
                        damage = 9999;
                    }

                    if (!zombie.targetIsSunflower && zombie.targetPlantIndex < placedWalnuts.size()) {
                        placedWalnuts[zombie.targetPlantIndex].health -= damage;
                        zombie.lastEatTime = currentTime;

                        if (zombieEatSound) Mix_PlayChannel(-1, zombieEatSound, 0);

                        if (placedWalnuts[zombie.targetPlantIndex].health <= 0) {
                            placedWalnuts.erase(placedWalnuts.begin() + zombie.targetPlantIndex);
                            zombie.state = ZOMBIE_WALKING;
                            zombie.targetPlantIndex = -1;
                        }
                    }
                    else if (!zombie.targetIsSunflower && zombie.targetPlantIndex < placedPeaShooters.size()) {
                        placedPeaShooters[zombie.targetPlantIndex].health -= damage;
                        zombie.lastEatTime = currentTime;

                        if (zombieEatSound) Mix_PlayChannel(-1, zombieEatSound, 0);

                        if (placedPeaShooters[zombie.targetPlantIndex].health <= 0) {
                            placedPeaShooters.erase(placedPeaShooters.begin() + zombie.targetPlantIndex);
                            zombie.state = ZOMBIE_WALKING;
                            zombie.targetPlantIndex = -1;
                        }
                    }
                    else if (zombie.targetIsSunflower && zombie.targetPlantIndex < placedSunflowers.size()) {
                        placedSunflowers[zombie.targetPlantIndex].health -= damage;
                        zombie.lastEatTime = currentTime;

                        if (zombieEatSound) Mix_PlayChannel(-1, zombieEatSound, 0);

                        if (placedSunflowers[zombie.targetPlantIndex].health <= 0) {
                            placedSunflowers.erase(placedSunflowers.begin() + zombie.targetPlantIndex);
                            zombie.state = ZOMBIE_WALKING;
                            zombie.targetPlantIndex = -1;
                        }
                    }
                    else {
                        zombie.state = ZOMBIE_WALKING;
                        zombie.targetPlantIndex = -1;
                    }
                }
            }
        }

        if (zombie.rect.x + zombie.rect.w < 0 && !gameLost) {
            gameLost = true;
            loseTime = currentTime;
            if (loseSound) {
                Mix_PlayChannel(-1, loseSound, 0);
            }
            Mix_HaltMusic();
            FadeEffect::LoseScreen(renderer, loseTexture, windowWidth, windowHeight);
            break;
        }
    }

    zombies.erase(std::remove_if(zombies.begin(), zombies.end(),
        [](const Zombie& z) { return !z.active; }), zombies.end());
}

void shootPea(Plant& peaShooter) {
    Uint32 currentTime = SDL_GetTicks();
    static std::map<Plant*, Uint32> lastShootTimes;

    bool zombieInRange = false;
    for (auto& zombie : zombies) {
        if (zombie.active &&
            zombie.gridRow == peaShooter.gridRow &&
            zombie.rect.x > peaShooter.rect.x &&
            zombie.rect.x < peaShooter.rect.x + gameGrid.cellWidth * 8) {
            zombieInRange = true;
            break;
        }
    }

    if (zombieInRange) {
        if (lastShootTimes.find(&peaShooter) == lastShootTimes.end() ||
            currentTime - lastShootTimes[&peaShooter] > PEA_SPAWN_DELAY) {

            for (int i = 0; i < peaShooter.level; i++) {
                Pea newPea;
                newPea.rect = {
                    peaShooter.rect.x + peaShooter.rect.w - 30,
                    peaShooter.rect.y + peaShooter.rect.h / 4 + (i * 10) - ((peaShooter.level - 1) * 5),
                    30, 30
                };
                newPea.speed = PEA_SPEED;
                newPea.damage = PEA_DAMAGE;
                newPea.row = peaShooter.gridRow;
                newPea.active = true;
                peas.push_back(newPea);
            }

            if (shootSound) {
                Mix_PlayChannel(-1, shootSound, 0);
            }

            lastShootTimes[&peaShooter] = currentTime;
        }
    }
}

void updatePeas(int windowWidth) {
    for (auto it = peas.begin(); it != peas.end(); ) {
        if (it->active) {
            it->rect.x += it->speed;

            bool hit = false;
            for (auto& zombie : zombies) {
                if (zombie.active &&
                    zombie.gridRow == it->row &&
                    SDL_HasIntersection(&it->rect, &zombie.rect)) {

                    zombie.health -= it->damage;
                    hit = true;

                    if (zombie.health <= 0) {
                        zombie.active = false;
                    }
                    break;
                }
            }

            if (hit) {
                it = peas.erase(it);
            }
            else {
                ++it;
            }
        }
        else {
            ++it;
        }
    }
}

void updateCherryBombs() {
    Uint32 currentTime = SDL_GetTicks();

    for (auto it = placedCherryBombs.begin(); it != placedCherryBombs.end(); ) {
        if (!it->exploded && currentTime - it->plantTime > 1500) {
            it->exploded = true;

            if (explosionSound) {
                Mix_PlayChannel(-1, explosionSound, 0);
            }

            for (auto& zombie : zombies) {
                if (zombie.active) {
                    int distanceX = abs((it->rect.x + it->rect.w / 2) - (zombie.rect.x + zombie.rect.w / 2));
                    int distanceY = abs((it->rect.y + it->rect.h / 2) - (zombie.rect.y + zombie.rect.h / 2));

                    if (distanceX < gameGrid.cellWidth * 1.5 &&
                        distanceY < gameGrid.cellHeight * 1.5) {
                        zombie.health -= 500;

                        if (zombie.health <= 0) {
                            zombie.active = false;
                        }
                    }
                }
            }

            it = placedCherryBombs.erase(it);
        }
        else {
            ++it;
        }
    }
}

// ==============================================
// Rendering Functions
// ==============================================
void renderSunCount(SDL_Renderer* renderer) {
    std::string sunText = std::to_string(sun_Count);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, sunText.c_str(), textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = { 50, 10, textSurface->w, textSurface->h };
    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void renderSuns(SDL_Renderer* renderer) {
    for (const auto& sun : suns) {
        if (!sun.collected) {
            SDL_RenderCopy(renderer, sun.texture, nullptr, &sun.rect);
        }
    }
}

void renderSunflowers(SDL_Renderer* renderer) {
    if (placedSunflowers.empty()) return;

    Uint32 currentTime = SDL_GetTicks();
    static Uint32 lastFrameTime = 0;
    static int currentFrame = 0;

    if (currentTime > lastFrameTime + SUNFLOWER_FRAME_DELAY) {
        currentFrame = (currentFrame + 1) % frames.size();
        lastFrameTime = currentTime;
    }

    for (const auto& sunflower : placedSunflowers) {
        if (sunflower.level == 2 && !sunflowerFramesLv2.empty()) {
            SDL_RenderCopy(renderer, sunflowerFramesLv2[currentFrame % sunflowerFramesLv2.size()], nullptr, &sunflower.rect);
        }
        else {
            SDL_RenderCopy(renderer, frames[currentFrame], nullptr, &sunflower.rect);
        }
    }
}

void renderPeaShooters(SDL_Renderer* renderer) {
    if (placedPeaShooters.empty()) return;

    Uint32 currentTime = SDL_GetTicks();
    if (currentTime > peaShooterLastFrameTime + PEASHOOTER_FRAME_DELAY) {
        peaShooterCurrentFrame = (peaShooterCurrentFrame + 1) % peaShooterFrames.size();
        peaShooterLastFrameTime = currentTime;
    }

    for (const auto& plant : placedPeaShooters) {
        if (plant.level == 3 && !peaShooterFramesLv3.empty()) {
            SDL_RenderCopy(renderer, peaShooterFramesLv3[peaShooterCurrentFrame % peaShooterFramesLv3.size()], nullptr, &plant.rect);
        }
        else if (plant.level == 2 && !peaShooterFramesLv2.empty()) {
            SDL_RenderCopy(renderer, peaShooterFramesLv2[peaShooterCurrentFrame % peaShooterFramesLv2.size()], nullptr, &plant.rect);
        }
        else {
            SDL_RenderCopy(renderer, peaShooterFrames[peaShooterCurrentFrame], nullptr, &plant.rect);
        }
    }
}

void renderPeas(SDL_Renderer* renderer) {
    for (const auto& pea : peas) {
        if (pea.active && peaTexture) {
            SDL_RenderCopy(renderer, peaTexture, nullptr, &pea.rect);
        }
    }
}

void renderZombies(SDL_Renderer* renderer) {
    for (const auto& zombie : zombies) {
        if (!zombie.active) continue;

        SDL_Texture* textureToRender = nullptr;
        SDL_Rect renderRect = zombie.originalSize;
        renderRect.x = zombie.rect.x;
        renderRect.y = zombie.rect.y;

        if (zombie.type == ZOMBIE_NORMAL) {
            if (zombie.state == ZOMBIE_WALKING) {
                textureToRender = zombieWalkFrames[zombie.currentFrame % zombieWalkFrames.size()];
            }
            else {
                textureToRender = zombieEatFrames[zombie.currentFrame % zombieEatFrames.size()];
            }
        }
        else if (zombie.type == ZOMBIE_CONE) {
            if (zombie.state == ZOMBIE_WALKING) {
                textureToRender = coneZombieWalkFrames[zombie.currentFrame % coneZombieWalkFrames.size()];
            }
            else {
                textureToRender = coneZombieEatFrames[zombie.currentFrame % coneZombieEatFrames.size()];
            }
        }
        else if (zombie.type == ZOMBIE_BOSS) {
            if (zombie.state == ZOMBIE_WALKING && !bossZombieWalkFrames.empty()) {
                textureToRender = bossZombieWalkFrames[zombie.currentFrame % bossZombieWalkFrames.size()];
            }
            else if (!bossZombieEatFrames.empty()) {
                textureToRender = bossZombieEatFrames[zombie.currentFrame % bossZombieEatFrames.size()];
            }
        }

        if (textureToRender) {
            SDL_RenderCopy(renderer, textureToRender, nullptr, &renderRect);
        }
    }
}

void renderWalnuts(SDL_Renderer* renderer) {
    if (placedWalnuts.empty()) return;

    Uint32 currentTime = SDL_GetTicks();
    static Uint32 lastWalnutFrameTime = 0;
    static int currentWalnutFrame = 0;

    if (currentTime > lastWalnutFrameTime + WALNUT_FRAME_DELAY) {
        currentWalnutFrame = (currentWalnutFrame + 1) % walnutFrames.size();
        lastWalnutFrameTime = currentTime;
    }

    for (auto& walnut : placedWalnuts) {
        SDL_Texture* textureToRender = nullptr;

        if (walnut.health > WALNUT_HEALTH * 0.66) {
            textureToRender = walnutFrames[currentWalnutFrame % walnutFrames.size()];
        }
        else if (walnut.health > WALNUT_HEALTH * 0.33) {
            textureToRender = walnutCrackedFrames1[currentWalnutFrame % walnutCrackedFrames1.size()];
        }
        else {
            textureToRender = walnutCrackedFrames2[currentWalnutFrame % walnutCrackedFrames2.size()];
        }

        if (textureToRender) {
            SDL_RenderCopy(renderer, textureToRender, nullptr, &walnut.rect);
        }
    }
}

void renderCherryBombs(SDL_Renderer* renderer) {
    if (placedCherryBombs.empty()) return;

    Uint32 currentTime = SDL_GetTicks();
    static Uint32 lastFrameTime = 0;
    static int currentFrame = 0;

    if (currentTime > lastFrameTime + 50) {
        currentFrame = (currentFrame + 1) % cherryBombFrames.size();
        lastFrameTime = currentTime;
    }

    for (const auto& cherryBomb : placedCherryBombs) {
        if (!cherryBomb.exploded && !cherryBombFrames.empty()) {
            SDL_RenderCopy(renderer, cherryBombFrames[currentFrame], nullptr, &cherryBomb.rect);
        }
    }
}

void renderGrid(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 100);
    SDL_RenderDrawRect(renderer, &gameGrid.area);

    for (int c = 1; c < gameGrid.cols; c++) {
        SDL_RenderDrawLine(renderer,
            gameGrid.area.x + c * gameGrid.cellWidth,
            gameGrid.area.y,
            gameGrid.area.x + c * gameGrid.cellWidth,
            gameGrid.area.y + gameGrid.area.h);
    }

    for (int r = 1; r < gameGrid.rows; r++) {
        SDL_RenderDrawLine(renderer,
            gameGrid.area.x,
            gameGrid.area.y + r * gameGrid.cellHeight,
            gameGrid.area.x + gameGrid.area.w,
            gameGrid.area.y + r * gameGrid.cellHeight);
    }
}

void renderLoseScreen(SDL_Renderer* renderer, int windowWidth, int windowHeight) {
    if (!gameLost || !loseTexture) return;

    SDL_Rect fullScreenRect = { 0, 0, windowWidth, windowHeight };
    SDL_RenderCopy(renderer, loseTexture, nullptr, &fullScreenRect);
}

// ==============================================
// Input Handling
// ==============================================
void handleMouseEvent(SDL_Event& e, SDL_Rect& SunFlowerCardRect, SDL_Rect& PeaShooterCardRect, 
                     SDL_Rect& CherryBombCardRect, SDL_Rect& WalnutCardRect, SDL_Rect& ShovelRect) {
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
        else if (SDL_PointInRect(&mousePos, &WalnutCardRect) && !onCooldownWalnut) {
            isDraggingWalnut = true;
            draggingWalnutRect = WalnutCardRect;
            previewingWalnut = true;
        }
        else if (SDL_PointInRect(&mousePos, &CherryBombCardRect) && !onCooldownCherryBomb) {
            isDraggingCherryBomb = true;
            draggingCherryBombRect = CherryBombCardRect;
            previewingCherryBomb = true;
        }
        else if (SDL_PointInRect(&mousePos, &ShovelRect)) {
            isDraggingShovel = true;
            draggingShovelRect = ShovelRect;
            previewingShovel = true;
        }
        else {
            collectSun(e.button.x, e.button.y);
        }
    }

    if (e.type == SDL_MOUSEBUTTONUP) {
        bool placedSuccessfully = false;
        int plantSize = static_cast<int>(gameGrid.cellWidth * 0.9f);

        if (isDragging) {
            for (int row = 0; row < gameGrid.rows; row++) {
                for (int col = 0; col < gameGrid.cols; col++) {
                    SDL_Rect cellRect = {
                        gameGrid.area.x + col * gameGrid.cellWidth,
                        gameGrid.area.y + row * gameGrid.cellHeight,
                        gameGrid.cellWidth,
                        gameGrid.cellHeight
                    };

                    if (e.button.x >= cellRect.x && e.button.x <= cellRect.x + cellRect.w &&
                        e.button.y >= cellRect.y && e.button.y <= cellRect.y + cellRect.h) {

                        bool hasSunflower = false;
                        for (auto& sunflower : placedSunflowers) {
                            if (sunflower.gridRow == row && sunflower.gridCol == col) {
                                hasSunflower = true;
                                break;
                            }
                        }

                        if (hasSunflower && canPlacePlant(e.button.x, e.button.y, 50, true, true) && !onCooldownSunflower) {
                            for (auto& sunflower : placedSunflowers) {
                                if (sunflower.gridRow == row && sunflower.gridCol == col && sunflower.level < 2) {
                                    sunflower.level++;
                                    sun_Count -= 50;
                                    onCooldownSunflower = true;
                                    cooldownTimerSunflower = cooldownDuration;
                                    placedSuccessfully = true;
                                    break;
                                }
                            }
                        }
                        else if (!hasSunflower && canPlacePlant(e.button.x, e.button.y, 50) && !onCooldownSunflower) {
                            sun_Count -= 50;
                            Sunflower newSunflower;
                            newSunflower.rect = {
                                gameGrid.area.x + col * gameGrid.cellWidth + (gameGrid.cellWidth - plantSize) / 2,
                                gameGrid.area.y + row * gameGrid.cellHeight + (gameGrid.cellHeight - plantSize) / 2,
                                plantSize,
                                plantSize
                            };
                            newSunflower.gridRow = row;
                            newSunflower.gridCol = col;
                            newSunflower.lastProducedTime = SDL_GetTicks();
                            newSunflower.level = 1;
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
                previewingSunflower = false;
            }
            isDragging = false;
        }
        else if (isDraggingPeaShooter) {
            for (int row = 0; row < gameGrid.rows; row++) {
                for (int col = 0; col < gameGrid.cols; col++) {
                    SDL_Rect cellRect = {
                        gameGrid.area.x + col * gameGrid.cellWidth,
                        gameGrid.area.y + row * gameGrid.cellHeight,
                        gameGrid.cellWidth,
                        gameGrid.cellHeight
                    };

                    if (e.button.x >= cellRect.x && e.button.x <= cellRect.x + cellRect.w &&
                        e.button.y >= cellRect.y && e.button.y <= cellRect.y + cellRect.h) {
                        bool isUpgrade = false;
                        for (auto& plant : placedPeaShooters) {
                            if (plant.gridRow == row && plant.gridCol == col) {
                                isUpgrade = true;
                                break;
                            }
                        }

                        if ((isUpgrade && canPlacePlant(e.button.x, e.button.y, 100, true) && !onCooldownPeaShooter) ||
                            (!isUpgrade && canPlacePlant(e.button.x, e.button.y, 100) && !onCooldownPeaShooter)) {

                            if (isUpgrade) {
                                for (auto& plant : placedPeaShooters) {
                                    if (plant.gridRow == row && plant.gridCol == col) {
                                        plant.level++;
                                        break;
                                    }
                                }
                            }
                            else {
                                sun_Count -= 100;
                                Plant newPeaShooter;
                                newPeaShooter.rect = {
                                    gameGrid.area.x + col * gameGrid.cellWidth + (gameGrid.cellWidth - plantSize) / 2,
                                    gameGrid.area.y + row * gameGrid.cellHeight + (gameGrid.cellHeight - plantSize) / 2,
                                    plantSize,
                                    plantSize
                                };
                                newPeaShooter.gridRow = row;
                                newPeaShooter.gridCol = col;
                                placedPeaShooters.push_back(newPeaShooter);
                            }
                            onCooldownPeaShooter = true;
                            cooldownTimerPeaShooter = cooldownDuration;
                            placedSuccessfully = true;
                        }
                        break;
                    }
                }
            }
            if (!placedSuccessfully) {
                previewingPeaShooter = false;
            }
            isDraggingPeaShooter = false;
        }
        else if (isDraggingCherryBomb) {
            bool placedSuccessfully = false;
            int plantSize = static_cast<int>(gameGrid.cellWidth * 0.9f);

            for (int row = 0; row < gameGrid.rows; row++) {
                for (int col = 0; col < gameGrid.cols; col++) {
                    SDL_Rect cellRect = {
                        gameGrid.area.x + col * gameGrid.cellWidth,
                        gameGrid.area.y + row * gameGrid.cellHeight,
                        gameGrid.cellWidth,
                        gameGrid.cellHeight
                    };

                    if (e.button.x >= cellRect.x && e.button.x <= cellRect.x + cellRect.w &&
                        e.button.y >= cellRect.y && e.button.y <= cellRect.y + cellRect.h) {

                        if (canPlacePlant(e.button.x, e.button.y, CHERRYBOMB_COST) && !onCooldownCherryBomb) {
                            sun_Count -= CHERRYBOMB_COST;
                            CherryBomb newCherryBomb;
                            newCherryBomb.rect = {
                                gameGrid.area.x + col * gameGrid.cellWidth + (gameGrid.cellWidth - plantSize) / 2,
                                gameGrid.area.y + row * gameGrid.cellHeight + (gameGrid.cellHeight - plantSize) / 2,
                                plantSize,
                                plantSize
                            };
                            newCherryBomb.gridRow = row;
                            newCherryBomb.gridCol = col;
                            newCherryBomb.plantTime = SDL_GetTicks();
                            placedCherryBombs.push_back(newCherryBomb);
                            onCooldownCherryBomb = true;
                            cooldownTimerCherryBomb = cooldownDuration;
                            placedSuccessfully = true;
                            cooldownTimerCherryBomb = CHERRYBOMB_COOLDOWN_DURATION;
                        }
                        break;
                    }
                }
            }
            if (!placedSuccessfully) {
                previewingCherryBomb = false;
            }
            isDraggingCherryBomb = false;
        }
        else if (isDraggingWalnut) {
            for (int row = 0; row < gameGrid.rows; row++) {
                for (int col = 0; col < gameGrid.cols; col++) {
                    SDL_Rect cellRect = {
                        gameGrid.area.x + col * gameGrid.cellWidth,
                        gameGrid.area.y + row * gameGrid.cellHeight,
                        gameGrid.cellWidth,
                        gameGrid.cellHeight
                    };

                    if (e.button.x >= cellRect.x && e.button.x <= cellRect.x + cellRect.w &&
                        e.button.y >= cellRect.y && e.button.y <= cellRect.y + cellRect.h) {

                        if (canPlacePlant(e.button.x, e.button.y, WALNUT_COST) && !onCooldownWalnut) {
                            sun_Count -= WALNUT_COST;
                            Walnut newWalnut;
                            newWalnut.rect = {
                                gameGrid.area.x + col * gameGrid.cellWidth + (gameGrid.cellWidth - plantSize) / 2,
                                gameGrid.area.y + row * gameGrid.cellHeight + (gameGrid.cellHeight - plantSize) / 2,
                                plantSize,
                                plantSize
                            };
                            newWalnut.gridRow = row;
                            newWalnut.gridCol = col;
                            newWalnut.health = WALNUT_HEALTH;
                            placedWalnuts.push_back(newWalnut);
                            onCooldownWalnut = true;
                            cooldownTimerWalnut = cooldownDuration;
                            placedSuccessfully = true;
                        }
                        break;
                    }
                }
            }
            if (!placedSuccessfully) {
                previewingWalnut = false;
            }
            isDraggingWalnut = false;
        }
        else if (isDraggingShovel) {
            for (int row = 0; row < gameGrid.rows; row++) {
                for (int col = 0; col < gameGrid.cols; col++) {
                    SDL_Rect cellRect = {
                        gameGrid.area.x + col * gameGrid.cellWidth,
                        gameGrid.area.y + row * gameGrid.cellHeight,
                        gameGrid.cellWidth,
                        gameGrid.cellHeight
                    };
                    if (e.button.x >= cellRect.x && e.button.x <= cellRect.x + cellRect.w &&
                        e.button.y >= cellRect.y && e.button.y <= cellRect.y + cellRect.h) {
                        for (auto it = placedPeaShooters.begin(); it != placedPeaShooters.end(); ) {
                            if (it->gridRow == row && it->gridCol == col) {
                                it = placedPeaShooters.erase(it);
                                break;
                            }
                            else {
                                ++it;
                            }
                        }
                        for (auto it = placedSunflowers.begin(); it != placedSunflowers.end(); ) {
                            if (it->gridRow == row && it->gridCol == col) {
                                it = placedSunflowers.erase(it);
                                break;
                            }
                            else {
                                ++it;
                            }
                        }
                        for (auto it = placedWalnuts.begin(); it != placedWalnuts.end(); ) {
                            if (it->gridRow == row && it->gridCol == col) {
                                it = placedWalnuts.erase(it);
                                break;
                            }
                            else {
                                ++it;
                            }
                        }
                        previewingShovel = false;
                    }
                }
            }
            isDraggingShovel = false;
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
        else if (isDraggingWalnut) {
            draggingWalnutRect.x = e.motion.x - draggingWalnutRect.w / 2;
            draggingWalnutRect.y = e.motion.y - draggingWalnutRect.h / 2;
        }
        else if (isDraggingCherryBomb) {
            draggingCherryBombRect.x = e.motion.x - draggingCherryBombRect.w / 2;
            draggingCherryBombRect.y = e.motion.y - draggingCherryBombRect.h / 2;
        }
        else if (isDraggingShovel) {
            draggingShovelRect.x = e.motion.x - draggingShovelRect.w / 2;
            draggingShovelRect.y = e.motion.y - draggingShovelRect.h / 2;
        }
    }
}

// ==============================================
// Preview Rendering Functions
// ==============================================
void renderSunflowerPreview(SDL_Renderer* renderer) {
    if (previewingSunflower && isDragging) {
        SDL_RenderCopy(renderer, sunflowerPreview, nullptr, &draggingRect);
    }
}

void renderPeaShooterPreview(SDL_Renderer* renderer) {
    if (previewingPeaShooter && isDraggingPeaShooter) {
        SDL_RenderCopy(renderer, peaShooterPreview, nullptr, &draggingPeaShooterRect);
    }
}

void renderWalnutPreview(SDL_Renderer* renderer) {
    if (previewingWalnut && isDraggingWalnut) {
        SDL_RenderCopy(renderer, walnutPreview, nullptr, &draggingWalnutRect);
    }
}

void renderCherryBombPreview(SDL_Renderer* renderer) {
    if (previewingCherryBomb && isDraggingCherryBomb) {
        SDL_RenderCopy(renderer, cherryBombPreview, nullptr, &draggingCherryBombRect);
    }
}

void renderShovelPreview(SDL_Renderer* renderer) {
    if (previewingShovel && isDraggingShovel) {
        SDL_RenderCopy(renderer, shovelPreview, nullptr, &draggingShovelRect);
    }
}

// ==============================================
// Card Rendering Functions
// ==============================================
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

void renderCherryBombCard(SDL_Renderer* renderer, SDL_Texture* CherryBombCardTexture, SDL_Rect& CherryBombCardRect) {
    if (isDraggingCherryBomb) {
        SDL_RenderCopy(renderer, cherryBombPreview, nullptr, &draggingCherryBombRect);
    }
    else {
        if (onCooldownCherryBomb) {
            SDL_SetTextureColorMod(CherryBombCardTexture, 128, 128, 128);
        }
        SDL_RenderCopy(renderer, CherryBombCardTexture, nullptr, &CherryBombCardRect);
        if (onCooldownCherryBomb) {
            SDL_SetTextureColorMod(CherryBombCardTexture, 255, 255, 255);
        }
    }
}

void renderWalnutCard(SDL_Renderer* renderer, SDL_Texture* WalnutCardTexture, SDL_Rect& WalnutCardRect) {
    if (isDraggingWalnut) {
        SDL_RenderCopy(renderer, walnutPreview, nullptr, &draggingWalnutRect);
    }
    else {
        if (onCooldownWalnut) {
            SDL_SetTextureColorMod(WalnutCardTexture, 128, 128, 128);
        }
        SDL_RenderCopy(renderer, WalnutCardTexture, nullptr, &WalnutCardRect);
        if (onCooldownWalnut) {
            SDL_SetTextureColorMod(WalnutCardTexture, 255, 255, 255);
        }
    }
}

// ==============================================
// Main Function
// ==============================================
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

    window = SDL_CreateWindow("Persist to quit", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
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

    // Load splash screen
    SDL_Texture* splashTexture = FadeEffect::loadTexture("Assets/Images/Background/begin.png", renderer);
    if (splashTexture) {
        FadeEffect::fadeIn(renderer, splashTexture, 1000);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, splashTexture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
        SDL_Delay(1000);
        FadeEffect::fadeOut(renderer, splashTexture, 1000);
        SDL_DestroyTexture(splashTexture);
    }

    IMG_Init(IMG_INIT_PNG);
    loadResources(renderer);

    // Load main menu music
    Mix_Music* mainMenuMusic = Mix_LoadMUS("Assets/Sound/day_soundtrack.wav");
    if (mainMenuMusic) Mix_PlayMusic(mainMenuMusic, -1);

    bool running = true;
    SDL_Event e;
    int windowWidth = SCREEN_WIDTH;
    int windowHeight = SCREEN_HEIGHT;
    initGrid(windowWidth, windowHeight);

    // Load main menu textures
    SDL_Texture* mainMenuTexture = FadeEffect::loadTexture("Assets/Images/Background/background1a.png", renderer);
    SDL_Texture* itemBarTexture = FadeEffect::loadTexture("Assets/Images/Background/Item_Bar.png", renderer);
    SDL_Texture* sunBarTexture = FadeEffect::loadTexture("Assets/Images/Background/sun_bar.png", renderer);
    SDL_Texture* SunFlowerCardTexture = FadeEffect::loadTexture("Assets/Images/Plants/SunFlower/1.png", renderer);
    SDL_Texture* PeaShooterCardTexture = FadeEffect::loadTexture("Assets/Images/Plants/Peashooter/1.png", renderer);
    SDL_Texture* WalnutCardTexture = FadeEffect::loadTexture("Assets/Images/Plants/Wallnut/1.png", renderer);
    SDL_Texture* ShovelTexture = FadeEffect::loadTexture("Assets/Images/Background/Shovel.png", renderer);

    // Set up UI rectangles
    SDL_Rect menuRect = { 0, 0, windowWidth, windowHeight };
    SDL_Rect itemBarRect = { 10, 50, 85, 400 };
    SDL_Rect sunBarRect = { 10, 0, 85, 40 };
    SDL_Rect ShovelRect = { 10, windowHeight - 60, 60, 60 };
    SDL_Rect SunFlowerCardRect = { 15, 55, 75, 50 };
    SDL_Rect PeaShooterCardRect = { 15, 115, 75, 50 };
    SDL_Rect WalnutCardRect = { 15, 180, 75, 50 };
    SDL_Rect CherryBombCardRect = { 15, 245, 75, 50 };
    previewRect.w = 75;
    previewRect.h = 75;

    gameStartTime = SDL_GetTicks();

    // Main game loop
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
            else if (e.type == SDL_KEYDOWN && gameLost) {
                running = false;
            }
            else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) {
                int oldWidth = windowWidth;
                int oldHeight = windowHeight;
                windowWidth = e.window.data1;
                windowHeight = e.window.data2;
                menuRect.w = windowWidth;
                menuRect.h = windowHeight;

                initGrid(windowWidth, windowHeight);

                // Update positions of all game objects
                for (auto& sun : suns) {
                    sun.rect.x = (sun.rect.x * windowWidth) / oldWidth;
                    if (sun.targetY != 0) {
                        sun.targetY = (sun.targetY * windowHeight) / oldHeight;
                    }
                    sun.rect.y = (sun.rect.y * windowHeight) / oldHeight;
                }

                for (auto& sunflower : placedSunflowers) {
                    sunflower.rect.x = gameGrid.area.x + sunflower.gridCol * gameGrid.cellWidth + (gameGrid.cellWidth - sunflower.rect.w) / 2;
                    sunflower.rect.y = gameGrid.area.y + sunflower.gridRow * gameGrid.cellHeight + (gameGrid.cellHeight - sunflower.rect.h) / 2;
                }

                for (auto& plant : placedPeaShooters) {
                    plant.rect.x = gameGrid.area.x + plant.gridCol * gameGrid.cellWidth + (gameGrid.cellWidth - plant.rect.w) / 2;
                    plant.rect.y = gameGrid.area.y + plant.gridRow * gameGrid.cellHeight + (gameGrid.cellHeight - plant.rect.h) / 2;
                }

                for (auto& walnut : placedWalnuts) {
                    walnut.rect.x = gameGrid.area.x + walnut.gridCol * gameGrid.cellWidth + (gameGrid.cellWidth - walnut.rect.w) / 2;
                    walnut.rect.y = gameGrid.area.y + walnut.gridRow * gameGrid.cellHeight + (gameGrid.cellHeight - walnut.rect.h) / 2;
                }

                for (auto& zombie : zombies) {
                    zombie.rect.y = gameGrid.area.y + zombie.gridRow * gameGrid.cellHeight;
                    zombie.rect.x = (zombie.rect.x * windowWidth) / oldWidth;
                }

                // Update UI rectangles
                itemBarRect.w = static_cast<int>(windowWidth * 0.6 * 85.0 / SCREEN_WIDTH);
                itemBarRect.h = static_cast<int>(windowHeight * 400.0 / SCREEN_HEIGHT); 
                sunBarRect.w = static_cast<int>(windowWidth * 0.6 * 85.0 / SCREEN_WIDTH);
                sunBarRect.h = static_cast<int>(windowHeight * 40.0 / SCREEN_HEIGHT);
                SunFlowerCardRect.w = static_cast<int>(windowWidth * 0.6 * 75.0 / SCREEN_WIDTH);
                SunFlowerCardRect.h = static_cast<int>(windowHeight * 0.95 * 50.0 / SCREEN_HEIGHT);
                PeaShooterCardRect.w = static_cast<int>(windowWidth * 0.6 * 75.0 / SCREEN_WIDTH);
                PeaShooterCardRect.h = static_cast<int>(windowHeight * 50.0 / SCREEN_HEIGHT);
                WalnutCardRect.w = static_cast<int>(windowWidth * 0.6 * 75 / SCREEN_WIDTH);
                WalnutCardRect.h = static_cast<int>(windowHeight * 50.0 / SCREEN_HEIGHT);
                CherryBombCardRect.w = static_cast<int>(windowWidth * 0.6 * 75 / SCREEN_WIDTH);
                CherryBombCardRect.h = static_cast<int>(windowHeight * 50.0 / SCREEN_HEIGHT);
                ShovelRect.w = 60;
                ShovelRect.h = 60;
                ShovelRect.x = 10;
                ShovelRect.y = windowHeight - ShovelRect.h - 10;
            }
            handleMouseEvent(e, SunFlowerCardRect, PeaShooterCardRect, CherryBombCardRect, WalnutCardRect, ShovelRect);
        }

        if (gameLost) {
            Uint32 currentTime = SDL_GetTicks();
            if (currentTime - loseTime > LOSE_DISPLAY_TIME) {
                running = false;
            }
            
            SDL_RenderClear(renderer);
            SDL_Rect fullScreenRect = { 0, 0, windowWidth, windowHeight };
            SDL_RenderCopy(renderer, loseTexture, nullptr, &fullScreenRect);
            SDL_RenderPresent(renderer);
            continue;
        }
        else {
            updateCooldown();
            spawnSun(renderer, windowWidth, windowHeight);
            produceSunFromSunflowers(renderer);
            updateSuns();
            updateZombies(windowWidth, windowHeight);
            updatePeas(windowWidth);
            updateCherryBombs();
        }

        // Render everything
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, mainMenuTexture, nullptr, &menuRect);
        renderGrid(renderer);
        SDL_RenderCopy(renderer, itemBarTexture, nullptr, &itemBarRect);
        SDL_RenderCopy(renderer, sunBarTexture, nullptr, &sunBarRect);
        SDL_RenderCopy(renderer, ShovelTexture, nullptr, &ShovelRect);
        
        renderSunflowerCard(renderer, SunFlowerCardTexture, SunFlowerCardRect);
        renderPeaShooterCard(renderer, PeaShooterCardTexture, PeaShooterCardRect);
        renderCherryBombCard(renderer, CherryBombCardTexture, CherryBombCardRect);
        renderWalnutCard(renderer, WalnutCardTexture, WalnutCardRect);
        
        renderSunflowers(renderer);
        renderSunflowerPreview(renderer);
        renderPeaShooters(renderer);
        renderPeaShooterPreview(renderer);
        renderCherryBombs(renderer);
        renderWalnuts(renderer);
        renderWalnutPreview(renderer);
        renderCherryBombPreview(renderer);
        renderShovelPreview(renderer);
        
        renderSuns(renderer);
        renderSunCount(renderer);
        renderZombies(renderer);
        renderPeas(renderer);
        
        for (auto& plant : placedPeaShooters) {
            shootPea(plant);
        }
        
        SDL_RenderPresent(renderer);
    }

    // Clean up
    Mix_FreeMusic(mainMenuMusic);
    SDL_DestroyTexture(mainMenuTexture);
    SDL_DestroyTexture(itemBarTexture);
    SDL_DestroyTexture(sunBarTexture);
    SDL_DestroyTexture(SunFlowerCardTexture);
    SDL_DestroyTexture(PeaShooterCardTexture);
    SDL_DestroyTexture(WalnutCardTexture);
    SDL_DestroyTexture(ShovelTexture);
    
    close();
    return 0;
}
