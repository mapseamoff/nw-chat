#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H
#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

class LoginDialog: public QDialog {
    Q_OBJECT

public:
    LoginDialog(QWidget *parent = 0);
    QLineEdit * loginText;
    QLineEdit * serverText;
    QLineEdit * portText;

private:
    QLabel * loginLabel;
    QLabel * serverLabel;
    QLabel * portLabel;

    QPushButton * loginButton;
    QPushButton * quitButton;

private slots:
};

#endif // LOGINDIALOG_H
