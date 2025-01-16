// map.cpp
#include "map.h"
#include <QPixmap>
#include <QPainter>
#include <QPen>

Map::Map(QGraphicsScene *scene) : scene(scene) {}

void Map::setupBackground(int width, int height, int gridSize) {
    // Create a pixmap of the specified size
    QPixmap background(width, height);
    background.fill(Qt::black);  // Set background color (black for Tron)

    QPainter painter(&background);
    painter.setRenderHint(QPainter::Antialiasing);

    // Set pen for grid lines
    QPen pen(Qt::darkCyan);  // Tron-like color
    pen.setWidth(1);
    painter.setPen(pen);

    // Draw vertical grid lines
    for (int x = 0; x < width; x += gridSize) {
        painter.drawLine(x, 0, x, height);
    }

    // Draw horizontal grid lines
    for (int y = 0; y < height; y += gridSize) {
        painter.drawLine(0, y, width, y);
    }

    painter.end();

    // Set the background of the scene using the generated QPixmap
    scene->setBackgroundBrush(background);
}

void Map::resetScene(int width, int height, int gridSize) {
    // Clear all items from the scene
    scene->clear();
    // Re-setup the background to reset the grid
    setupBackground(width, height, gridSize);
}
