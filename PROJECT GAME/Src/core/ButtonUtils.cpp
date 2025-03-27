#include "ButtonUtils.h"

// T?a ?? c�c n�t theo t? l? m�n h�nh g?c
struct ButtonInfo {
    float xRatio, yRatio, wRatio, hRatio;
};

// Danh s�ch c�c n�t v� t? l? v? tr�
std::vector<ButtonInfo> buttonRatios = {
    {0.6, 0.3, 0.3, 0.08}, // Phi�u l?u
    {0.6, 0.4, 0.3, 0.08}, // Th? th�ch
    {0.6, 0.5, 0.3, 0.08}, // Gi?i ??
    {0.6, 0.6, 0.3, 0.08}, // Sinh t?n
    {0.6, 0.8, 0.2, 0.08}  // Tho�t
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
