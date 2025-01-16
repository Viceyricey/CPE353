//usernameDialog.h

#ifndef USERNAMEDIALOG_H
#define USERNAMEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QString>
#include <QWidget>

class UsernameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UsernameDialog(QWidget *parent = nullptr);
    QString getUsername() const;

private:
    QLineEdit *usernameInput;
    QPushButton *okButton;
};

#endif // USERNAMEDIALOG_H
