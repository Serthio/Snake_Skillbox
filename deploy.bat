@echo off
echo === Snake Game Deployment ===

:: Очистка и создание папки Release
if exist Release rmdir /s /q Release
mkdir Release

:: Копируем EXE
copy build\snake_game.exe Release\

:: Основные Qt5 DLL
copy "C:\msys64\mingw64\bin\Qt5Core.dll" Release\
copy "C:\msys64\mingw64\bin\Qt5Gui.dll" Release\
copy "C:\msys64\mingw64\bin\Qt5Widgets.dll" Release\

:: Системные DLL Mingw64
copy "C:\msys64\mingw64\bin\libgcc_s_seh-1.dll" Release\
copy "C:\msys64\mingw64\bin\libstdc++-6.dll" Release\
copy "C:\msys64\mingw64\bin\libwinpthread-1.dll" Release\

:: Дополнительные зависимости
copy "C:\msys64\mingw64\bin\Qt5Svg.dll" Release\
copy "C:\msys64\mingw64\bin\libpcre2-16-0.dll" Release\

:: Плагины (только необходимые)
xcopy "C:\msys64\mingw64\share\qt5\plugins\platforms" Release\plugins\platforms /E /I /Y

:: Ресурсы игры
xcopy "src\pic" Release\src\pic /E /I /Y

:: Файл конфигурации Qt
echo [Paths] > Release\qt.conf
echo Plugins = plugins >> Release\qt.conf

echo.
echo Deployment successful!
echo All files are in: %CD%\Release\
echo.
echo To run the game:
echo 1. Go to Release folder
echo 2. Double-click snake_game.exe
echo.
pause