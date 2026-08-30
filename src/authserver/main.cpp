/*
* Copyright (C) 2026 Unreal Core
* Условия использования указаны в файле LICENSE.md в корне проекта.
*/

#include <iostream>
#include <asio.hpp>
#include "revision.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main() {

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    
    std::cout << "UnrealCore AuthServer" << std::endl;
    std::cout << std::endl;
    std::cout << "Revision: " << REVISION_ID << std::endl;
    std::cout << "Branch:   " << REVISION_BRANCH << std::endl;
    std::cout << "Date:     " << REVISION_DATE << std::endl;
    std::cout << "Built:    " << REVISION_TIME << std::endl;
    std::cout << std::endl;
    
    asio::io_context io_context;
    
    // Ждём Ctrl+C или сигнал завершения
    asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](const std::error_code&, int) {
        std::cout << "\nЗавершение работы..." << std::endl;
        io_context.stop();
    });
    
    std::cout << "Для остановки нажмите Ctrl+C." << std::endl;
    
    io_context.run(); // блокирует поток, пока io_context не остановлен
    
    return 0;
}
