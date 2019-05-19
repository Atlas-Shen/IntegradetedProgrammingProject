#ifndef SUDOKUGENERATORTHREAD_H
#define SUDOKUGENERATORTHREAD_H

#include <QThread>
#include <QString>

class SudokuGeneratorThread : public QThread {
    Q_OBJECT

public:
    SudokuGeneratorThread(unsigned, QObject *parent = nullptr);

signals:
    void sendSudokuAndSolution(QString, QString);

protected:
    void run() override;

private:
    const unsigned givenCellNum;
    unsigned sudoku[10][10];
    unsigned solution[10][10];
    bool rowFlag[10][10];
    bool colFlag[10][10];
    bool blockFlag[10][10];

    static unsigned getBlockIndex(unsigned, unsigned);
    bool lasVegas(unsigned);
    bool isUnique(unsigned, unsigned);
};

#endif // SUDOKUGENERATORTHREAD_H
