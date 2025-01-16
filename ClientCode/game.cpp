#include "game.h"
#include <QDebug>

GameDialog::GameDialog(QWidget *parent) :
    QDialog(parent)
{
    this->setWindowTitle("Game Window");
    this->resize(800, 600); // Set a reasonable size for the game window
    this->setStyleSheet("background-color: black; color: white;"); // Styling for the game
}

GameDialog::~GameDialog()
{

}

void GameDialog::keyPressEvent(QKeyEvent *event)
{
    // Monitor key presses
    switch (event->key()) {
    case Qt::Key_W:
        emit keyPressed("W");
        qDebug() << "W key pressed.";
        break;
    case Qt::Key_A:
        emit keyPressed("A");
        qDebug() << "A key pressed.";
        break;
    case Qt::Key_S:
        emit keyPressed("S");
        qDebug() << "S key pressed.";
        break;
    case Qt::Key_D:
        emit keyPressed("D");
        qDebug() << "D key pressed.";
        break;
    case Qt::Key_I:
        emit keyPressed("W");
        qDebug() << "I key pressed.";
        break;
    case Qt::Key_J:
        emit keyPressed("A");
        qDebug() << "J key pressed.";
        break;
    case Qt::Key_K:
        emit keyPressed("S");
        qDebug() << "K key pressed.";
        break;
    case Qt::Key_L:
        emit keyPressed("D");
        qDebug() << "L key pressed.";
        break;
    default:
        QDialog::keyPressEvent(event);
        break;
    }
}
