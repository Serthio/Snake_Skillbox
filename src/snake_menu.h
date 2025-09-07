#pragma once

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

// Класс меню игры "Змейка"
class SnakeMenu : public QWidget
{
    Q_OBJECT  // Макрос для поддержки сигналов и слотов Qt

public:
    explicit SnakeMenu(QWidget *parent = nullptr);  // Конструктор

signals:
    void startGameRequested();  // Сигнал запроса начала игры
    void quitGameRequested();   // Сигнал запроса выхода из игры

private slots:
    void onStartClicked();  // Слот обработки нажатия "Начать игру"
    void onQuitClicked();   // Слот обработки нажатия "Выход"

private:
    QPushButton *startButton;    // Указатель на кнопку начала игры
    QPushButton *quitButton;     // Указатель на кнопку выхода
    QLabel *titleLabel;          // Указатель на заголовок
    QLabel *controlsLabel;       // Указатель на информацию об управлении
    QVBoxLayout *menuLayout;     // Указатель на основной layout меню
};