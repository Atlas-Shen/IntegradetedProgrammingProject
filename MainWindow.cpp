#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CNFSolverThread.h"
#include <QString>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->dlcsRadioButton->setChecked(true);
    connect(ui->runButton, &QPushButton::clicked, this, &MainWindow::runCNFSolver);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::runCNFSolver() {
    //ui->textBrowser->clear();
    QString fileName = QFileDialog::getOpenFileName(this, "Choose a CNF file to run the solver", "../InputFile", "*.cnf");
    if (fileName == nullptr)
        ui->textBrowser->append("No selected file!\n");
    else {
        CNFSolverThread *solverThread = new CNFSolverThread(fileName, ui->momsRadioButton->isChecked());
        connect(solverThread, &CNFSolverThread::finished, solverThread, &CNFSolverThread::deleteLater);
        connect(solverThread, &CNFSolverThread::sendResult, this, &MainWindow::appendResult, Qt::AutoConnection);
        solverThread->start();
    }
}

void MainWindow::appendResult(QString result) {
    ui->textBrowser->append(result);
}
