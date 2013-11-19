#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QGridLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    chatEdit = new QTextEdit(this);
    messageEdit = new QTextEdit(this);
    sendButton = new QPushButton(tr("Send"), this);
    connectButton = new QPushButton(tr("Connect"), this);
    loginDialog = new LoginDialog(this);
    userList = new QListWidget(this);

    connect(sendButton, SIGNAL(clicked()), this, SLOT(sendMessage()));
    connect(connectButton, SIGNAL(clicked()), this, SLOT(showLoginDialog()));

//    QHBoxLayout * layout = new QHBoxLayout();
//    layout->addWidget(messageEdit);
//    layout->addWidget(sendButton);
//    layout->addWidget(connectButton);

    QGridLayout * mainLayout = new QGridLayout();
    mainLayout->addWidget(chatEdit, 0, 0);
    mainLayout->addWidget(userList, 0, 1);
    mainLayout->addWidget(messageEdit, 1, 0, 3, 1);
    mainLayout->addWidget(sendButton, 1, 1);
    mainLayout->addWidget(connectButton, 3, 1);
//    mainLayout->addLayout(layout, 1, 0, 1, 2);
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(5, 5, 5, 5);

    QWidget *w = new QWidget(this);
    w->setLayout(mainLayout);
    w->setWindowTitle(tr("Chat (Disconnected)"));
    this->setCentralWidget(w);
    this->resize(400, 400);
    userList->setMaximumWidth(100);
    messageEdit->setMaximumHeight(60);
    messageEdit->setFocus();
    chatEdit->setReadOnly(true);
    sendButton->setDisabled(true);

    client = new ChatClient(this);
    connect(client, SIGNAL(connectedToChat(bool)), this, SLOT(connectedToChat(bool)));
    connect(client, SIGNAL(loginResult(bool)), this, SLOT(loggedIn(bool)));
    connect(client, SIGNAL(socketError()), this, SLOT(socketError()));
    connect(client, SIGNAL(protocolError()), this, SLOT(protocolError()));
    connect(client, SIGNAL(gotMessage(QString)), this, SLOT(newMessage(QString)));
    connect(client, SIGNAL(gotUserList(QString)), this, SLOT(updateUserList(QString)));

    fetchTimer = new QTimer(this);
    connect(fetchTimer, SIGNAL(timeout()), this, SLOT(fetchMessages()));

    userListTimer = new QTimer(this);
    connect(userListTimer, SIGNAL(timeout()), this, SLOT(fetchUserList()));

    userName = "";

    showLoginDialog();
}

MainWindow::~MainWindow() {
    delete client;
}

void MainWindow::sendMessage() {
    QString msg = messageEdit->toPlainText();
    if (msg == "") {
        return;
    }
    if(msg == "/exit") {
        client->logoutUser();
    }
    else if(msg == "/list") {
        client->getUserList();
    } else {
        lastMessage = QString("[%1] %2").arg(userName).arg(msg);
        client->sendMessage(msg);
    }
    messageEdit->clear();
    fetchTimer->start();
}

void MainWindow::connectedToChat(bool ok) {
    if(!ok) QMessageBox::critical(this, "Error", "Unable to connect: " + client->getLastError());
    else client->loginUser(userName);
}

void MainWindow::loggedIn(bool ok) {
    if(!ok) {
        QMessageBox::critical(this, "Error", "Unable to connect: " + client->getLastError());
    } else {
        sendButton->setEnabled(true);
        connectButton->setText("Disconnect");
        disconnect(connectButton, SIGNAL(clicked()), this, SLOT(showLoginDialog()));
        connect(connectButton, SIGNAL(clicked()), this, SLOT(disconnectFromChat()));
        this->setWindowTitle("Chat (Connected)");
        chatEdit->append("Welcome, " + userName + "!");
        fetchUserList();
        fetchTimer->start(1000);
        userListTimer->start(10000);
    }
}

void MainWindow::showLoginDialog() {
    if (loginDialog->exec() == QDialog::Accepted) {
        QString server = loginDialog->serverText->text();
        QString port = loginDialog->portText->text();
        userName = loginDialog->loginText->text();
        client->connectToChat(server, port);
    } else {
        exit(0);
    }
}

void MainWindow::disconnectFromChat() {
    fetchTimer->stop();
    userListTimer->stop();
    client->logoutUser();
    sendButton->setEnabled(false);
    connectButton->setText("Connect");
    connect(connectButton, SIGNAL(clicked()), this, SLOT(showLoginDialog()));
    disconnect(connectButton, SIGNAL(clicked()), this, SLOT(disconnectFromChat()));
}

void MainWindow::socketError() {
    QMessageBox::critical(this, "Critical error", client->getLastError());
    if(client->connected()) disconnectFromChat();
}

void MainWindow::protocolError() {
    chatEdit->append(client->getLastError());
}

void MainWindow::newMessage(QString msg) {
    if(msg.isEmpty()) chatEdit->append(lastMessage);
    else chatEdit->append(msg);
}

void MainWindow::fetchMessages() {
    client->fetchMessages();
}

void MainWindow::fetchUserList() {
    client->getUserList();
}

void MainWindow::updateUserList(QString list) {
    QStringList users = list.split('\n');
    userList->clear();
    userList->addItems(users);
}
