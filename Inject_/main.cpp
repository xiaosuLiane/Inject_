#include "Inject_.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Inject_ w;
    w.show();
    return a.exec();
}
