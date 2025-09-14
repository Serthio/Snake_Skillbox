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

// Инициализация статических констант
constexpr qreal SnakeGame::MAX_SPEED;
constexpr qreal SnakeGame::ACCELERATION;
constexpr qreal SnakeGame::TURN_SPEED;
constexpr qreal SnakeGame::SEGMENT_DISTANCE;
constexpr qreal SnakeGame::SMOOTHNESS;

/**
 * @brief Конструктор игрового виджета
 * @param parent Родительский виджет
 */
SnakeGame::SnakeGame(QWidget *parent) : QWidget(parent),
    m_timerId(0),
    m_score(0),
    m_inGame(false),
    m_isPaused(false),
    m_directionAngle(0),
    m_currentHeadAngle(0),
    m_targetHeadAngle(0),
    m_currentSpeed(0),
    m_movementProgress(0),
    m_interpolationFactor(0),
    m_turningLeft(false),
    m_turningRight(false),
    m_accelerating(false),
    m_decelerating(false)
{
    setFixedSize(600, 600);
    setStyleSheet("background-color: white; color: black;");
    loadImages();
    setFocusPolicy(Qt::StrongFocus);
}

/**
 * @brief Деструктор игрового виджета
 */
SnakeGame::~SnakeGame()
{
    if (m_timerId != 0) {
        killTimer(m_timerId);
    }
}

/**
 * @brief Загружает изображения для элементов игры (ТРЕБОВАНИЕ 2)
 * 
 * Создает или загружает изображения для сегментов змейки, головы и яблока.
 * Если файл яблока не найден, создается запасной вариант.
 */
void SnakeGame::loadImages()
{
    // Создание изображения сегмента тела (зеленый круг)
    m_dotImage = QImage(DOT_SIZE, DOT_SIZE, QImage::Format_ARGB32);
    m_dotImage.fill(Qt::transparent);
    
    QPainter dotPainter(&m_dotImage);
    dotPainter.setRenderHint(QPainter::Antialiasing);
    dotPainter.setBrush(Qt::green);
    dotPainter.setPen(Qt::NoPen);
    dotPainter.drawEllipse(0, 0, DOT_SIZE, DOT_SIZE);

    // Создание изображения головы (темно-зеленый круг с глазами)
    m_headImage = QImage(DOT_SIZE, DOT_SIZE, QImage::Format_ARGB32);
    m_headImage.fill(Qt::transparent);
    
    QPainter headPainter(&m_headImage);
    headPainter.setRenderHint(QPainter::Antialiasing);
    headPainter.setBrush(Qt::darkGreen);
    headPainter.setPen(Qt::NoPen);
    headPainter.drawEllipse(0, 0, DOT_SIZE, DOT_SIZE);
    
    // Добавление глаз
    headPainter.setBrush(Qt::white);
    headPainter.drawEllipse(2, 2, 3, 3);
    headPainter.drawEllipse(5, 2, 3, 3);

    // Загрузка изображения яблока с обработкой ошибок
    QString applePath = QApplication::applicationDirPath() + "/src/pic/apple.jpg";
    m_appleImage = QImage(applePath);
    
    if (m_appleImage.isNull()) {
        // Создание запасного изображения яблока
        m_appleImage = QImage(DOT_SIZE, DOT_SIZE, QImage::Format_ARGB32);
        m_appleImage.fill(Qt::transparent);
        
        QPainter applePainter(&m_appleImage);
        applePainter.setRenderHint(QPainter::Antialiasing);
        applePainter.setBrush(Qt::red);
        applePainter.setPen(Qt::NoPen);
        applePainter.drawEllipse(0, 0, DOT_SIZE, DOT_SIZE);
    } else {
        // Масштабирование загруженного изображения
        m_appleImage = m_appleImage.scaled(DOT_SIZE, DOT_SIZE, 
                                         Qt::KeepAspectRatio, 
                                         Qt::SmoothTransformation);
    }
}

/**
 * @brief Инициализирует новую игру (ТРЕБОВАНИЕ 3)
 * 
 * Сбрасывает все игровые параметры, создает начальную змейку,
 * размещает яблоко и запускает игровой цикл.
 */
void SnakeGame::initGame()
{
    // Сброс игрового состояния
    m_score = 0;
    m_snake.clear();
    m_visualSnake.clear();
    m_targetPositions.clear();
    
    m_directionAngle = 0;
    m_currentHeadAngle = 0;
    m_targetHeadAngle = 0;
    m_currentSpeed = 0;
    m_movementProgress = 0;
    m_interpolationFactor = 0;
    
    m_turningLeft = false;
    m_turningRight = false;
    m_accelerating = false;
    m_decelerating = false;
    
    // Создание начальной змейки из 3 сегментов
    const qreal startX = 300;
    const qreal startY = 300;
    
    for (int i = 0; i < 3; i++) {
        QPointF segment(startX - i * SEGMENT_DISTANCE, startY);
        m_snake.append(segment);
        m_visualSnake.append(segment);
    }
    
    m_targetPositions = m_snake;
    locateApple(); // Размещение яблока на поле
    
    m_inGame = true;
    m_isPaused = false;
    
    emit scoreChanged(m_score);
    
    // Запуск игрового таймера
    m_timerId = startTimer(DELAY);
    setFocus();
}

/**
 * @brief Случайным образом размещает яблоко на игровом поле (ТРЕБОВАНИЕ 4)
 * 
 * Убеждается, что яблоко не размещается поверх змейки.
 * Вызывается при инициализации игры и после съедания яблока.
 */
void SnakeGame::locateApple()
{
    bool onSnake;
    
    do {
        onSnake = false;
        
        // Генерация случайной позиции в пределах поля
        const qreal x = QRandomGenerator::global()->bounded(DOT_SIZE, width() - DOT_SIZE);
        const qreal y = QRandomGenerator::global()->bounded(DOT_SIZE, height() - DOT_SIZE);
        m_applePos = QPointF(x, y);
        
        // Проверка, не попадает ли яблоко на змейку
        for (const QPointF &segment : m_snake) {
            const qreal dx = segment.x() - m_applePos.x();
            const qreal dy = segment.y() - m_applePos.y();
            const qreal distance = std::hypot(dx, dy);
            
            if (distance < DOT_SIZE * 2) {
                onSnake = true;
                break;
            }
        }
    } while (onSnake);
}

/**
 * @brief Управляет движением змейки с использованием матрицы перемещения (ТРЕБОВАНИЕ 5)
 * 
 * Контролируется только голова змейки - направление её движения изменяется с помощью стрелок.
 * Остальные части тела змейки по цепочке перемещаются друг за другом.
 * Перемещение задается с помощью матрицы аффинных преобразований.
 */
void SnakeGame::move()
{
    if (m_snake.isEmpty()) return;
    
    // Обработка ввода пользователя (управление стрелками/WASD)
    if (m_turningLeft) turnLeft();
    if (m_turningRight) turnRight();
    if (m_accelerating) accelerate();
    if (m_decelerating) decelerate();
    
    // Плавная интерполяция угла поворота головы
    m_targetHeadAngle = m_directionAngle;
    m_currentHeadAngle += (m_targetHeadAngle - m_currentHeadAngle) * 0.2;
    
    // Обновление прогресса движения
    m_movementProgress += m_currentSpeed;
    
    // Добавление нового сегмента при достаточном прогрессе
    if (m_movementProgress >= SEGMENT_DISTANCE) {
        m_movementProgress = 0;
        
        // Сохранение текущих позиций для интерполяции
        m_targetPositions = m_snake;
        
        // ████████████████████████████████████████████████████████████████████████
        // ИСПОЛЬЗОВАНИЕ МАТРИЦЫ ПЕРЕМЕЩЕНИЯ ДЛЯ АФФИННЫХ ПРЕОБРАЗОВАНИЙ
        // ████████████████████████████████████████████████████████████████████████
        
        m_movementTransform.reset(); // Сбрасываем матрицу преобразований
        
        // 1. Перенос в текущую позицию головы
        m_movementTransform.translate(m_snake.first().x(), m_snake.first().y());
        
        // 2. Поворот на текущий угол направления (аффинное преобразование)
        m_movementTransform.rotate(m_directionAngle * 180 / M_PI);
        
        // 3. Перемещение вперед на расстояние сегмента
        m_movementTransform.translate(SEGMENT_DISTANCE, 0);
        
        // 4. Получение новой позиции головы из матрицы преобразования
        QPointF newHeadPos = m_movementTransform.map(QPointF(0, 0));
        
        // Добавление новой головы
        m_snake.prepend(newHeadPos);
        
        // Удаление хвоста (остальные части движутся за головой по цепочке)
        if (m_snake.size() > 3 + m_score / 10) {
            m_snake.removeLast();
        }
        
        m_interpolationFactor = 0;
    }
    
    // Интерполяция для плавного движения
    m_interpolationFactor += SMOOTHNESS;
    m_interpolationFactor = qMin(m_interpolationFactor, 1.0);
    
    // Вычисление визуальных позиций для плавной анимации
    m_visualSnake.clear();
    for (int i = 0; i < m_snake.size(); i++) {
        if (i < m_targetPositions.size()) {
            // Линейная интерполяция между старыми и новыми позициями
            const qreal x = m_targetPositions[i].x() + 
                           (m_snake[i].x() - m_targetPositions[i].x()) * m_interpolationFactor;
            const qreal y = m_targetPositions[i].y() + 
                           (m_snake[i].y() - m_targetPositions[i].y()) * m_interpolationFactor;
            m_visualSnake.append(QPointF(x, y));
        } else {
            m_visualSnake.append(m_snake[i]);
        }
    }
    
    handleBoundaryTeleportation();
}

/**
 * @brief Проверяет столкновения змейки со стеной или своим телом (ТРЕБОВАНИЕ 6)
 */
void SnakeGame::checkCollision()
{
    if (m_snake.isEmpty()) return;
    
    const QPointF head = m_snake.first();
    
    // Проверка столкновения с собственным телом
    for (int i = 4; i < m_snake.size(); i++) {
        const qreal dx = head.x() - m_snake[i].x();
        const qreal dy = head.y() - m_snake[i].y();
        const qreal distance = std::hypot(dx, dy);
        
        if (distance < DOT_SIZE * 0.8) {
            m_inGame = false;
            killTimer(m_timerId);
            m_timerId = 0;
            emit gameOver();
            return;
        }
    }
    
    // Проверка съедания яблока
    const qreal appleDx = head.x() - m_applePos.x();
    const qreal appleDy = head.y() - m_applePos.y();
    const qreal appleDistance = std::hypot(appleDx, appleDy);
    
    if (appleDistance < DOT_SIZE) {
        m_score += 10;
        locateApple(); // Размещаем новое яблоко
        emit scoreChanged(m_score);
        
        // Добавление буфера для плавного роста змейки
        const int growthBuffer = 3;
        if (m_snake.size() < 3 + m_score / 10 + growthBuffer) {
            m_snake.append(m_snake.last());
        }
    }
}

/**
 * @brief Обрабатывает события таймера (игровой цикл) (ТРЕБОВАНИЕ 7)
 * 
 * При условии, что игра ещё не закончена, выполняется обнаружение столкновений 
 * змеи с препятствиями и её дальнейшее перемещение.
 */
void SnakeGame::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    
    if (m_inGame && !m_isPaused) {
        move();             // Перемещение змейки
        checkCollision();   // Проверка столкновений
        repaint();          // Перерисовка окна
    }
}

/**
 * @brief Отрисовывает экран завершения игры
 */
void SnakeGame::gameOverScreen(QPainter &painter)
{
    const QString message = "Game Over";
    const QFont font("Arial", 24, QFont::Bold);
    const QFontMetrics fm(font);
    
    const int textWidth = fm.horizontalAdvance(message);
    painter.setFont(font);
    painter.setPen(Qt::black);
    painter.drawText((width() - textWidth) / 2, height() / 2 - 30, message);
    
    const QString scoreMsg = QString("Score: %1").arg(m_score);
    const QFont scoreFont("Arial", 18);
    const QFontMetrics fmScore(scoreFont);
    const int scoreWidth = fmScore.horizontalAdvance(scoreMsg);
    
    painter.setFont(scoreFont);
    painter.drawText((width() - scoreWidth) / 2, height() / 2 + 10, scoreMsg);
    
    const QString restartMsg = "Press SPACE to restart";
    const QFont smallFont("Arial", 12);
    const QFontMetrics fmSmall(smallFont);
    const int restartWidth = fmSmall.horizontalAdvance(restartMsg);
    
    painter.setFont(smallFont);
    painter.drawText((width() - restartWidth) / 2, height() / 2 + 50, restartMsg);
}

/**
 * @brief Обрабатывает телепортацию змейки через границы экрана
 */
void SnakeGame::handleBoundaryTeleportation()
{
    if (m_snake.isEmpty() || m_visualSnake.isEmpty()) return;
    
    // Обработка телепортации для всех сегментов
    for (int i = 0; i < m_snake.size(); i++) {
        QPointF &segment = m_snake[i];
        QPointF &visualSegment = m_visualSnake[i];
        
        // Телепортация логических позиций
        if (segment.x() < -DOT_SIZE * 3) segment.setX(width() + DOT_SIZE * 2);
        else if (segment.x() > width() + DOT_SIZE * 3) segment.setX(-DOT_SIZE * 2);
        
        if (segment.y() < -DOT_SIZE * 3) segment.setY(height() + DOT_SIZE * 2);
        else if (segment.y() > height() + DOT_SIZE * 3) segment.setY(-DOT_SIZE * 2);
        
        // Телепортация визуальных позиций
        if (visualSegment.x() < -DOT_SIZE * 3) visualSegment.setX(width() + DOT_SIZE * 2);
        else if (visualSegment.x() > width() + DOT_SIZE * 3) visualSegment.setX(-DOT_SIZE * 2);
        
        if (visualSegment.y() < -DOT_SIZE * 3) visualSegment.setY(height() + DOT_SIZE * 2);
        else if (visualSegment.y() > height() + DOT_SIZE * 3) visualSegment.setY(-DOT_SIZE * 2);
    }
}

/**
 * @brief Обрабатывает событие перерисовки виджета
 * 
 * Выполняет отрисовку всех игровых элементов с использованием аффинных преобразований:
 * - Масштабирование игрового поля
 * - Поворот элементов  
 * - Изменение формы змейки
 * - Перемещение объектов
 */
void SnakeGame::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Отрисовка фона
    painter.fillRect(rect(), Qt::white);
    
    // Отрисовка границ поля
    painter.setPen(QPen(Qt::gray, 1, Qt::DashLine));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
    
    if (m_inGame) {
        // ████████████████████████████████████████████████████████████████████████
        // АФФИННЫЕ ПРЕОБРАЗОВАНИЯ ДЛЯ ЯБЛОКА: вращение + масштабирование
        // ████████████████████████████████████████████████████████████████████████
        QTransform appleTransform;
        appleTransform.translate(m_applePos.x() + DOT_SIZE / 2, m_applePos.y() + DOT_SIZE / 2);
        appleTransform.rotate(QDateTime::currentMSecsSinceEpoch() / 20.0); // Вращение
        appleTransform.scale(1.1, 1.1); // Масштабирование
        appleTransform.translate(-DOT_SIZE / 2, -DOT_SIZE / 2);
        
        painter.setTransform(appleTransform);
        painter.drawImage(0, 0, m_appleImage);
        painter.resetTransform();
        
        // Отрисовка змейки с аффинными преобразованиями
        for (int i = 0; i < m_visualSnake.size(); i++) {
            if (i == 0) {
                // ████████████████████████████████████████████████████████████████████████
                // АФФИННЫЕ ПРЕОБРАЗОВАНИЯ ДЛЯ ГОЛОВЫ: поворот
                // ████████████████████████████████████████████████████████████████████████
                QTransform headTransform;
                headTransform.translate(m_visualSnake[i].x() + DOT_SIZE / 2, 
                                      m_visualSnake[i].y() + DOT_SIZE / 2);
                headTransform.rotate(m_currentHeadAngle * 180 / M_PI);
                headTransform.translate(-DOT_SIZE / 2, -DOT_SIZE / 2);
                
                painter.setTransform(headTransform);
                painter.drawImage(0, 0, m_headImage);
                painter.resetTransform();
            } else {
                // ████████████████████████████████████████████████████████████████████████
                // АФФИННЫЕ ПРЕОБРАЗОВАНИЯ ДЛЯ ТЕЛА: масштабирование + изменение формы
                // ████████████████████████████████████████████████████████████████████████
                if (i > 0 && i < m_visualSnake.size() - 1) {
                    QTransform bodyTransform;
                    const qreal scale = 0.9 + 0.1 * (i % 2); // Чередующееся масштабирование
                    bodyTransform.translate(m_visualSnake[i].x() + DOT_SIZE / 2, 
                                          m_visualSnake[i].y() + DOT_SIZE / 2);
                    bodyTransform.scale(scale, scale); // Изменение формы
                    bodyTransform.translate(-DOT_SIZE / 2, -DOT_SIZE / 2);
                    
                    painter.setTransform(bodyTransform);
                    painter.drawImage(0, 0, m_dotImage);
                    painter.resetTransform();
                } else {
                    painter.drawImage(m_visualSnake[i], m_dotImage);
                }
            }
        }
        
        // Отрисовка игровой информации
        painter.setFont(QFont("Arial", 12));
        painter.setPen(Qt::black);
        painter.drawText(10, 20, QString("Score: %1").arg(m_score));
        painter.drawText(10, 40, QString("Speed: %1").arg(m_currentSpeed, 0, 'f', 1));
        painter.drawText(10, 60, QString("Length: %1").arg(m_snake.size()));
        painter.drawText(10, 80, QString("Angle: %1°").arg(int(m_currentHeadAngle * 180 / M_PI)));
        
    } else {
        gameOverScreen(painter);
    }
    
    if (m_isPaused) {
        painter.setFont(QFont("Arial", 28, QFont::Bold));
        painter.setPen(Qt::blue);
        painter.drawText(rect(), Qt::AlignCenter, "PAUSED");
    }
}

/**
 * @brief Выполняет поворот змейки влево
 */
void SnakeGame::turnLeft()
{
    m_directionAngle -= TURN_SPEED;
    // Нормализация угла
    while (m_directionAngle < 0) m_directionAngle += 2 * M_PI;
}

/**
 * @brief Выполняет поворот змейки вправо
 */
void SnakeGame::turnRight()
{
    m_directionAngle += TURN_SPEED;
    // Нормализация угла
    while (m_directionAngle >= 2 * M_PI) m_directionAngle -= 2 * M_PI;
}

/**
 * @brief Увеличивает скорость движения змейки
 */
void SnakeGame::accelerate()
{
    m_currentSpeed = qMin(m_currentSpeed + ACCELERATION, MAX_SPEED);
}

/**
 * @brief Уменьшает скорость движения змейки
 */
void SnakeGame::decelerate()
{
    m_currentSpeed = qMax(m_currentSpeed - ACCELERATION * 2, 0.0);
}

/**
 * @brief Обрабатывает нажатия клавиш
 */
void SnakeGame::keyPressEvent(QKeyEvent *event)
{
    const int key = event->key();
    
    // Управление движением (стрелки и WASD)
    switch (key) {
        case Qt::Key_Left:
        case Qt::Key_A:
            m_turningLeft = true;
            break;
        case Qt::Key_Right:
        case Qt::Key_D:
            m_turningRight = true;
            break;
        case Qt::Key_Up:
        case Qt::Key_W:
            m_accelerating = true;
            break;
        case Qt::Key_Down:
        case Qt::Key_S:
            m_decelerating = true;
            break;
    }
    
    // Управление игрой
    if (key == Qt::Key_P && m_inGame) {
        if (m_isPaused) {
            resumeGame();
        } else {
            pauseGame();
        }
    }
    
    if (key == Qt::Key_Escape) {
        emit escapePressed();
    }
    
    if (key == Qt::Key_Space && !m_inGame) {
        initGame();
    }
    
    QWidget::keyPressEvent(event);
}

/**
 * @brief Обрабатывает отпускания клавиш
 */
void SnakeGame::keyReleaseEvent(QKeyEvent *event)
{
    const int key = event->key();
    
    // Сброс флагов управления
    switch (key) {
        case Qt::Key_Left:
        case Qt::Key_A:
            m_turningLeft = false;
            break;
        case Qt::Key_Right:
        case Qt::Key_D:
            m_turningRight = false;
            break;
        case Qt::Key_Up:
        case Qt::Key_W:
            m_accelerating = false;
            break;
        case Qt::Key_Down:
        case Qt::Key_S:
            m_decelerating = false;
            break;
    }
    
    QWidget::keyReleaseEvent(event);
}

/**
 * @brief Приостанавливает игру
 */
void SnakeGame::pauseGame()
{
    if (m_inGame && !m_isPaused) {
        killTimer(m_timerId);
        m_timerId = 0;
        m_isPaused = true;
        emit gamePaused();
    }
}

/**
 * @brief Возобновляет игру после паузы
 */
void SnakeGame::resumeGame()
{
    if (m_inGame && m_isPaused) {
        m_timerId = startTimer(DELAY);
        m_isPaused = false;
        emit gameResumed();
    }
}