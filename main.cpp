#include "MainWindow.h"
#include <QApplication>

#include "CNFSolver.h"
#include <iostream>
#include <fstream>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
