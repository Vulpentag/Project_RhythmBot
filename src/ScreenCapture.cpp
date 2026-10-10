#include "ScreenCapture.h"
#include <stdexcept>

ScreenCapture::ScreenCapture() : hScreenDC_(GetDC(nullptr)) {
    if (!hScreenDC_) {
        throw std::runtime_error("No se pudo obtener el contexto de pantalla.");
    }
}

ScreenCapture::~ScreenCapture() {
    releaseResources();
    if (hScreenDC_) ReleaseDC(nullptr, hScreenDC_);
}

void ScreenCapture::setRegion(int x, int y, int w, int h) {
    if (w <= 0 || h <= 0) {
        throw std::invalid_argument("El ancho y alto de captura deben ser positivos.");
    }
    if (w == width_ && h == height_ && hMemDC_ && hBitmap_) {
        x_ = x;
        y_ = y;
        return;
    }

    releaseResources();
    x_ = x;
    y_ = y;
    width_ = w;
    height_ = h;
    createResources();
}

void ScreenCapture::releaseResources() {
    if (hMemDC_) {
        if (hOldBitmap_) SelectObject(hMemDC_, hOldBitmap_);
        DeleteDC(hMemDC_);
        hMemDC_ = nullptr; hOldBitmap_ = nullptr;
    }
    if (hBitmap_) { DeleteObject(hBitmap_); hBitmap_ = nullptr; }
    bitmapBits_ = nullptr;
}

void ScreenCapture::createResources() {
    hMemDC_ = CreateCompatibleDC(hScreenDC_);
    if (!hMemDC_) {
        throw std::runtime_error("No se pudo crear el contexto de captura.");
    }

    BITMAPINFO bitmapInfo{};
    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = width_;
    bitmapInfo.bmiHeader.biHeight = -height_;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    hBitmap_ = CreateDIBSection(hScreenDC_, &bitmapInfo, DIB_RGB_COLORS,
                                &bitmapBits_, nullptr, 0);
    if (!hBitmap_ || !bitmapBits_) {
        releaseResources();
        throw std::runtime_error("No se pudo crear el bitmap de captura.");
    }

    HGDIOBJ previousBitmap = SelectObject(hMemDC_, hBitmap_);
    if (!previousBitmap || previousBitmap == HGDI_ERROR) {
        releaseResources();
        throw std::runtime_error("No se pudo seleccionar el bitmap de captura.");
    }
    hOldBitmap_ = reinterpret_cast<HBITMAP>(previousBitmap);
}

cv::Mat ScreenCapture::capture() {
    if (!hMemDC_ || !hBitmap_ || !bitmapBits_) {
        throw std::runtime_error("La captura de pantalla no está inicializada.");
    }
    if (!BitBlt(hMemDC_, 0, 0, width_, height_, hScreenDC_, x_, y_, SRCCOPY)) {
        throw std::runtime_error("BitBlt no pudo capturar la región de pantalla.");
    }
    if (!GdiFlush()) {
        throw std::runtime_error("GdiFlush no pudo completar la captura.");
    }

    cv::Mat bgra(height_, width_, CV_8UC4, bitmapBits_);
    cv::Mat bgr;
    cv::cvtColor(bgra, bgr, cv::COLOR_BGRA2BGR);
    return bgr;
}