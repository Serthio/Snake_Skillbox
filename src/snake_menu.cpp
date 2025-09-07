#include "snake_menu.h"
#include <QApplication>

// Конструктор меню
SnakeMenu::SnakeMenu(QWidget *parent) : QWidget(parent),
    startButton(nullptr),
    quitButton(nullptr),
    titleLabel(nullptr),
    controlsLabel(nullptr),
    menuLayout(nullptr)
{
    setFixedSize(600, 600);
    setStyleSheet("background-color: white; color: black;");
    
    // Создаем вертикальный layout для меню
    menuLayout = new QVBoxLayout(this);
    
    // Заголовок игры
    titleLabel = new QLabel("SNAKE GAME", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; margin: 20px;");
    
    // Информация об управлении
    controlsLabel = new QLabel(
        "Управление:\n"
        "← → ↑ ↓ - Стрелки\n"
        "W A S D - WASD\n"
        "P - Пауза\n"
        "ESC - Выход в меню\n"
        "SPACE - Перезапуск",
        this
    );
    controlsLabel->setAlignment(Qt::AlignCenter);
    controlsLabel->setStyleSheet("font-size: 16px; margin: 20px;");
    
    // Кнопка начала игры
    startButton = new QPushButton("Начать игру", this);
    startButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #4CAF50;"
        "  border: none;"
        "  color: white;"
        "  padding: 15px;"
        "  font-size: 18px;"
        "  margin: 10px;"
        "}"
        "QPushButton:hover { background-color: #45a049; }"
    );
    
    // Кнопка выхода из игры
    quitButton = new QPushButton("Выход", this);
    quitButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #f44336;"
        "  border: none;"
        "  color: white;"
        "  padding: 15px;"
        "  font-size: 18px;"
        "  margin: 10px;"
        "}"
        "QPushButton:hover { background-color: #da190b; }"
    );
    
    // Добавляем элементы в layout
    menuLayout->addStretch();
    menuLayout->addWidget(titleLabel);
    menuLayout->addWidget(controlsLabel);
    menuLayout->addStretch();
    menuLayout->addWidget(startButton);
    menuLayout->addWidget(quitButton);
    menuLayout->addStretch();
    
    // Подключаем сигналы кнопок к слотам
    connect(startButton, &QPushButton::clicked, this, &SnakeMenu::onStartClicked);
    connect(quitButton, &QPushButton::clicked, this, &SnakeMenu::onQuitClicked);
}

// Обработчик нажатия кнопки "Начать игру"
void SnakeMenu::onStartClicked()
{
    emit startGameRequested();
}

// Обработчик нажатия кнопки "Выход"
void SnakeMenu::onQuitClicked()
{
    emit quitGameRequested();
}