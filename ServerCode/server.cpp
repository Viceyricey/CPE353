// server.cpp

#include "server.h"
#include "game.h"
#include "ui_dialog.h"
#include "player.h"  // Include the Player header to recognize Player::Direction
#include <QFontDatabase>
#include <QDebug>
#include <QMessageBox>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <QInputDialog>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    tcpServer(new QTcpServer(this))
     //game(new Game(this))               // Initialize the TCP server
{
    ui->setupUi(this); // necessary lol
    //game->hide(); //hide inittially lol

    // Load custom font
    int fontId = QFontDatabase::addApplicationFont(":/ethnocentricrg.otf"); // loading in our own font
    QString fontFamily;
    if (fontId != -1) {
        fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
    } else {
        qWarning() << "Failed to load custom font."; // error output if font not loaded
    }

    QFont customFont(fontFamily, 12);  // Set the font size as needed

    // Set background color
    this->setStyleSheet("background-color: #000000;");

    // Apply font and styles to UI elements
    ui->logOutput->setStyleSheet("background-color: #282828; color: #00FFFF; border: 2px solid #00FFFF; font-size: 14px; padding: 5px;");
    ui->logOutput->setFont(customFont);

    ui->player1Label->setStyleSheet("color: #00FFFF; font-weight: bold;");
    ui->player1Label->setFont(customFont);
    ui->player2Label->setStyleSheet("color: #00FFFF; font-weight: bold;");
    ui->player2Label->setFont(customFont);
    ui->player3Label->setStyleSheet("color: #00FFFF; font-weight: bold;");
    ui->player3Label->setFont(customFont);
    ui->player4Label->setStyleSheet("color: #00FFFF; font-weight: bold;");
    ui->player4Label->setFont(customFont);

    ui->startServerButton->setStyleSheet("background-color: #00FFFF; color: black; font-weight: bold;");
    ui->startServerButton->setFont(customFont);
    ui->stopServerButton->setStyleSheet("background-color: #FF5733; color: black; font-weight: bold;");
    ui->stopServerButton->setFont(customFont);

    ui->player1KickButton->setStyleSheet("background-color: #FF5733; color: black; font-weight: bold;");
    ui->player1KickButton->setFont(customFont);
    ui->player2KickButton->setStyleSheet("background-color: #FF5733; color: black; font-weight: bold;");
    ui->player2KickButton->setFont(customFont);
    ui->player3KickButton->setStyleSheet("background-color: #FF5733; color: black; font-weight: bold;");
    ui->player3KickButton->setFont(customFont);
    ui->player4KickButton->setStyleSheet("background-color: #FF5733; color: black; font-weight: bold;");
    ui->player4KickButton->setFont(customFont);

    ui->player1ReadyBox->setStyleSheet("color: #00FFFF;");
    ui->player1ReadyBox->setFont(customFont);
    ui->player2ReadyBox->setStyleSheet("color: #00FFFF;");
    ui->player2ReadyBox->setFont(customFont);
    ui->player3ReadyBox->setStyleSheet("color: #00FFFF;");
    ui->player3ReadyBox->setFont(customFont);
    ui->player4ReadyBox->setStyleSheet("color: #00FFFF;");
    ui->player4ReadyBox->setFont(customFont);

    ui->lobbyLabel->setStyleSheet("color: #00FFFF; font-weight: bold;");
    ui->lobbyLabel->setFont(customFont);

    ui->groupBox->setStyleSheet("border: 2px solid #00FFFF; color: #00FFFF; font-weight: bold; padding: 5px;");
    ui->groupBox->setFont(customFont);

    // Connect the QTcpServer signal for new connections
    connect(tcpServer, &QTcpServer::newConnection, this, &Dialog::acceptConnection);
}


Dialog::~Dialog()
{
    delete ui; // destructor
    delete tcpServer;
    delete game;

    for(QTcpSocket* obj: playerSockets){
        delete obj;
    }
    playerSockets.clear();

    for(auto it = playerNames.cbegin(); it!=playerNames.cend(); ++it){
        QTcpSocket* tp = it.key();
        QString string = it.value();
        delete tp;
    }
    playerNames.clear();
}

void Dialog::on_startServerButton_clicked() {
    // Create the dialog
    QDialog dialog(this);
    dialog.setWindowTitle("Start Server");
    dialog.setModal(true);
    dialog.setStyleSheet("background-color: #000000; color: #00FFFF;"); // Set background and text color

    // Create layout
    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    // Add label
    QLabel *label = new QLabel("Enter Port Number (1000-9999):", &dialog);
    label->setStyleSheet("color: #00FFFF; font-weight: bold;");
    layout->addWidget(label);

    // Add input field
    QLineEdit *portInput = new QLineEdit(&dialog);
    portInput->setValidator(new QIntValidator(1000, 9999, portInput)); // Limit input to numbers between 1000 and 9999
    portInput->setStyleSheet(
        "background-color: #000000; "
        "color: #00FFFF; "
        "border: 1px solid #00FFFF; "
        "padding: 5px;"
        "font-weight: bold;"
    );
    layout->addWidget(portInput);

    // Add buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("OK", &dialog);
    okButton->setStyleSheet("background-color: #00FFFF; color: black; font-weight: bold;");
    QPushButton *cancelButton = new QPushButton("Cancel", &dialog);
    cancelButton->setStyleSheet("background-color: #FF5733; color: black; font-weight: bold;");
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    // Connect buttons
    connect(okButton, &QPushButton::clicked, [&dialog]() {
        dialog.accept();
    });
    connect(cancelButton, &QPushButton::clicked, [&dialog]() {
        dialog.reject();
    });

    // Execute dialog
    if (dialog.exec() == QDialog::Accepted) {
        // Get port value
        int port = portInput->text().toInt();
        if (tcpServer->listen(QHostAddress::Any, port)) {
            QString ipAddress;
            ui->startServerButton->setEnabled(false);
            ui->stopServerButton->setEnabled(true);

            const QList<QHostAddress> &allAddresses = QNetworkInterface::allAddresses();
            for (const QHostAddress &address : allAddresses) {
                if (address != QHostAddress::LocalHost && address.protocol() == QAbstractSocket::IPv4Protocol) {
                    ipAddress = address.toString();
                    break;
                }
            }

            if (ipAddress.isEmpty())
                ipAddress = QHostAddress(QHostAddress::LocalHost).toString();

            ui->logOutput->append("Server started on IP: " + ipAddress + ", Port: " + QString::number(port));
            qDebug() << "Server started on IP:" << ipAddress << ", Port:" << port;
        } else {
            QMessageBox::critical(this, "Error", "Server failed to start. Please try again.");
            ui->logOutput->append("Server failed to start.");
            qDebug() << "Server failed to start.";
        }
    } else {
        ui->logOutput->append("Server start canceled.");
        qDebug() << "Server start canceled by user.";
    }
}


void Dialog::broadcastMessage(const QByteArray &message)
{
    for (QTcpSocket *socket : playerSockets) {
        if (socket) {  // make sure that the socket is valid
            socket->write(message);   // send the message to that socket
            qDebug() << message;
            socket->flush();          // make sure that it's sent immediately
        }
    }
}

bool Dialog::getReadyStatus(int index) const // this function just checks to see if all of the players are ready
{
    switch (index) {
        case 0: return ui->player1ReadyBox->isChecked(); // check for player(s) 1-4 and whether or not they are ready
        case 1: return ui->player2ReadyBox->isChecked();
        case 2: return ui->player3ReadyBox->isChecked();
        case 3: return ui->player4ReadyBox->isChecked();
        default: return false;  // Return false if the index is out of range
    }
}

void Dialog::on_stopServerButton_clicked() // this function is fired when the server's stop button is clicked
{
    if (tcpServer->isListening()) { // make sure that the server is actually on first
        tcpServer->close(); // close the server
        ui->startServerButton->setEnabled(true); // enable the start server button
        ui->stopServerButton->setEnabled(false); // disable the stop server button

        foreach (QTcpSocket *socket, playerSockets) { // for every socket in the list of player sockets
            socket->disconnectFromHost(); // disconnect them
            socket->deleteLater();
        }
        playerSockets.clear(); // clear the list of player sockets
        playerNames.clear(); // clear the list of player names

        ui->logOutput->append("Server stopped."); // inform the user through output that the server has stopped
        qDebug() << "Server stopped."; // output same thing to terminal

        for (int i = 0; i < 4; ++i){ // clear all of the player labels
            clearPlayerLabel(i);
        }
    }
}

void Dialog::acceptConnection() // this function allows the server to accept the connections from the players
{
    if (playerSockets.count(nullptr) == 0 && playerSockets.size() >= 4) // if there are already 4 players
    {
        QTcpSocket *extraPlayer = tcpServer->nextPendingConnection(); // make a socket connection to that extra player
        extraPlayer->write("Lobby is full. Try again later."); // inform player that the lobby is full
        extraPlayer->disconnectFromHost(); // disconnect that player from the host
        qDebug() << "Connection refused: Lobby is full."; // output to qdebug that the connection was refused because the lobby is full
        return;
    }

    QTcpSocket *incomingClient = tcpServer->nextPendingConnection(); // when someone is trying to join make an incomingClient socket object
    // look for an empty slot in the playerSockets list (nullptr indicates an unused slot in the playerSockets list)
    int index = playerSockets.indexOf(nullptr);

    // if no empty slot is found (which means that the index is -1), add the incoming client to the end of the playerSockets list
    if (index == -1) {
        index = playerSockets.size();  // set index to the next available slot (end of the list)
        playerSockets.append(incomingClient); // add the incoming client to the list of player sockets
    }
    else {
        playerSockets[index] = incomingClient; // if an empty slot is found, reuse that slot for the new incoming client
    }

    connect(incomingClient, &QTcpSocket::disconnected, this, &Dialog::onPlayerDisconnected); // make connect statements to detect the signal of a player disconnecting
    connect(incomingClient, &QTcpSocket::readyRead, this, &Dialog::onReadyRead); // detect signal of player/client sending packets

    qDebug() << "Player connected at index:" << index; // sending to server qDebug that a player connected and at what index
}

void Dialog::onPlayerDisconnected() // function for when a player disconnects
{
    QTcpSocket *playerSocket = qobject_cast<QTcpSocket*>(sender()); // make a player socket variable
    QString playerName = playerNames.value(playerSocket, "Unknown"); // pull the player name into this socket

    int index = playerSockets.indexOf(playerSocket); // getting the index of that player's socket
    if (index != -1) {
        clearPlayerLabel(index);
        playerSockets[index] = nullptr;
    }

    playerNames.remove(playerSocket); // removing this player from the list of player names
    playerSocket->deleteLater(); // queue up the socket to be deleted

    ui->logOutput->append(playerName + " disconnected."); // log the output to the server to show that the player disconnected
}

void Dialog::onReadyRead()
{
    QTcpSocket *playerSocket = qobject_cast<QTcpSocket *>(sender());
    if (!playerSocket) return; // If playerSocket is null, exit

    QString data = QString::fromUtf8(playerSocket->readAll()).trimmed(); // Read the raw data from the client

    qDebug() << "Received data from client:" << data;

    // Check if the player's name has been set yet
    if (!playerNames.contains(playerSocket)) {
        // Treat the first message as the player's name
        QString playerName = data;
        playerNames[playerSocket] = playerName;

        int index = playerSockets.indexOf(playerSocket);
        if (index != -1) {
            setPlayerLabel(index, playerName);  // Update the UI with the player's name
        }

        // Broadcast the player's joining message
        QString joinMessage = playerName + " has joined the game.";
        broadcastMessage(joinMessage.toUtf8());
        ui->logOutput->append(joinMessage);
        qDebug() << joinMessage;

        return;
    }

    QString playerName = playerNames.value(playerSocket, "Unknown"); // Get the player's name

    // Check if the data represents a movement command
    if (data.startsWith("PLAYERMOVE:")) {
        QString direction = data.mid(11).trimmed(); // Extract the direction after "PLAYERMOVE: "
        if (!direction.isEmpty()) {
            // Validate and process the movement
            if (game) {
                qDebug() << "Processing movement for" << playerName << "in direction:" << direction;
                game->processClientInput(playerName, direction);
            } else {
                qWarning() << "Game instance does not exist. Cannot process movement.";
            }
        } else {
            qWarning() << "Invalid PLAYERMOVE format (direction missing):" << data;
        }
    }

    // Handle other types of messages (READY, CHAT, etc.)
    else if (data.startsWith("READY:")) {
        setPlayerReadyStatus(playerSocket, playerName, true);
    } else if (data.startsWith("NOT_READY:")) {
        setPlayerReadyStatus(playerSocket, playerName, false);
    } else if (data.startsWith("CHAT:")) {
        QString chatMessage = data.mid(5).trimmed();
        QString fullMessage = playerName + ": " + chatMessage;
        broadcastMessage(fullMessage.toUtf8());
        ui->logOutput->append(fullMessage);
    } else {
        qDebug() << "Received unknown data:" << data;
    }
}



void Dialog::setPlayerLabel(int index, const QString &playerName) // this function is simply to set the labels for each associated player
{
    switch (index) {
        case 0: ui->player1Label->setText(playerName);
                ui->player1KickButton->setEnabled(true);
                ui->player1ReadyBox->setCheckable(true);
                break;
        case 1: ui->player2Label->setText(playerName);
                ui->player2KickButton->setEnabled(true);
                ui->player2ReadyBox->setCheckable(true);
                break;
        case 2: ui->player3Label->setText(playerName);
                ui->player3KickButton->setEnabled(true);
                ui->player3ReadyBox->setCheckable(true);
                break;
        case 3: ui->player4Label->setText(playerName);
                ui->player4KickButton->setEnabled(true);
                ui->player4ReadyBox->setCheckable(true);
                break;
    }
}

void Dialog::clearPlayerLabel(int index) // this function is simply to just clear the label of each associated player
{
    switch (index) {
        case 0: ui->player1Label->setText("Empty");
                ui->player1KickButton->setEnabled(false);
                ui->player1ReadyBox->setCheckable(false);
                break;
        case 1: ui->player2Label->setText("Empty");
                ui->player2KickButton->setEnabled(false);
                ui->player2ReadyBox->setCheckable(false);
                break;
        case 2: ui->player3Label->setText("Empty");
                ui->player3KickButton->setEnabled(false);
                ui->player3ReadyBox->setCheckable(false);
                break;
        case 3: ui->player4Label->setText("Empty");
                ui->player4KickButton->setEnabled(false);
                ui->player4ReadyBox->setCheckable(false);
                break;
    }
}

void Dialog::broadcastPlayerStates() { // this is the function for broadcasting messages to all of the players
    QString playerData;
    QString fullPacket;

    int index = 0;
    for (QTcpSocket* playerSocket : playerSockets) { // for each socket listed in the playerSocket list
        if (playerSocket && playerNames.contains(playerSocket)) { // if the socket is in the list
            QString username = playerNames.value(playerSocket); // get the username

            // Add 1 to the index to make it start from 1 instead of 0
            playerData += QString("%1:%2; ").arg(username).arg(index + 1);
            fullPacket = "LAYOUT: " + playerData;
            index++;
        }
    }

    qDebug() << "Broadcasting player data:" << playerData;  // Debug for verification

    // Send the player data as a plain text message to all clients
    //broadcastMessage(fullPacket.toUtf8());
}




void Dialog::checkAllPlayersReady() // function to check whether all players are ready or not
{
    bool allReady = true; // set all ready to true
    for (int i = 0; i < playerSockets.size(); ++i) {
        if (playerSockets[i] && !getReadyStatus(i)) { // if not all of the player sockets are ready
            allReady = false; // set allReady to false
            break;
        }
    }

    if (allReady) { // if they are all ready then start the match
        startMatch();
    }
}

void Dialog::startMatch()
{
    // Check if there are at least 2 players
    int activePlayers = 0;
    for (QTcpSocket *socket : playerSockets) {
        if (socket) {
            ++activePlayers;
        }
    }

    if (activePlayers < 2) {
        ui->logOutput->append("Not enough players to start the game. At least 2 players are required.");
        qDebug() << "Not enough players to start the game. At least 2 players are required.";
        return;
    }

    // Clear any pending packets
    for (QTcpSocket *socket : playerSockets) {
        if (socket && socket->bytesAvailable() > 0) {
            socket->readAll(); // Discard any pending data
        }
    }

    QByteArray gameStartMessage = "GAME_START";
    broadcastMessage(gameStartMessage);

    // Dynamically create the game instance and assign it to `game`
    game = new Game(this);
    game->setModal(false); // Make it non-modal
    game->show();

    // Add all players to the game
    for (const QString &playerName : playerNames.values()) {
        game->addPlayer(playerName);
    }

    connect(game, &Game::gameEnded, this, &Dialog::onGameEnded);

    ui->logOutput->append("Game started!");
    qDebug() << "Game started! Cleared pending packets and broadcasted start of game.";
}


void Dialog::onGameEnded()
{
    QByteArray gameEndMessage = "GAME_END";
    broadcastMessage(gameEndMessage);
    qDebug() << gameEndMessage;

    ui->logOutput->append("Game ended!");
    //qDebug() << "Game ended! Kicking all players and broadcasting game end message.";
    //kickPlayer(0);
    //kickPlayer(1);
    //kickPlayer(2);
    //kickPlayer(3);


    game->accept(); // End the game window
    //this->~Dialog();

    qDebug() << "All players have been disconnected.";
}


void Dialog::setPlayerReadyStatus(QTcpSocket *playerSocket, const QString &playerName, bool isReady) // this functions sets the ready status of the players
{
    int index = playerSockets.indexOf(playerSocket); // get the current player
    if (index == -1) {
        qDebug() << "Player" << playerName << "not found.";
        return;
    }

    // Update the UI to reflect the player's ready status
    switch (index) { // if any of these are checkd
        case 0: ui->player1ReadyBox->setChecked(isReady); break;
        case 1: ui->player2ReadyBox->setChecked(isReady); break;
        case 2: ui->player3ReadyBox->setChecked(isReady); break;
        case 3: ui->player4ReadyBox->setChecked(isReady); break;
    }

    QString status = isReady ? "ready" : "not ready"; // this is an if-else apparently. set them to either ready or not ready
    ui->logOutput->append(playerName + " is " + status + "."); // output this to the server
    qDebug() << playerName << "is" << status << ".";

    // Check if all players are ready after updating the status
    checkAllPlayersReady(); // at the end of setting the player's ready status, check to see if all of the players are ready
}

void Dialog::kickPlayer(int index) // This function kicks a player without prompting
{
    if (index < 0 || index >= playerSockets.size() || !playerSockets[index]) { // Validate index
        qDebug() << "Invalid player index for kicking.";
        return;
    }

    QTcpSocket *playerSocket = playerSockets.at(index); // Get the player's socket
    QString playerName = playerNames.value(playerSocket, "Unknown"); // Get the player's name

    playerSocket->write("You have been kicked");  // Inform the player they have been kicked
    playerSocket->flush();

    QTimer::singleShot(100, this, [this, playerSocket, playerName, index]() { // Set a timer for safe disconnection
        playerSocket->disconnectFromHost(); // Disconnect the player from the server
        playerSockets[index] = nullptr; // Clear the player's socket in the list
        playerNames.remove(playerSocket); // Remove their name from the list
        playerSocket->deleteLater(); // Queue the socket for deletion

        clearPlayerLabel(index); // Clear the player's label in the UI
        ui->logOutput->append(playerName + " has been kicked from the lobby."); // Log the kick action
        qDebug() << playerName << "has been kicked from the lobby.";
    });
}


void Dialog::on_player1KickButton_clicked()
{
    kickPlayer(0); // Kick the player at index 0
}

void Dialog::on_player2KickButton_clicked()
{
    kickPlayer(1); // Kick the player at index 1
}

void Dialog::on_player3KickButton_clicked()
{
    kickPlayer(2); // Kick the player at index 2
}

void Dialog::on_player4KickButton_clicked()
{
    kickPlayer(3); // Kick the player at index 3
}
