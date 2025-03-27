
#include "FadeEffect.h"

SDL_Texture* FadeEffect::loadTexture(const std::string& path, SDL_Renderer* renderer) {
    SDL_Texture* texture = IMG_LoadTexture(renderer, path.c_str());
    return texture;
}

void FadeEffect::fadeIn(SDL_Renderer* renderer, SDL_Texture* texture, int duration) {
    for (int alpha = 0; alpha <= 255; alpha += 5) {
        SDL_SetTextureAlphaMod(texture, alpha);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
        SDL_Delay(duration / 50);
    }
}

void FadeEffect::fadeOut(SDL_Renderer* renderer, SDL_Texture* texture, int duration) {
    for (int alpha = 255; alpha >= 0; alpha -= 5) {
        SDL_SetTextureAlphaMod(texture, alpha);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
        SDL_Delay(duration / 50);
    }
}

void FadeEffect::dropIn(SDL_Renderer* renderer, SDL_Texture* texture, int duration) {
    int screenW, screenH;
    SDL_GetRendererOutputSize(renderer, &screenW, &screenH);
    SDL_Rect dstRect = { screenW / 2 - 200, -400, 400, 400 }; // Bắt đầu từ trên trời
    int step = screenH / (duration / 10);

    while (dstRect.y < screenH / 2 - 200) {
        dstRect.y += step;
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
        SDL_RenderPresent(renderer);
        SDL_Delay(10);
    }
}
