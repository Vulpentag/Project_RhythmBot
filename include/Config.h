#pragma once
#include <string>
#include <vector>
#include <windows.h>

struct Config {
    // === Región de captura ===
    int captureX = 437;
    int captureY = 500;
    int captureW = 490;
    int captureH = 190;

    // Centro vertical de las notas objetivo, relativo al area capturada.
    int hitLineY = 145;
    int hitWindowPx = 18;

    // === Teclas de los 4 carriles ===
    std::vector<WORD> laneKeys = { 'A', 'S', 'K', 'L' };

    // === Detección de notas ===
    // Las notas bajan por cada carril; se pulsan al acercarse al tambor.

    // Tiempo mínimo entre pulsaciones del mismo carril (ms)
    // Evita pulsar la misma nota varias veces
    int laneCooldownMs = 250;

    // Si la misma nota (mismo carril, Y parecida) se ve en frames
    // consecutivos, no volver a pulsar. Este es el margen en px.
    int sameNoteTolerancePx = 25;

    // Si la nota desaparece de un carril durante este tiempo (ms),
    // se considera que ya se pulsó y se resetea el estado.
    int noteResetMs = 400;

    // === Rendimiento ===
    int targetFps = 60;
    bool debugView = false;

    // === Detección de color ===
    bool monochromeNotes = true;
    struct HsvRange { int hMin, hMax, sMin, sMax, vMin, vMax; };

    // Azul típico de las notas
    HsvRange noteHsv = { 100, 130, 100, 255, 100, 255 };

    std::vector<HsvRange> laneColors = {
        {  0,  10, 120, 255, 120, 255 },
        { 20,  35, 120, 255, 120, 255 },
        { 85, 100, 120, 255, 120, 255 },
        {140, 170, 120, 255, 120, 255 },
    };

    double minNoteArea = 100.0;
    double maxNoteArea = 5000.0;
};