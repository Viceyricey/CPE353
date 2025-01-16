#include "client.h"
#include "ui_dialog.h"
#include "chat.h"
#include "usernameDialog.h"
#include <QFontDatabase>
#include <QMessageBox>
#include <QNetworkProxy>
#include <QDebug>

Client::Client(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    socket(new QTcpSocket(this)),
    chat(nullptr),
    gameDialog(nullptr)
{
    ui->setupUi(this);

    // Load and set custom font for the welcome label
    int fontId = QFontDatabase::addApplicationFont(":/ethnocentricrg.otf");
    if (fontId == -1) {
        qWarning() << "Failed to load font";
    } else {
        QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).value(0);
        QFont ethnocentricFont(fontFamily, 70, QFont::Bold);
        ui->welcomeLabel->setFont(ethnocentricFont);
    }

    // Apply Tron-like styling
    this->setStyleSheet("background-color: #000000;");
    ui->welcomeLabel->setStyleSheet("color: #00FFFF; font-weight: bold;");
    ui->statusLabel->setStyleSheet("color: #00FFFF; font-weight: bold;");
    ui->ipInput->setStyleSheet("background-color: #333333; color: #00FFFF; border: 2px solid #00FFFF; font-size: 18px; font-weight: bold;");
    ui->portInput->setStyleSheet("background-color: #333333; color: #00FFFF; border: 2px solid #00FFFF; font-size: 18px; font-weight: bold;");
    ui->ipLabel->setStyleSheet("color: #00FFFF; font-weight: bold;");
    ui->portLabel->setStyleSheet("color: #00FFFF; font-weight: bold;");
    ui->joinServerButton->setStyleSheet("background-color: #00FFFF; color: black; font-weight: bold;");

    socket->setProxy(QNetworkProxy::NoProxy);

    // Connect signals and slots
    connect(ui->joinServerButton, &QPushButton::clicked, this, &Client::onJoinServerClicked);
    connect(socket, &QTcpSocket::connected, this, &Client::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &Client::onDisconnected);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), this, &Client::onError);


}

Client::~Client()
{
    delete socket;
    delete ui;
}

void Client::onJoinServerClicked()
{
    QString ip = ui->ipInput->text();
    quint16 port = ui->portInput->text().toUInt();

    if (ip.isEmpty() || port == 0) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a valid IP address and port.");
        return;
    }

    ui->statusLabel->setText("Connecting...");
    socket->connectToHost(QHostAddress(ip), port);

    if (!socket->waitForConnected(5000)) {
        QMessageBox::critical(this, "Connection Error", "Failed to connect to the server: " + socket->errorString());
        ui->statusLabel->setText("Connection Failed");
    }
}

void Client::onConnected()
{
    qDebug() << "Successfully connected to the server!";
    ui->statusLabel->setText("Connected to server");
    promptUsername();
}

void Client::promptUsername()
{
    // Open Username Window
    UsernameDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString username = dialog.getUsername();
        if (!username.isEmpty()) {
            this->username = username;
            socket->write((username + "\n").toUtf8());

            // Open & establish chat window
            chat = new Chat(socket, username);
            connect(chat, &Chat::gameStart, this, &Client::startGame);
            connect(chat, &Chat::gameEnd, this, &Client::endGame);
            chat->show();
            this->close();
        } else {
            ui->statusLabel->setText("Username entry canceled.");
            socket->disconnectFromHost();
        }
    }
}

void Client::onDisconnected()
{
    qDebug() << "Disconnected from the server.";
    ui->statusLabel->setText("Disconnected");
    chat->close();
    //this->show();
    gameDialog->accept();
}

void Client::onError(QAbstractSocket::SocketError error)
{
    qDebug() << "Socket error:" << socket->errorString();
    QMessageBox::critical(this, "Socket Error", socket->errorString());
    ui->statusLabel->setText("Error: " + socket->errorString());


}

void Client::endGame()
{
    if (gameDialog) {
        qDebug() << "Closing game dialog due to GAME_END message.";
        gameDialog->close();  // Close the game dialog
        gameDialog = nullptr;  // Optional: Reset pointer if necessary
    }
}

void Client::startGame()
{
    if (!gameDialog) {
        gameDialog = new GameDialog(this);  // Create the game dialog if it doesn't exist
        gameDialog->setAttribute(Qt::WA_DeleteOnClose);
    }

    // Connect the keyPressed signal to send movement data to the server
    connect(gameDialog, &GameDialog::keyPressed, this, [this](const QString &key) {
        if (username.isEmpty()) {
            qDebug() << "Username is not set. Cannot send PLAYERMOVE message.";
            return;
        }

        // Format the message to include the username
        QString message = "PLAYERMOVE: " + key + "\n";
        if (socket->state() == QAbstractSocket::ConnectedState) {
            socket->write(message.toUtf8());  // Send the message to the server
            socket->flush();  // Ensure the data is sent immediately
            qDebug() << "Sent to server:" << message;
        } else {
            qDebug() << "Socket not connected. Failed to send:" << message;
        }
    });

    // Show the game dialog as a non-modal dialog
    gameDialog->show();

    // Optional: Use a lambda to handle dialog closure
    connect(gameDialog, &GameDialog::finished, this, [this](int result) {
        if (result == QDialog::Accepted) {
            qDebug() << "Game closed successfully.";
        } else {
            qDebug() << "Game was closed with rejection or error.";
        }
    });

    qDebug() << "Game dialog displayed.";
}


