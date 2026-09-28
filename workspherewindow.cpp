#include "workspherewindow.h"
#include "./ui_workspherewindow.h"

WorkSphereWindow::WorkSphereWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::WorkSphereWindow)
{
    ui->setupUi(this);
}

WorkSphereWindow::~WorkSphereWindow()
{
    delete ui;
}
