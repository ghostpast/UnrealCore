# Unreal core

C++ проект, использующий систему сборки CMake.

## Требования

- CMake 4.4.2 или выше
- Perl
- gperf
- Компилятор C++ с поддержкой C++23
- JetBrains Rider

## Сборка и запуск

MS windows

Этот проект использует CMake Presets для конфигурации. Откройте проект в Rider и используйте встроенные инструменты CMake IDE для сборки и запуска:

1. Откройте проект в Rider
2. Rider автоматически определит конфигурацию CMake из `CMakePresets.json`
3. Выберите нужную конфигурацию сборки на панели инструментов
4. Используйте кнопки Run/Debug для сборки и запуска проекта

Linux

1. Установите необходимые зависимости:
   ```bash
   sudo apt update
   sudo apt install cmake g++ build-essential ninja-build
   ```

Убедитесь, что версия компилятора поддерживает C++23 (GCC 13+).

3. Настройте проект с помощью CMake:
   ```bash
   cmake --preset default
   ```

(либо укажите нужный пресет из CMakePresets.json)

4. Соберите проект:
   ```bash
   cmake --build build
   ```

5. Запустите исполняемые файлы:
   ```bash
    ./build/authserver/authserver
    ./build/worldserver/worldserver
   ```

Также можно открыть проект в JetBrains Rider на Linux — IDE автоматически подхватит конфигурацию из CMakePresets.json.

## Структура проекта

- `src/` - Исходные файлы
- `CMakeLists.txt` - Конфигурация CMake
- `CMakePresets.json` - Пресеты CMake для интеграции с IDE
