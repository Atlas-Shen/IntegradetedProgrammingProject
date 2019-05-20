#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CNFSolverThread.h"
#include "SudokuGeneratorThread.h"
#include <QFileDialog>
#include <QTextCodec>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->dlcsRadioButton->setChecked(true);
    connect(ui->runButton, &QPushButton::clicked, this, &MainWindow::runCNFSolver);

    connect(ui->generateButton, &QPushButton::clicked, this, &MainWindow::generateSudoku);
    connect(ui->checkButton, &QPushButton::clicked, this, &MainWindow::checkSudoku);
    connect(ui->solveButton, &QPushButton::clicked, this, &MainWindow::solveSudoku);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            ui->tableWidget->setItem(i, j, new QTableWidgetItem(""));
            ui->tableWidget->item(i, j)->setFlags(Qt::NoItemFlags);
            ui->tableWidget->item(i, j)->setTextAlignment(Qt::AlignCenter);
        }
    }
    ui->checkButton->setEnabled(false);
    ui->solveButton->setEnabled(false);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::runCNFSolver() {
    QString fileName = QFileDialog::getOpenFileName(this, "Choose a CNF file to run the solver", ".", "*.cnf");
    if (fileName == nullptr)
        ui->textBrowser->append("No selected file!\n");
    else {
        QTextCodec *code = QTextCodec::codecForLocale();
        std::string stdFileName = code->fromUnicode(fileName).data();
        CNFSolverThread *solverThread = new CNFSolverThread(stdFileName, ui->momsRadioButton->isChecked());
        connect(solverThread, &CNFSolverThread::finished, solverThread, &CNFSolverThread::deleteLater);
        connect(solverThread, &CNFSolverThread::sendResult, this, &MainWindow::appendResult, Qt::AutoConnection);
        solverThread->start();
    }
}

void MainWindow::appendResult(QString result) {
    ui->textBrowser->append(result);
}

void MainWindow::generateSudoku() {
    bool ok;
    unsigned givenCellNum = ui->lineEdit->text().toUInt(&ok);
    if (!ok || givenCellNum < 17 || givenCellNum > 81)
        QMessageBox::warning(this, "Warning", "Invalid number of given cells!");
    else {
        ui->lineEdit->clear();
        for (int i = 0; i < 9; ++i) {
            for (int j = 0; j < 9; ++j) {
                ui->tableWidget->item(i, j)->setText("");
                ui->tableWidget->item(i, j)->setFlags(Qt::NoItemFlags);
            }
        }
        ui->label->setText("Generating...");
        ui->generateButton->setEnabled(false);
        ui->checkButton->setEnabled(false);
        ui->solveButton->setEnabled(false);
        SudokuGeneratorThread *sudokuThread = new SudokuGeneratorThread(givenCellNum);
        connect(sudokuThread, &SudokuGeneratorThread::finished, sudokuThread, &SudokuGeneratorThread::deleteLater);
        connect(sudokuThread, &SudokuGeneratorThread::sendSudokuAndSolution, this, &MainWindow::receiveSudokuAndSolution, Qt::AutoConnection);
        sudokuThread->start();
    }
}

void MainWindow::checkSudoku() {
    ui->label->setText("Checking...");
    ui->generateButton->setEnabled(false);
    ui->checkButton->setEnabled(false);
    ui->solveButton->setEnabled(false);
    QString resultString = "";
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j)
            resultString.append(ui->tableWidget->item(i, j)->text());
    }
    ui->label->setText("");
    ui->generateButton->setEnabled(true);
    ui->checkButton->setEnabled(true);
    ui->solveButton->setEnabled(true);
    if (resultString != solutionString)
        QMessageBox::warning(this, "Warning", "Wrong Answer!");
    else
        QMessageBox::information(this, "Congratulation", "Correct Answer!");
}

void MainWindow::solveSudoku() {
    ui->label->setText("Solving...");
    ui->generateButton->setEnabled(false);
    ui->checkButton->setEnabled(false);
    ui->solveButton->setEnabled(false);
    int iter = 0;
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            if (QString(sudokuString[iter]) == "0") {
                ui->tableWidget->item(i, j)->setFlags(Qt::NoItemFlags);
                ui->tableWidget->item(i, j)->setText(QString(solutionString[iter]));
            }
            ++iter;
        }
    }
    ui->label->setText("");
    ui->generateButton->setEnabled(true);
}

void MainWindow::receiveSudokuAndSolution(QString sudokuString, QString solutionString) {
    this->sudokuString = sudokuString;
    this->solutionString = solutionString;
    ui->label->setText("");
    ui->generateButton->setEnabled(true);
    ui->checkButton->setEnabled(true);
    ui->solveButton->setEnabled(true);
    int iter = 0;
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            QString number = QString(sudokuString[iter]);
            if (number != "0") {
                ui->tableWidget->item(i, j)->setText(number);
                ui->tableWidget->item(i, j)->setTextColor(Qt::gray);
            }
            else {
                ui->tableWidget->item(i, j)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
                ui->tableWidget->item(i, j)->setTextColor(Qt::black);
            }
            ++iter;
        }
    }
}
