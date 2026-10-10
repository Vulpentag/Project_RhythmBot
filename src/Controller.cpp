#include "Controller.h"
#include <algorithm>
#include <chrono>
#include <thread>
#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <cstdlib>
#include <windows.h>

using Clock = std::chrono::steady_clock;

// ============================================================
// Envío de teclas
// ============================================================

static bool sendInputScanCode(WORD vk, bool keyDown) {
    UINT scanCode = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
    if (scanCode == 0) return false;

    INPUT in{};
    in.type = INPUT_KEYBOARD;
    in.ki.wScan = (WORD)scanCode;
    in.ki.dwFlags = KEYEVENTF_SCANCODE;
    if (!keyDown) in.ki.dwFlags |= KEYEVENTF_KEYUP;
    return SendInput(1, &in, sizeof(INPUT)) == 1;
}

static bool isRobloxForeground() {
    HWND foreground = GetForegroundWindow();
    if (!foreground) return false;

    char title[512]{};
    const int length = GetWindowTextA(foreground, title, sizeof(title));
    if (length <= 0) return false;

    std::string normalized(title, static_cast<size_t>(length));
    for (char& ch : normalized) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return normalized.find("roblox") != std::string::npos ||
           normalized.find("shogun") != std::string::npos;
}

struct PendingKeyReleases {
    const Config& cfg;
    std::vector<Clock::time_point>& releaseAt;

    ~PendingKeyReleases() {
        for (size_t lane = 0; lane < releaseAt.size(); ++lane) {
            if (releaseAt[lane] != Clock::time_point::min() &&
                !sendInputScanCode(cfg.laneKeys[lane], false)) {
                std::cerr << "[ERROR] No se pudo liberar la tecla del carril "
                          << lane << " al salir.\n";
            }
        }
    }
};

// ============================================================
// Controller
// ============================================================

Controller::Controller(const Config& cfg) : cfg_(cfg), detector_(cfg) {}
Controller::~Controller() {}

bool Controller::exitRequested() const {
    return (GetAsyncKeyState(VK_F12) & 0x8000) != 0;
}

void Controller::run() {
    capture_.setRegion(cfg_.captureX, cfg_.captureY,
                       cfg_.captureW, cfg_.captureH);

    std::cout << "==================================================\n";
    std::cout << "[Controller] Iniciado. F12 para salir.\n";
    std::cout << "[Controller] Region: " << cfg_.captureX << "," << cfg_.captureY
              << " (" << cfg_.captureW << "x" << cfg_.captureH << ")\n";
    std::cout << "[Controller] laneCooldown=" << cfg_.laneCooldownMs
              << "ms  sameNoteTol=" << cfg_.sameNoteTolerancePx << "px\n";

    std::cout << "==================================================\n";

    const size_t nLanes = cfg_.laneKeys.size();

    // Estado por carril
    std::vector<int> lastNoteY(nLanes, -9999);       // última Y pulsada
    std::vector<long long> lastNoteTime(nLanes, 0);  // cuándo la pulsamos
    std::vector<long long> lastSeenTime(nLanes, 0);  // última vez vista
    std::vector<Note> bestNote(nLanes, Note{-1, {}, {}, 0, 0});
    std::vector<int> bestDistance(nLanes, cfg_.hitWindowPx + 1);
    std::vector<Clock::time_point> releaseAt(
        nLanes, Clock::time_point::min());
    PendingKeyReleases pendingKeyReleases{cfg_, releaseAt};

    const auto frameTime = (cfg_.targetFps > 0)
        ? std::chrono::microseconds(1000000 / cfg_.targetFps)
        : std::chrono::microseconds(0);

    auto startTime = Clock::now();
    bool wasGameForeground = false;

    while (!exitRequested()) {
        auto t0 = Clock::now();
        const bool gameForeground = isRobloxForeground();
        if (gameForeground != wasGameForeground) {
            std::cout << (gameForeground
                ? "[INFO] Roblox esta activo; deteccion habilitada.\n"
                : "[INFO] Esperando a que Roblox este en primer plano.\n");
            wasGameForeground = gameForeground;
        }

        for (size_t lane = 0; lane < nLanes; ++lane) {
            if (releaseAt[lane] != Clock::time_point::min() &&
                t0 >= releaseAt[lane]) {
                if (sendInputScanCode(cfg_.laneKeys[lane], false)) {
                    releaseAt[lane] = Clock::time_point::min();
                } else {
                    std::cerr << "[ERROR] No se pudo liberar la tecla del carril "
                              << lane << ".\n";
                }
            }
        }

        cv::Mat frame = capture_.capture();
        if (frame.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }

        auto notes = detector_.detect(frame);
        auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                        t0 - startTime).count();

        // Si hay varias notas cerca del objetivo, elegir la más próxima.
        std::fill(bestNote.begin(), bestNote.end(), Note{-1, {}, {}, 0, 0});
        std::fill(bestDistance.begin(), bestDistance.end(),
                  cfg_.hitWindowPx + 1);

        for (const auto& n : notes) {
            if (n.lane < 0 || n.lane >= (int)nLanes) continue;
            const int distance = std::abs(n.center.y - cfg_.hitLineY);
            if (distance <= cfg_.hitWindowPx &&
                distance < bestDistance[n.lane]) {
                bestDistance[n.lane] = distance;
                bestNote[n.lane] = n;
            }
        }

        for (size_t lane = 0; lane < nLanes; ++lane) {
            bool hasNote = bestNote[lane].lane >= 0;

            // Resetear también si una nota nueva aparece después del intervalo.
            if ((nowMs - lastSeenTime[lane]) > cfg_.noteResetMs) {
                lastNoteY[lane] = -9999;
                lastNoteTime[lane] = 0;
            }
            if (!hasNote) continue;
            lastSeenTime[lane] = nowMs;

            int y = bestNote[lane].center.y;

            // ¿Es una nota NUEVA o la misma que ya pulsamos?
            bool sameAsBefore = false;
            if (std::abs(y - lastNoteY[lane]) < cfg_.sameNoteTolerancePx) {
                long long dt = nowMs - lastNoteTime[lane];
                if (dt < cfg_.noteResetMs) {
                    sameAsBefore = true;
                }
            }

            if (sameAsBefore) continue; // ya la pulsamos

            // ¿Cooldown del carril?
            WORD vk = cfg_.laneKeys[lane];
            if (!keys_.canPress(vk, cfg_.laneCooldownMs)) continue;
            if (!gameForeground) continue;

            // ¡PULSAR!
            std::cout << "[PULSA] " << (char)vk
                      << " lane=" << lane
                      << " y=" << y
                      << " area=" << (int)bestNote[lane].area << "\n";

            if (!sendInputScanCode(vk, true)) {
                std::cerr << "[ERROR] No se pudo pulsar la tecla del carril "
                          << lane << ".\n";
                continue;
            }

            releaseAt[lane] = Clock::now() + std::chrono::milliseconds(15);
            keys_.markPressed(vk);
            lastNoteY[lane] = y;
            lastNoteTime[lane] = nowMs;
        }

        if (cfg_.debugView) {
            cv::Mat vis = frame.clone();
            const int hitTop = std::max(0, cfg_.hitLineY - cfg_.hitWindowPx);
            const int hitBottom = std::min(
                vis.rows - 1, cfg_.hitLineY + cfg_.hitWindowPx);
            cv::line(vis, {0, cfg_.hitLineY},
                     {vis.cols - 1, cfg_.hitLineY},
                     cv::Scalar(0, 255, 0), 1);
            cv::line(vis, {0, hitTop}, {vis.cols - 1, hitTop},
                     cv::Scalar(0, 180, 0), 1);
            cv::line(vis, {0, hitBottom}, {vis.cols - 1, hitBottom},
                     cv::Scalar(0, 180, 0), 1);
            for (const auto& n : notes) {
                cv::rectangle(vis, n.box, cv::Scalar(0, 0, 255), 2);
                cv::putText(vis, "L" + std::to_string(n.lane),
                            {n.box.x, n.box.y - 5},
                            cv::FONT_HERSHEY_SIMPLEX, 0.4,
                            cv::Scalar(0, 255, 255), 1);
            }
            cv::imshow("Debug", vis);
            cv::waitKey(1);
        }

        auto t1 = Clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
        if (frameTime.count() > 0 && elapsed < frameTime) {
            std::this_thread::sleep_for(frameTime - elapsed);
        }
    }

    std::cout << "[Controller] Detenido.\n";
}