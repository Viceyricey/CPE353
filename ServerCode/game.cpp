#include "game.h"
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QTimer>
#include <QPen>
#include <QDebug>
#include <QGraphicsItem>

// Constants for the game
constexpr int SCENE_WIDTH = 800;
constexpr int SCENE_HEIGHT = 600;
constexpr int PLAYER_WIDTH = 20;
constexpr int PLAYER_HEIGHT = 20;
constexpr qreal PLAYER_SPEED = 1.5;

Game::Game(QWidget *parent)
    : QDialog(parent),
      scene(new QGraphicsScene(-SCENE_WIDTH / 2, -SCENE_HEIGHT / 2, SCENE_WIDTH, SCENE_HEIGHT, this))
{
    setWindowTitle("Game");
    resize(900, 700);

    // Set up the scene
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setBackgroundBrush(Qt::black);

    // Create and configure the view
    view = new QGraphicsView(scene, this);
    view->setRenderHint(QPainter::Antialiasing);
    view->setFixedSize(900, 700);
    view->move(0, 0);

    // Draw the perimeter lines
    drawPerimeterLines();

    // Timer for advancing the game state
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Game::advance);
    timer->start(10); // 10 ms interval

    initializeDatabase();

}

Game::~Game()
{
    delete view;
    delete scene;

    // Deallocate the players
    for( auto it = players.cbegin(); it != players.cend(); ++it){
        QString playerName = it.key();
        QGraphicsRectItem *rect =it.value();
        delete rect;
    }
    players.clear();

    // Deallocate trails
    for (QGraphicsRectItem* obj : trailItems) { delete obj; }
    trailItems.clear();

    playerVelocities.clear();

    for (auto it = playerFrontRectangles.cbegin(); it != playerFrontRectangles.cend(); ++it) {
        QString playerName = it.key();
        QGraphicsRectItem *rect = it.value();
        delete rect;
    }
    playerFrontRectangles.clear();

    // Cleanup database when the game ends
    if (db.isOpen()) {
        db.close();
    }

    displayLossOrder();

}

void Game::addPlayer(const QString &playerName)
{
    if (players.contains(playerName)) {
        qWarning() << "Player" << playerName << "already exists.";
        return;
    }

    // Define a list of colors for players
    static QVector<QColor> predefinedColors = {Qt::blue, QColor(255, 165, 0), Qt::green, Qt::red}; // Blue, Orange, Green, Red

    // Determine the color for the player based on the number of existing players
    QColor playerColor = predefinedColors[players.size() % predefinedColors.size()];

    // Store the player's color
    playerColors[playerName] = playerColor;

    // Create a new rectangle for the player
    QGraphicsRectItem *playerRect = scene->addRect(-PLAYER_WIDTH / 2, -PLAYER_HEIGHT / 2, PLAYER_WIDTH, PLAYER_HEIGHT);
    playerRect->setPen(QPen(Qt::white));
    playerRect->setBrush(playerColor); // Set the player's color

    // Position players in a line at the start
    QPointF initialPosition(players.size() * 120 - SCENE_HEIGHT / 4 - 20, -250);
    playerRect->setPos(initialPosition);

    players[playerName] = playerRect;
    playerVelocities[playerName] = QPointF(0, 0);

    // Add a small rectangle in front of the player
    QGraphicsRectItem *frontRect = scene->addRect(-10, -10, 20, 5, QPen(Qt::transparent), QBrush(Qt::transparent));
    playerFrontRectangles[playerName] = frontRect;

    qDebug() << "Added player:" << playerName << "at position" << initialPosition << "with color" << playerColor.name();
}



void Game::processClientInput(const QString &playerName, const QString &keyInput)
{
    if (!players.contains(playerName)) {
        qWarning() << "Unknown player:" << playerName;
        return;
    }

    QPointF velocity(0, 0);
    qreal rotation = 0; // Rotation angle for the front rectangle

    if (keyInput == "W") {
        velocity.setY(-PLAYER_SPEED);
        rotation = 0; // Facing up
    } else if (keyInput == "S") {
        velocity.setY(PLAYER_SPEED);
        rotation = 180; // Facing down
    } else if (keyInput == "A") {
        velocity.setX(-PLAYER_SPEED);
        rotation = 270; // Facing left
    } else if (keyInput == "D") {
        velocity.setX(PLAYER_SPEED);
        rotation = 90; // Facing right
    } else {
        qWarning() << "Unknown key input from player" << playerName << ":" << keyInput;
        return;
    }

    playerVelocities[playerName] = velocity;

    // Update front rectangle rotation
    if (playerFrontRectangles.contains(playerName)) {
        QGraphicsRectItem *frontRect = playerFrontRectangles[playerName];
        frontRect->setRotation(rotation); // Rotate the front rectangle
    }

    qDebug() << "Updated velocity for player" << playerName << "to" << velocity << "with rotation" << rotation;
}


void Game::advance()
{
    bool allFrozen = true; // Assume all players are frozen initially
    QString activePlayer;  // Keep track of the last active player
    int activePlayerCount = 0; // Count the number of active players

    for (auto it = players.begin(); it != players.end(); ++it) {
        QString playerName = it.key();
        QGraphicsRectItem *playerRect = it.value();

        // Skip processing for frozen players
        if (frozenPlayers.contains(playerName)) {
            continue;
        }

        allFrozen = false; // At least one player is active
        activePlayer = playerName; // Update the active player
        ++activePlayerCount; // Increment the active player count

        QPointF velocity = playerVelocities[playerName];

        // Update the player's position
        qreal x = playerRect->pos().x() + velocity.x();
        qreal y = playerRect->pos().y() + velocity.y();

        // Leave a trail behind the player
        if (!velocity.isNull()) {
            leaveTrail(playerRect, currentDirection, playerColors[playerName]); // Use player's color for the trail
        }

        // Clamp position to stay within scene bounds
        x = qBound(-SCENE_WIDTH / 2 + static_cast<qreal>(PLAYER_WIDTH) / 2, x, SCENE_WIDTH / 2 - static_cast<qreal>(PLAYER_WIDTH) / 2);
        y = qBound(-SCENE_HEIGHT / 2 + static_cast<qreal>(PLAYER_HEIGHT) / 2, y, SCENE_HEIGHT / 2 - static_cast<qreal>(PLAYER_HEIGHT) / 2);
        playerRect->setPos(x, y);

        // Update front rectangle position and rotation
        if (playerFrontRectangles.contains(playerName)) {
            QGraphicsRectItem *frontRect = playerFrontRectangles[playerName];

            // Determine the offset for the front rectangle based on velocity
            QPointF frontOffset(0, 0);
            qreal rotation = 0;
            if (velocity.y() < 0) { // Moving up
                frontOffset = QPointF(0, -PLAYER_HEIGHT / 2 + 6);
                rotation = 0;
            } else if (velocity.y() > 0) { // Moving down
                frontOffset = QPointF(0, PLAYER_HEIGHT / 2 + 6);
                rotation = 180;
            } else if (velocity.x() < 0) { // Moving left
                frontOffset = QPointF(-PLAYER_WIDTH / 2, 7);
                rotation = 270;
            } else if (velocity.x() > 0) { // Moving right
                frontOffset = QPointF(PLAYER_WIDTH / 2 - 2, 7);
                rotation = 90;
            }

            // Update front rectangle's position and rotation
            frontRect->setPos(playerRect->pos() + frontOffset);
            frontRect->setRotation(rotation);
            frontRect->setTransformOriginPoint(frontRect->rect().center());

            // Check for collisions with trails
            for (auto *trailSegment : trailItems) {
                if (frontRect->collidesWithItem(trailSegment)) {
                    qDebug() << "Player" << playerName << "collided with a trail!";
                    frozenPlayers.insert(playerName); // Freeze the player
                    playerVelocities[playerName] = QPointF(0, 0); // Stop the player's movement

                    // Record the player's loss order in the database
                    recordPlayerLoss(playerName, lossCounter++);

                    // Update the player's color to indicate collision
                    if (playerColors.contains(playerName)) {
                        QColor collisionColor = Qt::gray; // Use gray to indicate frozen state
                        playerColors[playerName] = collisionColor;
                        playerRect->setBrush(collisionColor);
                    }
                    break; // Exit collision detection for this player
                }
            }

            // Check for collisions with the border
            for (auto *border : borderItems) {
                if (frontRect->collidesWithItem(border)) {
                    qDebug() << "Player" << playerName << "collided with the border!";
                    frozenPlayers.insert(playerName); // Freeze the player
                    playerVelocities[playerName] = QPointF(0, 0); // Stop the player's movement

                    // Record the player's loss order in the database
                    recordPlayerLoss(playerName, lossCounter++);

                    // Update the player's color to indicate collision
                    if (playerColors.contains(playerName)) {
                        QColor collisionColor = Qt::gray; // Use gray to indicate frozen state
                        playerColors[playerName] = collisionColor;
                        playerRect->setBrush(collisionColor);
                    }
                    break; // Exit collision detection for this player
                }
            }
        }
    }

    // Check if only one player is active
    if (activePlayerCount == 1 && !hasGameEnded) {
        qDebug() << activePlayer << "wins!";

        // Show the winner message
        QMessageBox winnerBox;
        winnerBox.setStyleSheet("background-color: #000000; color: #00FFFF; font-weight: bold; font-size: 18px; border: 2px solid #00FFFF;");
        winnerBox.setWindowTitle("Game Over");
        winnerBox.setText(activePlayer + " wins!");
        winnerBox.setStandardButtons(QMessageBox::Ok);
        winnerBox.exec();

        hasGameEnded = true;

        // Record winner as last standing
        recordPlayerLoss(activePlayer, lossCounter++);

        // Show the final scores
        displayLossOrder();

        emit gameEnded();
        return;
    }

    // Check if all players are frozen and emit the gameEnded signal
    if (allFrozen && !hasGameEnded) {
        qDebug() << "All players are frozen. Game over!";
        hasGameEnded = true;

        // Display the final scores
        displayLossOrder();

        emit gameEnded();
    }
}


void Game::leaveTrail(QGraphicsRectItem *playerRect, Direction direction, const QColor &color)
{
    int trailWidth = 10;
    int trailHeight = 10;

    // Calculate offset based on direction
    int offsetX = 0, offsetY = 0;
    switch (direction) {
        case Up:    offsetY = PLAYER_HEIGHT / 2; offsetX = 0; break;
        case Down:  offsetY = -PLAYER_HEIGHT / 2; offsetX = 0; break;
        case Left:  offsetX = PLAYER_WIDTH / 2; offsetY = 0; break;
        case Right: offsetX = -PLAYER_WIDTH / 2; offsetY = 0; break;
    }

    // Calculate trail segment position
    qreal trailX = playerRect->x();
    qreal trailY = playerRect->y();



    // Create a trail segment
    QGraphicsRectItem *trailSegment = new QGraphicsRectItem(trailX - trailWidth / 2, trailY - trailHeight / 2, trailWidth, trailHeight);
    trailSegment->setBrush(QBrush(color)); // Use the passed color for the trail
    trailSegment->setPen(Qt::NoPen);       // No border for the trail

    // Add to the scene and keep track of it
    if (scene) {
        scene->addItem(trailSegment);
        trailItems.append(trailSegment);
    }

    // Start a timer to shrink and remove the trail segment
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this, trailSegment, timer]() {
        QRectF rect = trailSegment->rect();
        if (rect.width() > 0 && rect.height() > 0) {
            // Shrink the trail segment
            rect.adjust(1, 1, -1, -1); // Reduce size from all sides
            trailSegment->setRect(rect);
        } else {
            // Remove the trail segment when it's fully shrunk
            trailItems.removeOne(trailSegment);
            scene->removeItem(trailSegment);
            delete trailSegment;
            timer->stop();
            timer->deleteLater();
        }
    });
    timer->start(2500); // Shrink every 2500ms
}


void Game::changeDirection(Direction newDirection) {
    // Prevent the player from reversing direction directly
    if ((currentDirection == Up && newDirection != Down) ||
        (currentDirection == Down && newDirection != Up) ||
        (currentDirection == Left && newDirection != Right) ||
        (currentDirection == Right && newDirection != Left)) {
        currentDirection = newDirection;
    }
}



void Game::drawPerimeterLines()
{
    QBrush brush(Qt::blue); // Use a blue brush for the border
    int thickness = 5; // Thickness of the border rectangles

    // Top border
    borderItems.append(scene->addRect(-SCENE_WIDTH / 2, -SCENE_HEIGHT / 2 - thickness / 2, SCENE_WIDTH, thickness, Qt::NoPen, brush));

    // Bottom border
    borderItems.append(scene->addRect(-SCENE_WIDTH / 2, SCENE_HEIGHT / 2 - thickness / 2, SCENE_WIDTH, thickness, Qt::NoPen, brush));

    // Left border
    borderItems.append(scene->addRect(-SCENE_WIDTH / 2 - thickness / 2, -SCENE_HEIGHT / 2, thickness, SCENE_HEIGHT, Qt::NoPen, brush));

    // Right border
    borderItems.append(scene->addRect(SCENE_WIDTH / 2 - thickness / 2, -SCENE_HEIGHT / 2, thickness, SCENE_HEIGHT, Qt::NoPen, brush));
}

void Game::initializeDatabase() {
    // Create an in-memory SQLite database
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");  // In-memory database

    if (!db.open()) {
        qCritical() << "Error: Unable to open in-memory database";
        return;
    }

    // Create a table to store player names and their loss order
    QSqlQuery query;
    if (!query.exec("CREATE TABLE player_losses (player_name TEXT, loss_order INTEGER)")) {
        qCritical() << "Error: Unable to create table" << query.lastError();
    }
}

void Game::recordPlayerLoss(const QString &playerName, int lossOrder)
{
    // Insert the player name and loss order into the table
    QSqlQuery query;
    query.prepare("INSERT INTO player_losses (player_name, loss_order) VALUES (:player_name, :loss_order)");
    query.bindValue(":player_name", playerName);
    query.bindValue(":loss_order", lossOrder);

    if (!query.exec()) {
        qCritical() << "Error: Unable to record loss" << query.lastError();
    }

    qDebug() << "Recorded loss for player" << playerName << "with order" << lossOrder;
}

void Game::displayLossOrder()
{
    // Adjust point mapping based on the number of players
    QMap<int, int> pointsMap;

    int totalPlayers = players.size();  // get the total number of players
    if (totalPlayers == 2)
    {
        pointsMap = {{2, 1}, {1, 10}};  // map 10 points to first place
    }
    else if (totalPlayers == 3)
    {
        pointsMap = {{3, 1}, {2, 2}, {1, 10}};  // map 10 points to 1st, 1 to last, and 2 to 2nd
    }
    else if (totalPlayers == 4)
    {
        pointsMap = {{4, 1}, {3, 2}, {2, 5}, {1, 10}};  // do more of the same
    }

    QSqlQuery query("SELECT * FROM player_losses ORDER BY loss_order DESC");  // make sure to list in descending order

    QString scoreDisplay = "Player Scores:\n";
    int placement = 1;

    while (query.next()) {
        QString playerName = query.value(0).toString();
        int points = pointsMap.value(placement, 0);  // default points to 0

        // make sure that the players and their scores are listed correctly
        QString placeSuffix;
        switch (placement)
        {
            case 1: placeSuffix = "1st Place"; break;
            case 2: placeSuffix = "2nd Place"; break;
            case 3: placeSuffix = "3rd Place"; break;
            case 4: placeSuffix = "4th Place"; break;
            default: placeSuffix = QString::number(placement) + "th Place"; break;
        }

        scoreDisplay += QString("%1: %2 points (%3)\n")
                            .arg(playerName)
                            .arg(points)
                            .arg(placeSuffix);

        placement++;
    }

    // Display the final scores
    QMessageBox scoreBox;
    scoreBox.setStyleSheet("background-color: #000000; color: #00FFFF; font-weight: bold; font-size: 18px; border: 2px solid #00FFFF;");
    scoreBox.setWindowTitle("Game Over - Final Scores");
    scoreBox.setText(scoreDisplay);
    scoreBox.setStandardButtons(QMessageBox::Ok);
    scoreBox.exec();
}




