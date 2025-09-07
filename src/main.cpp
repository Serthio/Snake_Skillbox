#include "snake_game.h"
#include "snake_menu.h"
#include <QApplication>
#include <QStackedWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QStackedWidget *stackedWidget = new QStackedWidget;
    stackedWidget->setFixedSize(600, 600);
    
    SnakeMenu *menu = new SnakeMenu;
    SnakeGame *game = new SnakeGame;
    
    stackedWidget->addWidget(menu);
    stackedWidget->addWidget(game);
    stackedWidget->setCurrentWidget(menu);
    
    stackedWidget->setWindowTitle("Snake Game");
    
    QObject::connect(menu, &SnakeMenu::startGameRequested, [=]() {
        game->initGame();
        stackedWidget->setCurrentWidget(game);
        game->setFocus();
    });
    
    QObject::connect(menu, &SnakeMenu::quitGameRequested, &app, &QApplication::quit);
    
    QObject::connect(game, &SnakeGame::escapePressed, [=]() {
        if (game->isGameActive()) {
            game->pauseGame();
        }
        stackedWidget->setCurrentWidget(menu);
    });
    
    QObject::connect(game, &SnakeGame::gameOver, [=]() {
        // Дополнительная логика при завершении игры
    });
    
    stackedWidget->show();
    
    return app.exec();
}