// movements.cpp
#include "movements.h"

void Movements::handleKeyPress(QKeyEvent *event, Player *player1, Player *player2) {
    switch (event->key()) {
        case Qt::Key_W: player1->changeDirection(Player::Up); break;
        case Qt::Key_S: player1->changeDirection(Player::Down); break;
        case Qt::Key_A: player1->changeDirection(Player::Left); break;
        case Qt::Key_D: player1->changeDirection(Player::Right); break;
        case Qt::Key_Up: player2->changeDirection(Player::Up); break;
        case Qt::Key_Down: player2->changeDirection(Player::Down); break;
        case Qt::Key_Left: player2->changeDirection(Player::Left); break;
        case Qt::Key_Right: player2->changeDirection(Player::Right); break;
    }
}
