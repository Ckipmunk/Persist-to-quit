#pragma once
#ifndef BUTTON_UTILS_H
#define BUTTON_UTILS_H

#include <SDL.h>
#include <vector>

// Hàm tính toán các nút dựa trên kích thước cửa sổ
std::vector<SDL_Rect> getButtonPositions(int windowWidth, int windowHeight);

#endif
