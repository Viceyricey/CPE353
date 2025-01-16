// player.cpp

#include "player.h"
#include <QBrush>

Player::Player(const QPixmap &pixmap, int startX, int startY)
    : QGraphicsPixmapItem(pixmap)
{
    setPos(startX, startY);
    currentDirection = Right;
}

void Player::move() {
    int x = pos().x();
    int y = pos().y();

    switch (currentDirection) {
        case Up:    setPos(x, y - 5); break;
        case Down:  setPos(x, y + 5); break;
        case Left:  setPos(x - 5, y); break;
        case Right: setPos(x + 5, y); break;
    }
    leaveTrail();
}

void Player::changeDirection(Direction newDirection) {
    if ((currentDirection == Up && newDirection != Down) ||
        (currentDirection == Down && newDirection != Up) ||
        (currentDirection == Left && newDirection != Right) ||
        (currentDirection == Right && newDirection != Left))
    {
        currentDirection = newDirection;
    }
}

void Player::leaveTrail() {
    QGraphicsRectItem *trailPart = new QGraphicsRectItem(x(), y(), 5, 5);
    trailPart->setBrush(QBrush(Qt::yellow)); // Explicitly create a QBrush with the color
    trail.append(trailPart);
}
