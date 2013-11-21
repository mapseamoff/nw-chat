#include <QCoreApplication>
#include <iostream>

#include "RunChat.h"

int main(int argc, char *argv[])
{
    if(argc != 5) {
        std::cout << "Usage: " << argv[0] << " <host> <port> <username> <time>" << std::endl;
        return 0;
    }

    QCoreApplication a(argc, argv);
    RunChat chat(&a, argv[1], argv[2], argv[3], QString(argv[4]).toUInt());
    return a.exec();
}
