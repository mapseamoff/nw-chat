#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

MainWindow::MainWindow()
{
    chatEdit = new QTextEdit();
    messageEdit = new QTextEdit();
    sendButton = new QPushButton(tr("send"));
    connectButton = new QPushButton(tr("Connect"));
    connect(sendButton, SIGNAL(clicked()), this, SLOT(sendMessage()));
    connect(connectButton, SIGNAL(clicked()), this, SLOT(connectWindow()));

    QHBoxLayout * layout = new QHBoxLayout();
    layout->addWidget(messageEdit);
    layout->addWidget(sendButton);
    layout->addWidget(connectButton);
    QVBoxLayout * mainLayout = new QVBoxLayout();
    mainLayout->addWidget(chatEdit);
    mainLayout->addLayout(layout);
    QWidget *w = new QWidget(this);
    w->setLayout(mainLayout);
    w->setWindowTitle("Chat");
    this->setCentralWidget(w);
    this->resize(400, 400);
    messageEdit->setFixedHeight(40);
    messageEdit->setFocus();
    chatEdit->setReadOnly(true);

    connectWindow();
}

MainWindow::~MainWindow() {
   // delete client;
}

void MainWindow::sendMessage() {/*
    std::string msg;
    if(msg == "/exit") {
        client->logoutUser();
    }
    else if(msg == "/list") {
        client->getUserList();
    } else {
        client->sendMessage(msg);
    }*/
}

void MainWindow::update() {

}

void MainWindow::connectWindow() {
    LoginDialog * dialog = new LoginDialog();
    if (dialog->exec() == QDialog::Accepted) {
    /*    boost::mutex::scoped_lock lock(clientLock);
        std::string server = dialog->serverText->text().toStdString();
        std::string login = dialog->loginText->text().toStdString();
        std::string port = dialog->portText->text().toStdString();

        boost::asio::io_service io_service;
        tcp::resolver resolver(io_service);

        tcp::resolver::query query(server, port);
        tcp::resolver::iterator iterator = resolver.resolve(query);
        boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));

        client = new ChatClient(io_service, iterator, waitClient);
        waitClient.wait(lock);
        if(!client->connected()) {
            chatEdit->setText("Unable to connect ");
            return;
        }

        client->loginUser(login);
        waitClient.wait(lock);
        if(!client->isLoggedIn()) {
            chatEdit->setText("Unable to login");
            return;
        }
        chatEdit->setText("Hello! ");*/
    } else {
        exit(0);
    }
    chatEdit->setText("eeee ");
}
