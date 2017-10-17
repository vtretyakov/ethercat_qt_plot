#ifndef CETHERCATTHREAD_H
#define CETHERCATTHREAD_H

#include <QObject>
#include <QMutex>
#include "cia402.h"

class CEthercatThread : public QObject
{
    Q_OBJECT

public:
    explicit CEthercatThread(QObject *parent = 0);
    /**
     * @brief Requests the process to start
     *
     * It is thread safe as it uses #mutex to protect access to #_working variable.
     */
    void requestWork();
    /**
     * @brief Requests the process to abort
     *
     * It is thread safe as it uses #mutex to protect access to #_abort variable.
     */
    void abort();   
    void set_torque_reference(int16_t torque_ref[]);
    void set_op_mode(int op_mode);

private:
    /**
     * @brief Process is aborted when @em true
     */
    bool _abort;
    /**
     * @brief @em true when Worker is doing work
     */
    bool _working;
    /**
     * @brief Protects access to #_abort
     */
    QMutex mutex;

    int16_t _torque_ref;
    int _op_mode;
    CIA402State _req_cia402_state;

signals:
    /**
     * @brief This signal is emitted when the Worker request to Work
     * @sa requestWork()
     */
    void workRequested();
    /**
     * @brief This signal is emitted when number of detected slaves is changed
     */
    void numSlavesChanged(const QString &value);
    /**
     * @brief This signal is emitted when process is finished (either by counting 60 sec or being aborted)
     */
    void finished();

public slots:
    /**
     * @brief Main ethercat cyclic task
     */
    void doWork();
};
#endif // CETHERCATTHREAD_H
