#include "NoteDetector.h"

NoteDetector::NoteDetector(const Config& cfg)
    : cfg_(cfg),
      kernel_(cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3))) {}

int NoteDetector::xToLane(int x) const {
    const int n = (int)cfg_.laneKeys.size();
    if (n <= 0) return -1;
    int laneW = cfg_.captureW / n;
    if (laneW <= 0) return -1;
    int lane = x / laneW;
    if (lane < 0) lane = 0;
    if (lane >= n) lane = n - 1;
    return lane;
}

std::vector<Note> NoteDetector::detect(const cv::Mat& frame) const {
    std::vector<Note> notes;
    if (frame.empty()) return notes;

    cv::cvtColor(frame, hsv_, cv::COLOR_BGR2HSV);

    auto detectRange = [&](const Config::HsvRange& c) {
        cv::inRange(hsv_, cv::Scalar(c.hMin, c.sMin, c.vMin),
                    cv::Scalar(c.hMax, c.sMax, c.vMax), mask_);
        cv::morphologyEx(mask_, mask_, cv::MORPH_OPEN, kernel_);
        cv::morphologyEx(mask_, mask_, cv::MORPH_CLOSE, kernel_);

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(mask_, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        for (const auto& ct : contours) {
            double area = cv::contourArea(ct);
            if (area < cfg_.minNoteArea || area > cfg_.maxNoteArea) continue;

            cv::Rect box = cv::boundingRect(ct);
            Note note;
            note.box = box;
            note.center = cv::Point(box.x + box.width/2, box.y + box.height/2);
            note.area = area;
            note.lane = xToLane(note.center.x);
            note.timestamp = 0;
            notes.push_back(note);
        }
    };

    if (cfg_.monochromeNotes) {
        detectRange(cfg_.noteHsv);
    } else {
        for (const auto& range : cfg_.laneColors) {
            detectRange(range);
        }
    }
    return notes;
}