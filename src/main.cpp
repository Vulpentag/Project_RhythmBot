#include <iostream>
#include <exception>
#include <windows.h>
#include <string>
#include "Config.h"
#include "Controller.h"
#include "Calibrator.h"

int main(int argc, char** argv) {
    try {
        SetConsoleOutputCP(CP_UTF8);

        Config cfg;

        bool calibrar = false;
        for (int i = 1; i < argc; ++i)
            if (std::string(argv[i]) == "calibrar") calibrar = true;

        if (calibrar) {
            std::cout << "=== MODO CALIBRACION ===\n";
            Calibrator::run(cfg);
            return 0;
        }

        std::cout << "=== MODO BOT ===\n";
        Controller controller(cfg);
        controller.run();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "[ERROR] " << error.what() << "\n";
        return 1;
    }
}