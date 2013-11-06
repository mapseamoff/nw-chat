#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H
#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

class LoginDialog: public QDialog {
    Q_OBJECT

public:
    LoginDialog();

private:
    QLineEdit * loginText;
    QLineEdit * serverText;
    QLineEdit * portText;

    QLabel * loginLabel;
    QLabel * serverLabel;
    QLabel * portLabel;

    QPushButton * loginButton;
    QPushButton * quitButton;

private slots:
    void quit();
    void login();
};

#endif // LOGINDIALOG_H
