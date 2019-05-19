#ifndef CNFSOLVERTHREAD_H
#define CNFSOLVERTHREAD_H

#include <QThread>
#include <QString>

class CNFSolverThread : public QThread {
    Q_OBJECT

public:
    CNFSolverThread(const QString &, bool);

signals:
    void sendResult(QString);

protected:
    void run() override;

private:
    QString fileName;
    bool selectedBranchingRule;
};

#endif // CNFSOLVERTHREAD_H
