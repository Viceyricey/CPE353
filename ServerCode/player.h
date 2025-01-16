// player.h

#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QList>

class Player : public QGraphicsPixmapItem {
public:
    enum Direction { Up, Down, Left, Right };

    Player(const QPixmap &pixmap, int startX, int startY);
    void move();
    void changeDirection(Direction newDirection);
    void leaveTrail();

    Direction currentDirection;
    QList<QGraphicsRectItem *> trail; // List of trail segments
};

#endif // PLAYER_H
