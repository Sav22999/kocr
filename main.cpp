#include "kocrmainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    kocrMainWindow w;
    w.show();

    return a.exec();
}
