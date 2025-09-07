#include "snake_game.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QFont>
#include <QFontMetrics>
#include <QApplication>
#include <QDebug>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const qreal SnakeGame::MOVE_SPEED = 5.0;

SnakeGame::SnakeGame(QWidget *parent) : QWidget(parent),
    timerId(0),
    score(0),
    directionAngle(0),
    lastValidDirection(0),
    inGame(false),
    isPaused(false)
{
    setFixedSize(600, 600);
    setStyleSheet("background-color: white; color: black;");
    loadImages();
    setFocusPolicy(Qt::StrongFocus);
}

SnakeGame::~SnakeGame()
{
    if (timerId != 0) {
        killTimer(timerId);
    }
}

void SnakeGame::loadImages()
{
    // Создаем изображение сегмента тела (зеленый круг)
    dotImage = QImage(DOT_SIZE, DOT_SIZE, QImage::Format_ARGB32);
    dotImage.fill(Qt::transparent);
    QPainter dotPainter(&dotImage);
    dotPainter.setRenderHint(QPainter::Antialiasing);
    dotPainter.setBrush(Qt::green);
    dotPainter.setPen(Qt::NoPen);
    dotPainter.drawEllipse(0, 0, DOT_SIZE, DOT_SIZE);

    // Создаем изображение головы (темно-зеленый круг с глазами)
    headImage = QImage(DOT_SIZE, DOT_SIZE, QImage::Format_ARGB32);
    headImage.fill(Qt::transparent);
    QPainter headPainter(&headImage);
    headPainter.setRenderHint(QPainter::Antialiasing);
    headPainter.setBrush(Qt::darkGreen);
    headPainter.setPen(Qt::NoPen);
    headPainter.drawEllipse(0, 0, DOT_SIZE, DOT_SIZE);
    // Глаза
    headPainter.setBrush(Qt::white);
    headPainter.drawEllipse(3, 3, 4, 4);
    headPainter.drawEllipse(8, 3, 4, 4);

    // Загружаем изображение яблока из файла
    QString applePath = QApplication::applicationDirPath() + "/src/pic/apple.jpg";
    appleImage = QImage(applePath);
    
    // Если файл не найден, создаем красный круг как запасной вариант
    if (appleImage.isNull()) {
        appleImage = QImage(DOT_SIZE, DOT_SIZE, QImage::Format_ARGB32);
        appleImage.fill(Qt::transparent);
        QPainter applePainter(&appleImage);
        applePainter.setRenderHint(QPainter::Antialiasing);
        applePainter.setBrush(Qt::red);
        applePainter.setPen(Qt::NoPen);
        applePainter.drawEllipse(0, 0, DOT_SIZE, DOT_SIZE);
    } else {
        // Масштабируем изображение яблока
        appleImage = appleImage.scaled(DOT_SIZE, DOT_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
}

void SnakeGame::initGame()
{
    score = 0;
    snake.clear();
    directionAngle = 0;
    lastValidDirection = 0;
    pressedKeys.clear();
    
    // Создаем начальную змейку из 3 сегментов
    for (int i = 0; i < 3; i++) {
        snake.append(QPointF(300 - i * DOT_SIZE * 1.5, 300));
    }
    
    locateApple();
    inGame = true;
    isPaused = false;
    
    emit scoreChanged(score);
    
    timerId = startTimer(DELAY);
    setFocus();
}

bool SnakeGame::isOppositeDirection(int key) const
{
    // Текущее направление движения
    qreal currentDir = lastValidDirection;
    
    // Проверяем, является ли нажатая клавиша противоположной текущему направлению
    // Только для основных направлений, не для диагоналей
    const qreal tolerance = 0.1;
    
    if (std::abs(currentDir - 0) < tolerance || 
        std::abs(currentDir - 2*M_PI) < tolerance) { // Вправо
        return key == Qt::Key_Left || key == Qt::Key_A;
    }
    else if (std::abs(currentDir - M_PI) < tolerance) { // Влево
        return key == Qt::Key_Right || key == Qt::Key_D;
    }
    else if (std::abs(currentDir - (-M_PI/2)) < tolerance) { // Вверх
        return key == Qt::Key_Down || key == Qt::Key_S;
    }
    else if (std::abs(currentDir - (M_PI/2)) < tolerance) { // Вниз
        return key == Qt::Key_Up || key == Qt::Key_W;
    }
    
    return false;
}

void SnakeGame::updateDirection()
{
    if (pressedKeys.isEmpty()) return;

    // Собираем все активные направления
    bool right = pressedKeys.contains(Qt::Key_Right) || pressedKeys.contains(Qt::Key_D);
    bool left = pressedKeys.contains(Qt::Key_Left) || pressedKeys.contains(Qt::Key_A);
    bool up = pressedKeys.contains(Qt::Key_Up) || pressedKeys.contains(Qt::Key_W);
    bool down = pressedKeys.contains(Qt::Key_Down) || pressedKeys.contains(Qt::Key_S);

    // Определяем новое направление на основе комбинации клавиш
    qreal targetAngle = directionAngle;
    bool directionChanged = false;

    // Диагональные движения имеют приоритет
    if (right && up) {
        targetAngle = -M_PI / 4; // Вправо-вверх
        directionChanged = true;
    }
    else if (right && down) {
        targetAngle = M_PI / 4; // Вправо-вниз
        directionChanged = true;
    }
    else if (left && up) {
        targetAngle = -3 * M_PI / 4; // Влево-вверх
        directionChanged = true;
    }
    else if (left && down) {
        targetAngle = 3 * M_PI / 4; // Влево-вниз
        directionChanged = true;
    }
    // Основные направления
    else if (right) {
        targetAngle = 0; // Вправо
        directionChanged = true;
    }
    else if (left) {
        targetAngle = M_PI; // Влево
        directionChanged = true;
    }
    else if (up) {
        targetAngle = -M_PI / 2; // Вверх
        directionChanged = true;
    }
    else if (down) {
        targetAngle = M_PI / 2; // Вниз
        directionChanged = true;
    }

    if (directionChanged) {
        // Проверяем, не является ли новое направление противоположным текущему
        // Только для основных направлений
        bool isOpposite = false;
        
        if (std::abs(targetAngle - 0) < 0.1 && 
            (std::abs(lastValidDirection - M_PI) < 0.1)) {
            isOpposite = true;
        }
        else if (std::abs(targetAngle - M_PI) < 0.1 && 
                 (std::abs(lastValidDirection - 0) < 0.1)) {
            isOpposite = true;
        }
        else if (std::abs(targetAngle - (-M_PI/2)) < 0.1 && 
                 (std::abs(lastValidDirection - (M_PI/2)) < 0.1)) {
            isOpposite = true;
        }
        else if (std::abs(targetAngle - (M_PI/2)) < 0.1 && 
                 (std::abs(lastValidDirection - (-M_PI/2)) < 0.1)) {
            isOpposite = true;
        }
        
        if (!isOpposite) {
            directionAngle = targetAngle;
            lastValidDirection = targetAngle;
        }
    }
}

void SnakeGame::pauseGame()
{
    if (inGame && !isPaused) {
        killTimer(timerId);
        timerId = 0;
        isPaused = true;
        emit gamePaused();
    }
}

void SnakeGame::resumeGame()
{
    if (inGame && isPaused) {
        timerId = startTimer(DELAY);
        isPaused = false;
        emit gameResumed();
    }
}

void SnakeGame::locateApple()
{
    bool onSnake;
    
    do {
        onSnake = false;
        // Случайная позиция на поле с учетом границ
        qreal x = QRandomGenerator::global()->bounded(DOT_SIZE, width() - DOT_SIZE);
        qreal y = QRandomGenerator::global()->bounded(DOT_SIZE, height() - DOT_SIZE);
        applePos = QPointF(x, y);
        
        // Проверяем, не на змее ли яблоко
        for (const QPointF &segment : snake) {
            qreal dx = segment.x() - applePos.x();
            qreal dy = segment.y() - applePos.y();
            qreal distance = sqrt(dx * dx + dy * dy);
            
            if (distance < DOT_SIZE * 2) {
                onSnake = true;
                break;
            }
        }
    } while (onSnake);
}

void SnakeGame::move()
{
    if (snake.isEmpty()) return;
    
    updateDirection();
    
    // Вычисляем смещение на основе угла и скорости
    qreal dx = cos(directionAngle) * MOVE_SPEED;
    qreal dy = sin(directionAngle) * MOVE_SPEED;
    
    // Сохраняем старые позиции для движения хвоста
    QVector<QPointF> oldPositions = snake;
    
    // Двигаем голову
    QPointF newHeadPos = snake.first();
    newHeadPos.rx() += dx;
    newHeadPos.ry() += dy;
    snake[0] = newHeadPos;
    
    // Остальные сегменты следуют за предыдущими позициями
    for (int i = 1; i < snake.size(); i++) {
        snake[i] = oldPositions[i - 1];
    }
}

void SnakeGame::checkCollision()
{
    if (snake.isEmpty()) return;
    
    QPointF head = snake.first();
    
    // Проверка столкновения со стенами
    if (head.x() < 0 || head.x() >= width() ||
        head.y() < 0 || head.y() >= height()) {
        inGame = false;
        killTimer(timerId);
        timerId = 0;
        emit gameOver();
        return;
    }
    
    // Проверка столкновения с собственным телом
    for (int i = 4; i < snake.size(); i++) {
        qreal dx = head.x() - snake[i].x();
        qreal dy = head.y() - snake[i].y();
        qreal distance = sqrt(dx * dx + dy * dy);
        
        if (distance < DOT_SIZE * 0.7) {
            inGame = false;
            killTimer(timerId);
            timerId = 0;
            emit gameOver();
            return;
        }
    }
    
    // Проверка съедания яблока
    qreal appleDx = head.x() - applePos.x();
    qreal appleDy = head.y() - applePos.y();
    qreal appleDistance = sqrt(appleDx * appleDx + appleDy * appleDy);
    
    if (appleDistance < DOT_SIZE) {
        // Добавляем новый сегмент в конец
        QPointF newSegment = snake.last();
        snake.append(newSegment);
        
        score += 10;
        locateApple();
        emit scoreChanged(score);
    }
}

void SnakeGame::gameOverScreen(QPainter &painter)
{
    QString message = "Game Over";
    QFont font("Arial", 24, QFont::Bold);
    QFontMetrics fm(font);
    
    int textWidth = fm.horizontalAdvance(message);
    painter.setFont(font);
    painter.setPen(Qt::black);
    painter.drawText((width() - textWidth) / 2, height() / 2 - 30, message);
    
    QString scoreMsg = QString("Score: %1").arg(score);
    QFont scoreFont("Arial", 18);
    QFontMetrics fmScore(scoreFont);
    int scoreWidth = fmScore.horizontalAdvance(scoreMsg);
    
    painter.setFont(scoreFont);
    painter.drawText((width() - scoreWidth) / 2, height() / 2 + 10, scoreMsg);
    
    QString restartMsg = "Press SPACE to restart";
    QFont smallFont("Arial", 12);
    QFontMetrics fmSmall(smallFont);
    int restartWidth = fmSmall.horizontalAdvance(restartMsg);
    
    painter.setFont(smallFont);
    painter.drawText((width() - restartWidth) / 2, height() / 2 + 50, restartMsg);
}

void SnakeGame::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Белый фон
    painter.fillRect(rect(), Qt::white);
    
    if (inGame) {
        // Отрисовка яблока
        painter.drawImage(applePos, appleImage);
        
        // Отрисовка змеи
        for (int i = 0; i < snake.size(); i++) {
            if (i == 0) {
                // Голова с поворотом
                painter.save();
                painter.translate(snake[i].x() + DOT_SIZE / 2, snake[i].y() + DOT_SIZE / 2);
                painter.rotate(directionAngle * 180 / M_PI);
                painter.drawImage(-DOT_SIZE / 2, -DOT_SIZE / 2, headImage);
                painter.restore();
            } else {
                // Тело
                painter.drawImage(snake[i], dotImage);
            }
        }
        
        // Отображение счета
        painter.setFont(QFont("Arial", 14));
        painter.setPen(Qt::black);
        painter.drawText(10, 25, QString("Score: %1").arg(score));
        
        // Отладочная информация
        painter.drawText(10, 45, QString("Angle: %1°").arg(int(directionAngle * 180 / M_PI)));
        
    } else {
        gameOverScreen(painter);
    }
    
    if (isPaused) {
        painter.setFont(QFont("Arial", 28, QFont::Bold));
        painter.setPen(Qt::blue);
        painter.drawText(rect(), Qt::AlignCenter, "PAUSED");
    }
}

void SnakeGame::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    
    if (inGame && !isPaused) {
        move();
        checkCollision();
        repaint();
    }
}

void SnakeGame::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    
    // Управление движением
    if (key == Qt::Key_Left || key == Qt::Key_A ||
        key == Qt::Key_Right || key == Qt::Key_D ||
        key == Qt::Key_Up || key == Qt::Key_W ||
        key == Qt::Key_Down || key == Qt::Key_S) {
        
        // Для диагональных движений не блокируем противоположные направления
        // Блокируем только чистые противоположные направления
        bool shouldBlock = isOppositeDirection(key);
        
        if (!shouldBlock) {
            pressedKeys.insert(key);
        }
    }
    
    // Управление игрой
    if (key == Qt::Key_P && inGame) {
        if (isPaused) {
            resumeGame();
        } else {
            pauseGame();
        }
    }
    
    if (key == Qt::Key_Escape) {
        emit escapePressed();
    }
    
    if (key == Qt::Key_Space && !inGame) {
        initGame();
    }
    
    QWidget::keyPressEvent(event);
}

void SnakeGame::keyReleaseEvent(QKeyEvent *event)
{
    int key = event->key();
    
    if (key == Qt::Key_Left || key == Qt::Key_A ||
        key == Qt::Key_Right || key == Qt::Key_D ||
        key == Qt::Key_Up || key == Qt::Key_W ||
        key == Qt::Key_Down || key == Qt::Key_S) {
        pressedKeys.remove(key);
    }
    
    QWidget::keyReleaseEvent(event);
}