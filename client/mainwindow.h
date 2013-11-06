#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QtGui>
class MainWindow: public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
private:
    QTextEdit * chatEdit;
    QTextEdit * messageEdit;
    QPushButton * sendButton;

private slots:
    void sentMessage();
    void update();
};

#endif // MAINWINDOW_H
