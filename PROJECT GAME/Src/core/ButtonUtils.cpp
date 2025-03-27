#include "ButtonUtils.h"

// T?a ?? các nút theo t? l? màn hình g?c
struct ButtonInfo {
    float xRatio, yRatio, wRatio, hRatio;
};

// Danh sách các nút và t? l? v? trí
std::vector<ButtonInfo> buttonRatios = {
    {0.6, 0.3, 0.3, 0.08}, // Phiêu l?u
    {0.6, 0.4, 0.3, 0.08}, // Th? thách
    {0.6, 0.5, 0.3, 0.08}, // Gi?i ??
    {0.6, 0.6, 0.3, 0.08}, // Sinh t?n
    {0.6, 0.8, 0.2, 0.08}  // Thoát
};

std::vector<SDL_Rect> getButtonPositions(int windowWidth, int windowHeight) {
    std::vector<SDL_Rect> buttons;

    for (const auto& btn : buttonRatios) {
        buttons.push_back({
            static_cast<int>(btn.xRatio * windowWidth),
            static_cast<int>(btn.yRatio * windowHeight),
            static_cast<int>(btn.wRatio * windowWidth),
            static_cast<int>(btn.hRatio * windowHeight)
            });
    }

    return buttons;
}
