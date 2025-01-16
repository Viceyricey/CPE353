#include "chat.h"
#include "ui_chat.h"
#include <QDebug>
#include "client.h"

Chat::Chat(QTcpSocket *socket, QString username, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Chat),
    socket(socket),
    username(username),
    isReady(false)
{
    ui->setupUi(this);

    // Set the entire background color of the chat dialog
    this->setStyleSheet("background-color: #000000;");  // Black background for the chat window

    // Set the color scheme for chat display and input areas with neon orange
    ui->chatDisplay->setStyleSheet("background-color: #333333; color: #FFA500; border: 2px solid #FFA500;");  // Neon orange text and border
    ui->messageInput->setStyleSheet("background-color: #333333; color: #FFA500; border: 2px solid #FFA500; font-size: 18px; font-weight: bold;");  // Neon orange text and border, bold

    // Style the send button with neon orange
    ui->sendButton->setStyleSheet("background-color: #DA6E02; color: black; font-weight: bold;");  // Neon orange background, black text

    // Style the readyButton with neon orange border and text
    ui->readyButton->setStyleSheet("border: 2px solid #FFA500; color: #FFA500; font-weight: bold;");  // Neon orange border and text

    // Style the readyLabel with neon orange for visibility
    ui->readyLabel->setStyleSheet("background-color: #282828; color: #FFA500; border: 2px solid #FFA500; font-weight: bold; font-size: 16px; padding: 5px;");  // Dark background, neon orange text and border

    // Connect send button to send message function
    connect(ui->sendButton, &QPushButton::clicked, this, &Chat::sendMessage);

    // Connect readyCheckBox (or readyButton) to toggle between ready and not ready
    connect(ui->readyButton, &QCheckBox::stateChanged, this, [this](int state) {
        if (state == Qt::Checked) {
            readyUp();
        } else {
            notReadyUp();
        }
    });

    // Connect socket's readyRead signal to onReadyRead slot to receive messages
    connect(socket, &QTcpSocket::readyRead, this, &Chat::onReadyRead);

    // Set a welcome message in the chat display
    ui->chatDisplay->append("Hey " + username + ", you joined the server!");
    ui->chatDisplay->append("==================================================\n");
}

Chat::~Chat()
{
    // Disconnect the socket's readyRead signal before deleting the socket
    disconnect(socket, &QTcpSocket::readyRead, this, &Chat::onReadyRead);

    socket->close();

    // Clean up the socket if it is owned by this dialog
    delete socket;  // If you're passing a raw socket, make sure it's deleted only once
    delete ui;      // Delete the UI (automatically handles child widgets)
    qDebug() << "Chat dialog closed and resources deallocated.";
}

void Chat::sendMessage()
{
    QString message = ui->messageInput->text();
    if (!message.isEmpty()) {
        ui->chatDisplay->append("You: " + message);
        socket->write(("CHAT:" + message + "\n").toUtf8());
        ui->messageInput->clear();
    }
}

void Chat::onReadyRead() {
    QByteArray data = socket->readAll();
    QString message = QString::fromUtf8(data).trimmed();

    if (message == "GAME_START") {
        qDebug() << "Game start message received!";
        emit gameStart();  // Emit signal to start game
        qDebug() << "TEST";
    }
    else if (message == "GAME_END") {
        emit gameEnd();
        ui->readyButton->setChecked(false);


    }else {
        // Check if the message contains your username to prevent double display
        if (!message.startsWith(username + ":")) {
            ui->chatDisplay->append(message);
        }
    }
}



void Chat::readyUp()
{
    isReady = true;
    QString statusMessage = "READY:" + username;
    socket->write(statusMessage.toUtf8() + "\n");
    ui->readyLabel->setText("You Are Ready!");
}

void Chat::notReadyUp()
{
    isReady = false;
    QString statusMessage = "NOT_READY:" + username;
    socket->write(statusMessage.toUtf8() + "\n");
    ui->readyLabel->setText("You Are Not Ready!");
}
