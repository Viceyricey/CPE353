// chat.h

#ifndef CHAT_H
#define CHAT_H

#include <QDialog>
#include <QTcpSocket>

namespace Ui {
class Chat;
}

class Chat : public QDialog
{
    Q_OBJECT

public:
    explicit Chat(QTcpSocket *socket, QString username, QWidget *parent = nullptr);
    ~Chat();

signals:
    void gameStart();  // Signal to notify the client when the game starts
    void gameEnd();

private slots:
    void sendMessage();
    void onReadyRead();
    void readyUp();                        // Slot for handling the "ReadyUp" button click
    void notReadyUp();                        // Slot for handling the "NotReadyUp" button click


private:
    Ui::Chat *ui;
    QTcpSocket *socket;
    QString username;
    bool isReady;
};

#endif // CHAT_H
