#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include "Config.h"

struct Note {
    int lane;
    cv::Rect box;
    cv::Point center;
    double area;
    double timestamp;
};

class NoteDetector {
public:
    explicit NoteDetector(const Config& cfg);
    std::vector<Note> detect(const cv::Mat& frame) const;
    void setConfig(const Config& cfg) { cfg_ = cfg; }
private:
    Config cfg_;
    mutable cv::Mat hsv_;
    mutable cv::Mat mask_;
    const cv::Mat kernel_;
    int xToLane(int x) const;
};