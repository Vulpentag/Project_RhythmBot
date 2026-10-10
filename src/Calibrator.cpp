#include "Calibrator.h"
#include "ScreenCapture.h"
#include "NoteDetector.h"
#include <iostream>

static Config* g_cfg = nullptr;
static cv::Mat g_lastFrame;

void Calibrator::createTrackbars(Config& cfg) {
    cv::namedWindow("Calibracion", cv::WINDOW_AUTOSIZE);

    // Solo trackbars de la región de captura
    cv::createTrackbar("capX", "Calibracion", nullptr, 1366,
        [](int v, void*){ g_cfg->captureX = v; }, nullptr);
    cv::createTrackbar("capY", "Calibracion", nullptr, 768,
        [](int v, void*){ g_cfg->captureY = v; }, nullptr);
    cv::createTrackbar("capW", "Calibracion", nullptr, 1366,
        [](int v, void*){ g_cfg->captureW = v < 50 ? 50 : v; }, nullptr);
    cv::createTrackbar("capH", "Calibracion", nullptr, 768,
        [](int v, void*){ g_cfg->captureH = v < 50 ? 50 : v; }, nullptr);
    cv::createTrackbar("hitY", "Calibracion", nullptr, 768,
        [](int v, void*){ g_cfg->hitLineY = v; }, nullptr);

    cv::setTrackbarPos("capX", "Calibracion", cfg.captureX);
    cv::setTrackbarPos("capY", "Calibracion", cfg.captureY);
    cv::setTrackbarPos("capW", "Calibracion", cfg.captureW);
    cv::setTrackbarPos("capH", "Calibracion", cfg.captureH);
    cv::setTrackbarPos("hitY", "Calibracion", cfg.hitLineY);
}

void Calibrator::drawOverlay(cv::Mat& frame, const Config& cfg) {
    const int n = (int)cfg.laneKeys.size();
    if (n <= 0) return;
    int laneW = frame.cols / n;

    cv::line(frame, {0, cfg.hitLineY},
             {frame.cols - 1, cfg.hitLineY},
             cv::Scalar(0, 255, 0), 1);
    cv::line(frame, {0, cfg.hitLineY - cfg.hitWindowPx},
             {frame.cols - 1, cfg.hitLineY - cfg.hitWindowPx},
             cv::Scalar(0, 180, 0), 1);
    cv::line(frame, {0, cfg.hitLineY + cfg.hitWindowPx},
             {frame.cols - 1, cfg.hitLineY + cfg.hitWindowPx},
             cv::Scalar(0, 180, 0), 1);

    // Líneas verticales de carril
    for (int i = 0; i <= n; ++i)
        cv::line(frame, {i*laneW, 0}, {i*laneW, frame.rows},
                 cv::Scalar(0, 255, 0), 1);

    // Números de carril
    for (int i = 0; i < n; ++i)
        cv::putText(frame, std::to_string(i), {i*laneW + 5, 20},
                    cv::FONT_HERSHEY_SIMPLEX, 0.5,
                    cv::Scalar(0, 255, 0), 1);
}

void Calibrator::run(const Config& cfgIn) {
    static Config cfg = cfgIn;
    g_cfg = &cfg;
    ScreenCapture capture;
    NoteDetector detector(cfg);

    createTrackbars(cfg);

    // Muestreo por clic
    cv::setMouseCallback("Calibracion",
        [](int event, int x, int y, int, void*) {
            if (event != cv::EVENT_LBUTTONDOWN || g_lastFrame.empty()) return;
            int fx = x / 2, fy = y / 2;   // vista previa está x2
            if (fx < 0 || fy < 0 ||
                fx >= g_lastFrame.cols || fy >= g_lastFrame.rows) return;
            cv::Vec3b bgr = g_lastFrame.at<cv::Vec3b>(fy, fx);
            cv::Mat px(1, 1, CV_8UC3, bgr), pxHsv;
            cv::cvtColor(px, pxHsv, cv::COLOR_BGR2HSV);
            cv::Vec3b hsv = pxHsv.at<cv::Vec3b>(0, 0);
            std::cout << "Punto (" << fx << "," << fy << ") "
                      << "BGR=(" << (int)bgr[0] << "," << (int)bgr[1]
                      << "," << (int)bgr[2] << ") "
                      << "HSV=(" << (int)hsv[0] << "," << (int)hsv[1]
                      << "," << (int)hsv[2] << ")\n";
        }, nullptr);

    std::cout << "[Calibrador] Ajusta trackbars. Clic en una nota para ver su HSV. ESC para salir.\n";

    while (true) {
        capture.setRegion(cfg.captureX, cfg.captureY,
                          cfg.captureW, cfg.captureH);
        cv::Mat frame = capture.capture();
        if (frame.empty()) { cv::waitKey(10); continue; }

        g_lastFrame = frame.clone();
        detector.setConfig(cfg);
        auto notes = detector.detect(frame);

        // Dibujar notas detectadas
        for (const auto& n : notes) {
            cv::rectangle(frame, n.box, cv::Scalar(255, 0, 255), 1);
            cv::circle(frame, n.center, 3, cv::Scalar(255, 255, 0), -1);
            cv::putText(frame, "L" + std::to_string(n.lane),
                        {n.box.x, n.box.y - 5},
                        cv::FONT_HERSHEY_SIMPLEX, 0.4,
                        cv::Scalar(0, 255, 255), 1);
        }

        drawOverlay(frame, cfg);

        // Vista previa ampliada x2
        cv::Mat preview;
        cv::resize(frame, preview, {frame.cols * 2, frame.rows * 2},
                   0, 0, cv::INTER_NEAREST);
        cv::imshow("Calibracion", preview);

        if (cv::waitKey(1) == 27) break;  // ESC
    }
    cv::destroyAllWindows();

    std::cout << "\n=== Copia estos valores en Config.h ===\n";
    std::cout << "cfg.captureX = " << cfg.captureX << ";\n";
    std::cout << "cfg.captureY = " << cfg.captureY << ";\n";
    std::cout << "cfg.captureW = " << cfg.captureW << ";\n";
    std::cout << "cfg.captureH = " << cfg.captureH << ";\n";
    std::cout << "cfg.hitLineY = " << cfg.hitLineY << ";\n";
}