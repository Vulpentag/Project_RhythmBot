#pragma once
#include <windows.h>
#include <unordered_map>
#include <chrono>

class KeySimulator {
public:
    void tap(WORD vk, int holdMs = 12);
    void keyDown(WORD vk);
    void keyUp(WORD vk);
    bool canPress(WORD vk, int cooldownMs);
    void markPressed(WORD vk);
private:
    using Clock = std::chrono::steady_clock;
    std::unordered_map<WORD, Clock::time_point> lastPress_;
};