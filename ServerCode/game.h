#ifndef GAME_H
#define GAME_H

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsView>
#include <QMap>
#include <QString>
#include <QGraphicsItem>
#include <QSet>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>


class Game : public QDialog
{
    Q_OBJECT

public:
    explicit Game(QWidget *parent = nullptr);
    ~Game();

    void processClientInput(const QString &playerName, const QString &keyInput);
    void addPlayer(const QString &playerName);

    enum Direction { Up, Down, Left, Right };
    Direction currentDirection;
    QList<QGraphicsRectItem *> trailItems;
    void initializeDatabase();
    void initializeLifetimeDatabase();
    void recordPlayerLoss(const QString &playerName, int lossOrder);
    void displayLossOrder();
    void updateLifetimeLeaderboard();
    void displayLifetimeLeaderboard();

signals:
    void gameEnded();

private slots:
    void advance();

private:
    void drawPerimeterLines();
    void changeDirection(Direction d);
    void leaveTrail(QGraphicsRectItem *re, Direction d, const QColor &color);

    QColor playerColor;
    bool hasStartedMoving;
    QMap<QString, QColor> playerColors;
    QSet<QString> frozenPlayers;
    QList<QGraphicsRectItem *> borderItems;

    QGraphicsScene *scene;
    QGraphicsView *view;
    QMap<QString, QPointF> playerVelocities;
    QMap<QString, QGraphicsRectItem *> players;
    QHash<QString, QGraphicsRectItem*> playerFrontRectangles;

    bool hasGameEnded = false;

    QSqlDatabase db;
    QSqlDatabase lifetimeDb;  // Persistent database

    int lossCounter = 1;
};

#endif // GAME_H

