// map.h

#ifndef MAP_H
#define MAP_H

#include <QGraphicsScene>

class Map {
public:
    explicit Map(QGraphicsScene *scene);

    // Sets up the background grid for the game
    void setupBackground(int width, int height, int gridSize);

    // Resets the scene and redraws the background grid
    void resetScene(int width, int height, int gridSize);

private:
    QGraphicsScene *scene;
};

#endif // MAP_H
