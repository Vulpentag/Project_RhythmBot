#pragma once
#include <opencv2/opencv.hpp>
#include <windows.h>

class ScreenCapture {
public:
    ScreenCapture();
    ~ScreenCapture();
    void setRegion(int x, int y, int w, int h);
    cv::Mat capture();
private:
    int x_ = 0, y_ = 0, width_ = 0, height_ = 0;
    HDC hScreenDC_ = nullptr;
    HDC hMemDC_ = nullptr;
    HBITMAP hBitmap_ = nullptr;
    HBITMAP hOldBitmap_ = nullptr;
    void* bitmapBits_ = nullptr;
    void releaseResources();
    void createResources();
};