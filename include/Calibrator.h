#pragma once
#include <opencv2/opencv.hpp>
#include "Config.h"

class Calibrator {
public:
    static void run(const Config& cfgIn);
private:
    static void drawOverlay(cv::Mat& frame, const Config& cfg);
    static void createTrackbars(Config& cfg);
};