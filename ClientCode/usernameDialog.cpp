#include "usernameDialog.h"

UsernameDialog::UsernameDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Enter Username/Gamertag");

    // Create and style the username input field
    usernameInput = new QLineEdit(this);
    usernameInput->setStyleSheet(
        "background-color: #333333;"       // Dark background color
        "color: #00FFFF;"                  // Cyan text color
        "border: 2px solid #00FFFF;"       // Cyan border
        "font-size: 18px;"                 // Larger font size for readability
        "font-weight: bold;"               // Bold text
        "padding: 5px;"                    // Add padding to make it visually larger
        "min-width: 250px;"                // Minimum width to increase box size
    );
    usernameInput->setPlaceholderText("Username");

    // Create and style the OK button
    okButton = new QPushButton("OK", this);
    okButton->setStyleSheet("background-color: #00FFFF; color: black; font-weight: bold;");

    // Connect the OK button to accept the dialog
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);

    // Set up the layout and add the input field and button
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(usernameInput);
    layout->addWidget(okButton);

    setLayout(layout);
}

// Function to retrieve the entered username
QString UsernameDialog::getUsername() const
{
    return usernameInput->text();
}
