#include <SDL.h>
#include <SDL_image.h>
#include "Src/ui/FadeEffect.h"

int main(int argc, char* argv[]) {
    // 1️⃣ Khởi tạo SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Không thể khởi tạo SDL: %s", SDL_GetError());
        return -1;
    }

    // 2️⃣ Tạo cửa sổ
    SDL_Window* window = SDL_CreateWindow("PvZ Fusion",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Log("Không thể tạo cửa sổ: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // 3️⃣ Tạo renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("Không thể tạo renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // 4️⃣ Load background
    SDL_Surface* bgSurface = IMG_Load("Assets/Images/Background/begin.png");
    if (!bgSurface) {
        SDL_Log("Không thể load ảnh: %s", IMG_GetError());
    }
    SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);

    // 5️⃣ Hiệu ứng hiện dần (2 giây)
    FadeEffect::fadeIn(renderer, bgTexture, 2000);

    // 6️⃣ Giữ màn hình 1 chút
    SDL_Delay(1000);

    // 7️⃣ Hiệu ứng mờ dần (2 giây)
    FadeEffect::fadeOut(renderer, bgTexture, 2000);

    // 8️⃣ Giải phóng tài nguyên
    SDL_DestroyTexture(bgTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
