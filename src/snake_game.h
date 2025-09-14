#pragma once

#include <QWidget>
#include <QVector>
#include <QPointF>
#include <QKeyEvent>
#include <QImage>
#include <QPaintEvent>
#include <QTimerEvent>
#include <QTransform>
#include <QDateTime>

/**
 * @class SnakeGame
 * @brief Пользовательский виджет для игры "Змейка" с аффинными преобразованиями
 * 
 * Реализует классическую игру "Змейка" с возможностью движения под любым углом,
 * использованием матриц аффинных преобразований и плавной анимацией.
 */
class SnakeGame : public QWidget
{
    Q_OBJECT

public:
    explicit SnakeGame(QWidget *parent = nullptr);
    ~SnakeGame();

    // Основные игровые методы
    void initGame();
    void pauseGame();
    void resumeGame();
    bool isGameActive() const { return m_inGame; }

signals:
    void gameOver();
    void scoreChanged(int score);
    void gamePaused();
    void gameResumed();
    void escapePressed();

protected:
    // Обработчики событий Qt
    void paintEvent(QPaintEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    // Основные методы игры (требования задания)
    void loadImages();           ///< Загружает изображения элементов игры
    void locateApple();          ///< Размещает яблоко на игровом поле
    void move();                 ///< Управляет движением змейки с использованием матриц
    void checkCollision();       ///< Проверяет столкновения змейки
    
    // Вспомогательные методы
    void gameOverScreen(QPainter &painter);
    void handleBoundaryTeleportation();
    
    // Методы управления движением
    void turnLeft();
    void turnRight();
    void accelerate();
    void decelerate();

    // Игровые константы
    static const int DOT_SIZE = 10;                  ///< Размер сегмента змейки и яблока
    static const int DELAY = 16;                     ///< Задержка таймера (~60 FPS)
    static constexpr qreal MAX_SPEED = 12.0;         ///< Максимальная скорость движения
    static constexpr qreal ACCELERATION = 0.3;       ///< Ускорение при нажатии клавиш
    static constexpr qreal TURN_SPEED = 0.08;        ///< Скорость поворота в радианах за кадр
    static constexpr qreal SEGMENT_DISTANCE = 8.0;   ///< Фиксированное расстояние между сегментами
    static constexpr qreal SMOOTHNESS = 0.1;         ///< Коэффициент плавности интерполяции

    // Игровое состояние
    int m_timerId;                                   ///< ID таймера для игрового цикла
    int m_score;                                     ///< Текущий счет игрока
    bool m_inGame;                                   ///< Флаг активности игры
    bool m_isPaused;                                 ///< Флаг паузы игры

    // Переменные движения и преобразований
    qreal m_directionAngle;                          ///< Текущий угол направления движения (радианы)
    qreal m_currentHeadAngle;                        ///< Интерполированный угол поворота головы
    qreal m_targetHeadAngle;                         ///< Целевой угол поворота головы
    qreal m_currentSpeed;                            ///< Текущая скорость движения
    qreal m_movementProgress;                        ///< Прогресс движения до следующего сегмента
    qreal m_interpolationFactor;                     ///< Фактор интерполяции для плавности
    QTransform m_movementTransform;                  ///< Матрица для аффинных преобразований движения

    // Состояние управления
    bool m_turningLeft;                              ///< Флаг поворота влево
    bool m_turningRight;                             ///< Флаг поворота вправо
    bool m_accelerating;                             ///< Флаг ускорения
    bool m_decelerating;                             ///< Флаг торможения

    // Данные змейки и яблока
    QVector<QPointF> m_snake;                        ///< Логические позиции сегментов змейки
    QVector<QPointF> m_visualSnake;                  ///< Визуальные позиции для плавного отображения
    QVector<QPointF> m_targetPositions;              ///< Целевые позиции для интерполяции
    QPointF m_applePos;                              ///< Позиция яблока на поле
    
    // Изображения элементов игры
    QImage m_dotImage;                               ///< Изображение сегмента тела змейки
    QImage m_headImage;                              ///< Изображение головы змейки
    QImage m_appleImage;                             ///< Изображение яблока
};