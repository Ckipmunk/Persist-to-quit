
#ifndef FADE_EFFECT_H
#define FADE_EFFECT_H

#include <SDL.h>
#include <SDL_image.h>
#include <string>

class FadeEffect {
public:
    static SDL_Texture* loadTexture(const std::string& path, SDL_Renderer* renderer);
    static void fadeIn(SDL_Renderer* renderer, SDL_Texture* texture, int duration);
    static void fadeOut(SDL_Renderer* renderer, SDL_Texture* texture, int duration);
    static void dropIn(SDL_Renderer* renderer, SDL_Texture* texture, int duration);
    static void LoseScreen(SDL_Renderer* renderer, SDL_Texture* loseTexture, int windowWidth, int windowHeight);
};

#endif
