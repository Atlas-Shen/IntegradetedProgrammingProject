#include "CNFSolver.h"
#include <string>
#include <sstream>
#include <chrono>

CNFSolver::CNFSolver(std::istream &input, bool selectedBranchingRule)
    : originalClauseNum(0),
      currentClauseNum(0),
      clausesInfo(nullptr),
      variableNum(0),
      variablesInfo(nullptr),
      getBranchingLiteral(nullptr),
      originalMaxClauseLength(0),
      hasEmptyClause(false) {
    std::string line;
    while (getline(input, line)) {
        if (line.front() == 'p') {
             std::stringstream ss(line);
             std::string p, cnf;
             unsigned variableNum, clauseNum;
             ss >> p >> cnf >> variableNum >> clauseNum;
             this->variableNum = variableNum;
             currentClauseNum = originalClauseNum = clauseNum;
             break;
        }
    }

    //clause index is from 0 to originalClauseNum - 1
    clausesInfo = new ClauseInfo[originalClauseNum];

    //variable index is from 1 to variableNum
    //variablesInfo[0] is not used
    variablesInfo = new VariableInfo[variableNum + 1];

    getBranchingLiteral = selectedBranchingRule ? &CNFSolver::getMOMSBranchingLiteral : &CNFSolver::getDLCSBranchingLiteral;

    int literal;
    for (unsigned i = 0; i < originalClauseNum; ++i) {
        while (input >> literal) {
            if (literal == 0) {
                if (clausesInfo[i].literals.size() == 1)
                    if (!unitClauseLiteralsToAssign.doesContain(clausesInfo[i].literals.front()))
                        unitClauseLiteralsToAssign.addBack(clausesInfo[i].literals.front());
                break;
            }
            if (!clausesInfo[i].literals.doesContain(literal)) {
                clausesInfo[i].literals.addBack(literal);
                if (literal > 0)
                    variablesInfo[literal].positiveOccur.addBack(i);
                else
                    variablesInfo[-literal].negativeOccur.addBack(i);
            }
        }
    }
}

CNFSolver::~CNFSolver() {
    delete[] clausesInfo;
    delete[] variablesInfo;
}

bool CNFSolver::isSatisfied() {
    ProcessResult preprocessResult = preprocess();
    if (preprocessResult == Satisfied)
        return true;
    if (preprocessResult == Unsatisfied)
        return false;

    int currentBranchingLiteral = 0;
    while (true) {
        if (hasEmptyClause) {
            hasEmptyClause = false;
            applyAssignment(currentBranchingLiteral);
            assignmentsInfo.addFront(AssignmentInfo(currentBranchingLiteral, true));
        }
        else if ((currentBranchingLiteral = (this->*getBranchingLiteral)()) != 0) {
            applyAssignment(currentBranchingLiteral);
            assignmentsInfo.addFront(AssignmentInfo(currentBranchingLiteral, false));
        }
        ProcessResult firstCheckResult = checkWithBacktracking(currentBranchingLiteral);
        if (firstCheckResult == BacktrackingDone)
            continue;
        if (firstCheckResult == Satisfied)
            return true;
        if (firstCheckResult == Unsatisfied)
            return false;

        while (!unitClauseLiteralsToAssign.isEmpty()) {
            int unitClauseLiteral = unitClauseLiteralsToAssign.front();
            unitClauseLiteralsToAssign.removeFront();
            applyAssignment(unitClauseLiteral);
            assignmentsInfo.front().assignedUnitClauseLiterals.addFront(unitClauseLiteral);
            ProcessResult secondCheckResult = checkWithBacktracking(currentBranchingLiteral);
            if (secondCheckResult == BacktrackingDone)
                break;
            if (secondCheckResult == Satisfied)
                return true;
            if (secondCheckResult == Unsatisfied)
                return false;
        }
    }
}

void CNFSolver::printSatisfiabilityInfo(std::ostream &output) {
    using namespace std::chrono;

    auto begin = steady_clock::now();
    bool result = isSatisfied();
    auto end = steady_clock::now();
    auto timeSpan = duration_cast<duration<unsigned, std::micro>>(end - begin);
    output << "s " << result << std::endl;
    if (result) {
        output << "v ";
        for (unsigned i = 1; i <= variableNum; ++i) {
            switch (variablesInfo[i].assignedStatus) {
            case VariableInfo::True:
                output << i << ' ';
                break;
            case VariableInfo::False:
                output << '-' << i << ' ';
                break;
            case VariableInfo::None:
                output<< '[' << i << "] ";
                break;
            }
        }
        output << std::endl;
    }
    output << "t " << timeSpan.count() / 1000.0 << std::endl;
}

CNFSolver::ProcessResult CNFSolver::preprocess() {
    while (!unitClauseLiteralsToAssign.isEmpty()) {
        int literal = unitClauseLiteralsToAssign.front();
        unitClauseLiteralsToAssign.removeFront();

        List<unsigned>::Iterator satisfyIter;
        List<unsigned>::Iterator deleteIter;
        if (literal > 0) {
            variablesInfo[literal].assignedStatus = VariableInfo::True;
            satisfyIter = variablesInfo[literal].positiveOccur.iterator();
            deleteIter = variablesInfo[literal].negativeOccur.iterator();
        }
        else {
            variablesInfo[-literal].assignedStatus = VariableInfo::False;
            satisfyIter = variablesInfo[-literal].negativeOccur.iterator();
            deleteIter = variablesInfo[-literal].positiveOccur.iterator();
        }
        while (satisfyIter.isValid()) {
            if (!clausesInfo[satisfyIter.element()].isSatisfied) {
                clausesInfo[satisfyIter.element()].isSatisfied = true;
                --currentClauseNum;
            }
            satisfyIter.next();
        }
        while (deleteIter.isValid()) {
            unsigned clauseIndex = deleteIter.element();
            if (!clausesInfo[clauseIndex].isSatisfied) {
                clausesInfo[clauseIndex].literals.removeFirstOf(-literal);
                //check whether it is a unit clause or empty clause
                if (clausesInfo[clauseIndex].literals.size() == 0) {
                    hasEmptyClause = true;
                    break;
                }
                if (clausesInfo[clauseIndex].literals.size() == 1)
                    if (!unitClauseLiteralsToAssign.doesContain(clausesInfo[clauseIndex].literals.front()))
                        unitClauseLiteralsToAssign.addBack(clausesInfo[clauseIndex].literals.front());
            }
            deleteIter.next();
        }

        if (currentClauseNum == 0)
            return Satisfied;
        if (hasEmptyClause)
            return Unsatisfied;
    }
    for (unsigned i = 0; i < originalClauseNum; ++i) {
        if (!clausesInfo[i].isSatisfied && clausesInfo[i].literals.size() > originalMaxClauseLength)
            originalMaxClauseLength = clausesInfo[i].literals.size();
    }
    return Continued;
}

void CNFSolver::applyAssignment(int literal) {
    List<unsigned>::Iterator satisfyIter;
    List<unsigned>::Iterator deleteIter;
    if (literal > 0) {
        variablesInfo[literal].assignedStatus = VariableInfo::True;
        satisfyIter = variablesInfo[literal].positiveOccur.iterator();
        deleteIter = variablesInfo[literal].negativeOccur.iterator();
    }
    else {
        variablesInfo[-literal].assignedStatus = VariableInfo::False;
        satisfyIter = variablesInfo[-literal].negativeOccur.iterator();
        deleteIter = variablesInfo[-literal].positiveOccur.iterator();
    }
    while (satisfyIter.isValid()) {
        if (!clausesInfo[satisfyIter.element()].isSatisfied) {
            clausesInfo[satisfyIter.element()].isSatisfied = true;
            variablesInfo[std::abs(literal)].satisfiedOccur.addFront(satisfyIter.element());
            --currentClauseNum;
        }
        satisfyIter.next();
    }
    while (deleteIter.isValid()) {
        unsigned clauseIndex = deleteIter.element();
        if (!clausesInfo[clauseIndex].isSatisfied) {
            clausesInfo[clauseIndex].literals.removeFirstOf(-literal);
            variablesInfo[std::abs(literal)].deletedOccur.addFront(clauseIndex);

            //check whether it is a unit clause or empty clause
            if (clausesInfo[clauseIndex].literals.size() == 0) {
                hasEmptyClause = true;
                break;
            }
            if (clausesInfo[clauseIndex].literals.size() == 1) {
                if (!unitClauseLiteralsToAssign.doesContain(clausesInfo[clauseIndex].literals.front()))
                    unitClauseLiteralsToAssign.addBack(clausesInfo[clauseIndex].literals.front());
            }
        }
        deleteIter.next();
    }
}

void CNFSolver::undoAssignment(int literal) {
    auto variableIndex = std::abs(literal);
    variablesInfo[variableIndex].assignedStatus = VariableInfo::None;
    while (!variablesInfo[variableIndex].satisfiedOccur.isEmpty()) {
        clausesInfo[variablesInfo[variableIndex].satisfiedOccur.front()].isSatisfied = false;
        variablesInfo[variableIndex].satisfiedOccur.removeFront();
        ++currentClauseNum;
    }
    while (!variablesInfo[variableIndex].deletedOccur.isEmpty()) {
        clausesInfo[variablesInfo[variableIndex].deletedOccur.front()].literals.addBack(-literal);
        variablesInfo[variableIndex].deletedOccur.removeFront();
    }
}

CNFSolver::ProcessResult CNFSolver::checkWithBacktracking(int &currentBranchingLiteral) {
    if (currentClauseNum == 0)
        return Satisfied;
    if (hasEmptyClause) {
        //backtracking step, undo all the assignment made in this iteration
        unitClauseLiteralsToAssign.clear();
        while (!assignmentsInfo.isEmpty()) {
            while (!assignmentsInfo.front().assignedUnitClauseLiterals.isEmpty()) {
                undoAssignment(assignmentsInfo.front().assignedUnitClauseLiterals.front());
                assignmentsInfo.front().assignedUnitClauseLiterals.removeFront();
            }
            undoAssignment(assignmentsInfo.front().assignedBranchingLiteral);
            if (!assignmentsInfo.front().isForcedAssignment) {
                currentBranchingLiteral = -assignmentsInfo.front().assignedBranchingLiteral;
                assignmentsInfo.removeFront();
                return BacktrackingDone;
            }
            assignmentsInfo.removeFront();
        }
        return Unsatisfied;
    }
    return Continued;
}

int CNFSolver::getDLCSBranchingLiteral() const {
    unsigned maxCombinedSum = 0;
    int literal = 0;
    for (unsigned i = 1; i <= variableNum; ++i) {
        if (variablesInfo[i].assignedStatus == VariableInfo::None) {
            unsigned positiveSum = 0;
            auto positiveIter = variablesInfo[i].positiveOccur.iterator();
            while (positiveIter.isValid()) {
                positiveSum += 1 - clausesInfo[positiveIter.element()].isSatisfied;
                positiveIter.next();
            }
            unsigned negativeSum = 0;
            auto negativeIter = variablesInfo[i].negativeOccur.iterator();
            while (negativeIter.isValid()) {
                negativeSum += 1 - clausesInfo[negativeIter.element()].isSatisfied;
                negativeIter.next();
            }
            unsigned combinedSum = positiveSum + negativeSum;
            if (combinedSum > maxCombinedSum) {
                maxCombinedSum = combinedSum;
                if (positiveSum >= negativeSum)
                    literal = static_cast<int>(i);
                else
                    literal = -static_cast<int>(i);
            }
        }
    }
    return literal;
}

int CNFSolver::getMOMSBranchingLiteral() const {
    unsigned minUnsatisfiedClauseLength = originalMaxClauseLength;
    for (unsigned i = 0; i < originalClauseNum && minUnsatisfiedClauseLength != 2; ++i) {
        if (!clausesInfo[i].isSatisfied && clausesInfo[i].literals.size() < minUnsatisfiedClauseLength)
            minUnsatisfiedClauseLength = clausesInfo[i].literals.size();
    }
    unsigned maxResult = 0;
    int literal = 0;
    for (unsigned i = 1; i <= variableNum; ++i) {
        if (variablesInfo[i].assignedStatus == VariableInfo::None) {
            unsigned positiveSum = 0;
            auto positiveIter = variablesInfo[i].positiveOccur.iterator();
            while (positiveIter.isValid()) {
                if (!clausesInfo[positiveIter.element()].isSatisfied
                        && clausesInfo[positiveIter.element()].literals.size() == minUnsatisfiedClauseLength)
                    ++positiveSum;
                positiveIter.next();
            }
            unsigned negativeSum = 0;
            auto negativeIter = variablesInfo[i].negativeOccur.iterator();
            while (negativeIter.isValid()) {
                if (!clausesInfo[negativeIter.element()].isSatisfied
                        && clausesInfo[negativeIter.element()].literals.size() == minUnsatisfiedClauseLength)
                    ++negativeSum;
                negativeIter.next();
            }
            unsigned result = (positiveSum + 1) * (negativeSum + 1);
            if (result > maxResult) {
                maxResult = result;
                if (positiveSum >= negativeSum)
                    literal = static_cast<int>(i);
                else
                    literal = -static_cast<int>(i);
            }
        }
    }
    return literal;
}
