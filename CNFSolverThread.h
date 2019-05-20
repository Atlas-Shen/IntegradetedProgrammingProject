#ifndef CNFSOLVERTHREAD_H
#define CNFSOLVERTHREAD_H

#include <QThread>
#include <string>

class CNFSolverThread : public QThread {
    Q_OBJECT

public:
    CNFSolverThread(const std::string &, bool, QObject *parent = nullptr);

signals:
    void sendResult(QString);

protected:
    void run() override;

private:
    std::string fileName;
    bool selectedBranchingRule;
};

#endif // CNFSOLVERTHREAD_H
