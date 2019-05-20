#include "CNFSolverThread.h"
#include "CNFSolver.h"
#include <fstream>
#include <sstream>
#include <QTextCodec>

CNFSolverThread::CNFSolverThread(const std::string &fileName, bool selectedBranchingRule, QObject *parent)
    : QThread(parent),
      fileName(fileName),
      selectedBranchingRule(selectedBranchingRule) {}

void CNFSolverThread::run() {
    std::ifstream input(fileName);
    std::stringstream output;
    QTextCodec *code = QTextCodec::codecForLocale();
    QString fileNameString = code->toUnicode(fileName.c_str());
    output << fileNameString.toStdString() << " solved!" << std::endl;
    if (selectedBranchingRule)
        output << "Used MOMS(Maximum Occurrences on clauses of Minimum Size) branching rule." << std::endl;
    else
        output << "Used DLCS(Dynamic Largest Combined Sum) branching rule." << std::endl;
    CNFSolver(input, selectedBranchingRule).printSatisfiabilityInfo(output);
    emit sendResult(QString::fromStdString(output.str()));
}
