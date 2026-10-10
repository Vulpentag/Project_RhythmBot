#pragma once
#include "Config.h"
#include "ScreenCapture.h"
#include "NoteDetector.h"
#include "KeySimulator.h"

class Controller {
public:
    explicit Controller(const Config& cfg);
    ~Controller();
    void run();
private:
    Config cfg_;
    ScreenCapture capture_;
    NoteDetector detector_;
    KeySimulator keys_;
    bool exitRequested() const;
};