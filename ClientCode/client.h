#ifndef CLIENT_H
#define CLIENT_H

#include <QDialog>
#include <QTcpSocket>
#include "game.h"


namespace Ui {
class Dialog;
}

class Chat;

class Client : public QDialog
{
    Q_OBJECT

public:
    explicit Client(QWidget *parent = nullptr);
    ~Client();

private slots:
    void onJoinServerClicked();
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    //void onReadyRead();
    void startGame();
    void endGame();

private:
    void promptUsername();

    Ui::Dialog *ui;
    QTcpSocket *socket;
    Chat *chat;
    QString username;
    GameDialog *gameDialog = new GameDialog(this);

};

#endif // CLIENT_H
