#pragma once

#include <QWidget>
#include <QVector>
#include <QPointF>
#include <QKeyEvent>
#include <QImage>
#include <QPaintEvent>
#include <QTimerEvent>
#include <QSet>
#include <cmath>

class SnakeGame : public QWidget
{
    Q_OBJECT

public:
    explicit SnakeGame(QWidget *parent = nullptr);
    ~SnakeGame();

    void initGame();
    void pauseGame();
    void resumeGame();
    bool isGameActive() const { return inGame; }

signals:
    void gameOver();
    void scoreChanged(int score);
    void gamePaused();
    void gameResumed();
    void escapePressed();

protected:
    void paintEvent(QPaintEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    void loadImages();
    void locateApple();
    void move();
    void checkCollision();
    void gameOverScreen(QPainter &painter);
    void updateDirection();
    bool isOppositeDirection(int key) const;

    // Константы игры
    static const int DOT_SIZE = 15;
    static const int DELAY = 50;
    static const qreal MOVE_SPEED;

    int timerId;
    int score;
    qreal directionAngle;
    qreal lastValidDirection;

    QVector<QPointF> snake;
    QPointF applePos;
    
    QImage dotImage;
    QImage headImage;
    QImage appleImage;

    bool inGame;
    bool isPaused;
    
    QSet<int> pressedKeys;
};