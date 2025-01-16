#ifndef GAME_H
#define GAME_H

#include <QDialog>
#include <QKeyEvent>

class GameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GameDialog(QWidget *parent = nullptr);
    ~GameDialog();

protected:
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void keyPressed(const QString &key);
};

#endif // GAME_H
