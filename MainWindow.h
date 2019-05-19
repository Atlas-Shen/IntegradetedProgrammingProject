#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void runCNFSolver();
    void appendResult(QString);
    void generateSudoku();
    void checkSudoku();
    void solveSudoku();
    void receiveSudokuAndSolution(QString, QString);

private:
    Ui::MainWindow *ui;
    QString sudokuString;
    QString solutionString;
};

#endif // MAINWINDOW_H
