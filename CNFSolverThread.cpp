#include "CNFSolverThread.h"
#include "CNFSolver.h"
#include <fstream>
#include <sstream>

CNFSolverThread::CNFSolverThread(const QString &fileName, bool selectedBranchingRule)
    : fileName(fileName),
      selectedBranchingRule(selectedBranchingRule) {}

void CNFSolverThread::run() {
    std::ifstream input(fileName.toStdString());
    std::stringstream output;
    output << fileName.toStdString() << std::endl;
    if (selectedBranchingRule)
        output << "MOMS(Maximum Occurrences on clauses of Minimum Size) branching rule." << std::endl;
    else
        output << "DLCS(Dynamic Largest Combined Sum) branching rule." << std::endl;
    CNFSolver(input, selectedBranchingRule).printSatisfiabilityInfo(output);
    emit sendResult(QString::fromStdString(output.str()));
}
