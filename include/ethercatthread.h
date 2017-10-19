#ifndef ETHERCATTHREAD_H
#define ETHERCATTHREAD_H

#include <QObject>
#include <QMutex>
#include "cia402.h"

class EthercatThread : public QObject
{
    Q_OBJECT

public:
    explicit EthercatThread(QObject *parent = 0);
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
    void set_torque_reference(int16_t torque_ref);
    void set_op_mode(int op_mode);
    int get_position1_actual();
    int get_position2_actual();
    int get_velocity1_actual();
    int get_velocity2_actual();
    int get_torque_actual();
    bool is_running();
    void select_slave(int slave_id);


private:
    /**
     * @brief Process is aborted when @em true
     */
    bool abort_;
    /**
     * @brief @em true when Worker is doing work
     */
    bool working_;
    /**
     * @brief Protects access to #_abort
     */
    QMutex mutex_;

    int16_t torque_ref_;
    int op_mode_;
    CIA402State req_cia402_state_;

    int position_actual1_;
    int position_actual2_;
    int velocity_actual1_;
    int velocity_actual2_;
    int torque_actual_;
    int selected_slave_id_;

signals:
    /**
     * @brief This signal is emitted when the Worker request to Work
     * @sa requestWork()
     */
    void workRequested();
    /**
     * @brief This signal is emitted when number of detected slaves is changed
     */
    void numSlavesChanged(const int &value);
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
#endif // ETHERCATTHREAD_H
