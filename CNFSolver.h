#ifndef CNFSOLVER_H
#define CNFSOLVER_H

#include "List.h"
#include <iostream>

class CNFSolver {

public:

    explicit CNFSolver(std::istream &, bool);
    explicit CNFSolver(unsigned [][10]);
    ~CNFSolver();
    bool isSatisfied(); //DPLL based algorithm
    void printSatisfiabilityInfo(std::ostream &);
    static bool solveSudoku(unsigned [][10]);

    //disable all the unused functions
    CNFSolver(const CNFSolver &) = delete;
    CNFSolver(CNFSolver &&) = delete;
    CNFSolver &operator=(const CNFSolver &) = delete;
    CNFSolver &operator=(CNFSolver &&) = delete;

private:

    struct ClauseInfo {
        bool isSatisfied;
        List<int> literals;

        ClauseInfo() : isSatisfied(false) {}
    };

    struct VariableInfo {

        enum AssignedStatus {
            False,
            True,
            None
        };

        AssignedStatus assignedStatus;
        List<unsigned> positiveOccur;
        List<unsigned> negativeOccur;

        //stacks that store information about changes after an assignment
        List<unsigned> satisfiedOccur;
        List<unsigned> deletedOccur;

        VariableInfo() : assignedStatus(None) {}
    };

    enum ProcessResult {
        Unsatisfied,
        Satisfied,
        Continued,
        BacktrackingDone
    };

    //two types of assignment:
    //1. unit clause literal assignment
    //2. branching literal assignment
    //assignment information in a single iteration
    struct AssignmentInfo {
        int assignedBranchingLiteral;
        bool isForcedAssignment;
        //stack that stores all assigned unit clause literals after a branching
        List<int> assignedUnitClauseLiterals;

        AssignmentInfo() : assignedBranchingLiteral(0), isForcedAssignment(false) {}
        AssignmentInfo(int branchingLiteral, bool isForced)
            : assignedBranchingLiteral(branchingLiteral), isForcedAssignment(isForced) {}
    };

    unsigned originalClauseNum;
    unsigned currentClauseNum;
    ClauseInfo *clausesInfo; //array size decided by originalClauseNum

    unsigned variableNum;
    VariableInfo *variablesInfo; //array size decided by variableNum

    //function pointer to apply branching rule selected by user
    int (CNFSolver::*getBranchingLiteral)() const;

    unsigned originalMaxClauseLength;
    bool hasEmptyClause;

    //queue that stores all current unit clause literals
    List<int> unitClauseLiteralsToAssign;

    //stack that stores all current assignment information
    List<AssignmentInfo> assignmentsInfo;

    ProcessResult preprocess();
    void applyAssignment(int);
    void undoAssignment(int);
    ProcessResult checkWithBacktracking(int &);
    int getDLCSBranchingLiteral() const;
    int getMOMSBranchingLiteral() const;
};

#endif // CNFSOLVER_H
