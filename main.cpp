#include "workspherewindow.h"

#include <QApplication>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFont appFont("Segoe UI", 10);
    a.setFont(appFont);

    WorkSphereWindow w;
    w.show();
    return QApplication::exec();
}
