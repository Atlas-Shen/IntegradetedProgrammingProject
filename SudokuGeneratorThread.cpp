#include "SudokuGeneratorThread.h"
#include "CNFSolver.h"
#include <random>
#include <chrono>
#include <functional>
#include <cstring>

SudokuGeneratorThread::SudokuGeneratorThread(unsigned givenCellNum, QObject *parent)
    : QThread(parent),
      givenCellNum(givenCellNum),
      sudoku{{0}},
      solution{{0}},
      rowFlag{{false}},
      colFlag{{false}},
      blockFlag{{false}} {}

void SudokuGeneratorThread::run() {
    //Use Las Vegas Algorithm to generate a complete sudoku solution
    while (!lasVegas(11))
        ;//try again
    memcpy(sudoku, solution, sizeof(solution));

    //dig holes to generate a sudoku
    //digging from top to bottom and from left to right
    unsigned blankCellNum = 81 - givenCellNum;
    for (unsigned i = 1; i <= 9; ++i) {
        for (unsigned j = 1; j <= 9; ++j) {
            if (blankCellNum > 0 && isUnique(i, j)) {
                rowFlag[i][sudoku[i][j]] = false;
                colFlag[j][sudoku[i][j]] = false;
                blockFlag[getBlockIndex(i, j)][sudoku[i][j]] = false;
                sudoku[i][j] = 0;
                --blankCellNum;
            }
        }
    }
    QString sudokuString, solutionString;
    for (unsigned i = 1; i <= 9; ++i) {
        for (unsigned j = 1; j <= 9; ++j) {
            sudokuString.append(QString::number(sudoku[i][j]));
            solutionString.append(QString::number(solution[i][j]));
        }
    }
    emit sendSudokuAndSolution(sudokuString, solutionString);
}

inline unsigned SudokuGeneratorThread::getBlockIndex(unsigned i, unsigned j) {
    return (i - 1) / 3 * 3 + (j - 1) / 3 + 1;
}

bool SudokuGeneratorThread::lasVegas(unsigned n) {
    std::default_random_engine randomGenerator(static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<unsigned> distribution(1, 9);
    auto dice = std::bind(distribution, randomGenerator);

    //init
    unsigned i, j;
    for (i = 1; i <= 9; ++i) {
        for (j = 1; j <= 9; ++j) {
            solution[i][j] = 0;
            rowFlag[i][j] = false;
            colFlag[i][j] = false;
            blockFlag[i][j] = false;
        }
    }

    //fill cells with random numbers
    while (n > 0) {
        i = dice();
        j = dice();
        if (solution[i][j] == 0) {
            unsigned k = getBlockIndex(i, j);
            unsigned value = dice();
            if (!rowFlag[i][value] && !colFlag[j][value] && !blockFlag[k][value]) {
                solution[i][j] = value;
                rowFlag[i][value] = true;
                colFlag[j][value] = true;
                blockFlag[k][value] = true;
                --n;
            }
        }
    }

    //check and generate a complete sudoku solution, return false if it fails
    return CNFSolver::solveSudoku(solution);
}

bool SudokuGeneratorThread::isUnique(unsigned i, unsigned j) {
    //the first cell must be unique
    if (i == 0 && j == 0)
        return true;

    //temp flag array
    unsigned tempSudoku[10][10];
    bool tempRowFlag[10][10];
    bool tempColFlag[10][10];
    bool tempBlockFlag[10][10];
    memcpy(tempSudoku, sudoku, sizeof(sudoku));
    memcpy(tempRowFlag, rowFlag, sizeof(rowFlag));
    memcpy(tempColFlag, colFlag, sizeof(colFlag));
    memcpy(tempBlockFlag, blockFlag, sizeof(blockFlag));

    //suppose we dig this cell
    unsigned k = getBlockIndex(i, j);
    tempRowFlag[i][sudoku[i][j]] = false;
    tempColFlag[j][sudoku[i][j]] = false;
    tempBlockFlag[k][sudoku[i][j]] = false;

    for (unsigned num = 1; num <= 9; ++num)
        if (sudoku[i][j] != num) {
            tempSudoku[i][j] = num;
            if (!tempRowFlag[i][num] && !tempColFlag[j][num] && !tempBlockFlag[k][num]) {
                tempRowFlag[i][num] = true;
                tempColFlag[j][num] = true;
                tempBlockFlag[k][num] = true;
                //change for another number and check if there exist another solution
                if (CNFSolver(tempSudoku).isSatisfied())
                    return false;
                tempRowFlag[i][num] = false;
                tempColFlag[j][num] = false;
                tempBlockFlag[k][num] = false;
            }
        }
    //after trying all the other numbers, it turns out to be unique
    return true;
}
