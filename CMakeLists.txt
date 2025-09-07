# Минимальная требуемая версия CMake
cmake_minimum_required(VERSION 3.14)

# Настройка проекта
project(
    video5
    LANGUAGES CXX
    VERSION 0.9.0
    DESCRIPTION "Snake Game"
)

# Установка типа сборки (Debug/Release)
set(CMAKE_BUILD_TYPE Debug)
# Использование стандарта C++17
set(CMAKE_CXX_STANDARD 17)
# Требование поддержки стандарта C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Включение автоматической генерации MOC (Meta-Object Compiler)
set(CMAKE_AUTOMOC ON)

# Флаги линковки для Windows (консольное приложение)
set(LINK_FLAGS "-mwindows -mconsole -Wl,-subsystem,console")

# Список исходных файлов проекта
set(sources
    src/snake_game.h
    src/snake_game.cpp
    src/snake_menu.h
    src/snake_menu.cpp
    src/main.cpp
)

# Создаем директорию и копируем файл с яблоком
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/src/pic)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/pic/apple.jpg 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/src/pic)

# Поиск необходимых компонентов Qt5
find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Gui REQUIRED)

# Создание исполняемого файла с WIN32 для Windows приложения
add_executable(snake_game WIN32 ${sources})

# Включение автоматической генерации MOC для целевого файла
set_target_properties(snake_game PROPERTIES AUTOMOC ON)

# Подключение библиотек Qt5 к целевому исполняемому файлу
target_link_libraries(snake_game PRIVATE Qt5::Core Qt5::Widgets Qt5::Gui ${LINK_FLAGS})

# Установка свойства для создания Windows исполняемого файла
set_property(TARGET snake_game PROPERTY WIN32_EXECUTABLE true)