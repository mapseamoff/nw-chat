#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

MainWindow::MainWindow()
{
    chatEdit = new QTextEdit();
    messageEdit = new QTextEdit();
    sendButton = new QPushButton(tr("send"));
    QHBoxLayout * layout = new QHBoxLayout();
    layout->addWidget(messageEdit);
    layout->addWidget(sendButton);
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
}

void MainWindow::sentMessage() {

}

void MainWindow::update() {

}
