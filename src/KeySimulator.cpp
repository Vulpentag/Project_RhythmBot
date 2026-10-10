#include "KeySimulator.h"

void KeySimulator::keyDown(WORD vk) {
    INPUT in{};
    in.type = INPUT_KEYBOARD;
    in.ki.wVk = vk;
    SendInput(1, &in, sizeof(INPUT));
}

void KeySimulator::keyUp(WORD vk) {
    INPUT in{};
    in.type = INPUT_KEYBOARD;
    in.ki.wVk = vk;
    in.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &in, sizeof(INPUT));
}

void KeySimulator::tap(WORD vk, int holdMs) {
    keyDown(vk);
    Sleep(holdMs);
    keyUp(vk);
}

bool KeySimulator::canPress(WORD vk, int cooldownMs) {
    auto it = lastPress_.find(vk);
    if (it == lastPress_.end()) return true;
    auto now = Clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - it->second).count();
    return diff >= cooldownMs;
}

void KeySimulator::markPressed(WORD vk) {
    lastPress_[vk] = Clock::now();
}