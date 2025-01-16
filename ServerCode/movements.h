// movements.h

#ifndef MOVEMENTS_H
#define MOVEMENTS_H

#include <QKeyEvent>
#include "player.h"

class Movements {
public:
    static void handleKeyPress(QKeyEvent *event, Player *player1, Player *player2);
};

#endif // MOVEMENTS_H
