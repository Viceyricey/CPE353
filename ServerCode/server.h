// server.h
#ifndef SERVER_H
#define SERVER_H

#include <QDialog>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QMap>
#include <QNetworkInterface>
#include <QTimer>
#include "game.h"

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();


private slots:
    void on_startServerButton_clicked(); // function fired when server's start button clicked
    void on_stopServerButton_clicked(); // function fired when server's stop button clicked
    void acceptConnection(); // function allowing server to accept player/client connection
    void onPlayerDisconnected(); // function fired when player/client disconnects
    void onReadyRead(); // function allowing server to read information from client/player
    void on_player1KickButton_clicked(); // function fired when player(s) 1-4 kicked from server
    void on_player2KickButton_clicked();
    void on_player3KickButton_clicked();
    void on_player4KickButton_clicked();
    void kickPlayer(int index); // function allowing server to kick players
    void onGameEnded();

private:
    Ui::Dialog *ui;
    QTcpServer *tcpServer; // tcp socket variable
    QList<QTcpSocket*> playerSockets; // list of player sockets

    Game *game = nullptr;
    QMap<QTcpSocket*, QString> playerNames; // map of player names and their sockets

    void setPlayerLabel(int index, const QString &playerName); // function used for setting each label with their associated player name
    void clearPlayerLabel(int index); // function allowing server to clear a player's label when they leave the game
    void setPlayerReadyStatus(QTcpSocket *playerSocket, const QString &playerName, bool isReady); // function allowing server to set the ready status of the player
    void startMatch(); // function that allows the player to start the match
    void checkAllPlayersReady(); // function to check and see if all of the players are ready
    void broadcastMessage(const QByteArray &message); // function to broadcast messages to the players including the game countdown
    bool getReadyStatus(int index) const; // function to check the ready status of each player
    void broadcastPlayerStates();  // New function for broadcasting initial player states
};

#endif // SERVER_H
