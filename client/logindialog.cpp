#include "logindialog.h"
#include <QLayout>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent, Qt::Window | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint)
{
    serverText = new QLineEdit(tr("127.0.0.1"));
    portText = new QLineEdit(tr("9999"));
    loginText = new QLineEdit(tr("User"));

    serverLabel = new QLabel(tr("Server: "));
    portLabel = new QLabel(tr("Port: "));
    loginLabel = new QLabel(tr("Login: "));

    loginButton = new QPushButton(tr("Login"));
    quitButton = new QPushButton(tr("Quit"));
    connect(loginButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(reject()));

    QGridLayout * loginLayout = new QGridLayout();
    loginLayout->addWidget(loginLabel, 0, 0);
    loginLayout->addWidget(loginText, 0, 1);

    loginLayout->addWidget(serverLabel, 1, 0);
    loginLayout->addWidget(serverText, 1, 1);

    loginLayout->addWidget(portLabel, 2, 0);
    loginLayout->addWidget(portText, 2, 1);

    QHBoxLayout * buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(quitButton);

    QVBoxLayout * mainLayout = new QVBoxLayout();

    mainLayout->addLayout(loginLayout);
    mainLayout->addLayout(buttonLayout);
    mainLayout->setSpacing(5);
    this->setLayout(mainLayout);
    this->setWindowTitle("Connect to Chat");
    this->resize(200, 150);
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}
