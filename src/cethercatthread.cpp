#include "cethercatthread.h"
#include <QTimer>
#include <QEventLoop>

#include <QThread>
#include <QDebug>

#include <stdio.h>

#include <ethercat_wrapper.h>
#include <ethercat_wrapper_slave.h>
#include "ecat_master.h"
#include "profile.h"
#include "operation.h"

CEthercatThread::CEthercatThread(QObject *parent) :
    QObject(parent)
{
    _working =false;
    _abort = false;
}

void CEthercatThread::requestWork()
{
    mutex.lock();
    _working = true;
    _abort = false;
    qDebug()<<"Request worker start in Thread "<<thread()->currentThreadId();
    mutex.unlock();

    emit workRequested();
}

void CEthercatThread::abort()
{
    mutex.lock();
    if (_working) {
        _abort = true;
        qDebug()<<"Request worker aborting in Thread "<<thread()->currentThreadId();
    }
    mutex.unlock();
}

void CEthercatThread::doWork()
{
    qDebug()<<"Starting worker process in Thread "<<thread()->currentThreadId();

    int debug = 0;
    int profile_speed = 50; //rpm
    int profile_acc = 50; //rpm/s
    int profile_torque_acc = 50; // 1/1000 rated torque / s

    /********* ethercat init **************/
    /* use master id 0 for the first ehtercat master interface (defined by the
     * libethercat).
     * The logging output must be redirected into a file, otherwise the output will
     * interfere with the ncurses windowing. */
    FILE *ecatlog = fopen("./ecat.log", "w");
    Ethercat_Master_t *master = ecw_master_init(0 /* master id */, ecatlog);
    if (master == NULL) {
        qDebug() << "Cannot initialize master " << stderr;
        _abort = true;
    }

    int num_slaves = ecw_master_slave_count(master);
    qDebug()<< num_slaves << " slaves found";
    emit valueChanged("slaves: " + QString::number(num_slaves));

    /* Init pdos */
    PDOInput  *pdo_input  = (PDOInput *)malloc(num_slaves*sizeof(PDOInput));
    PDOOutput *pdo_output = (PDOOutput *)malloc(num_slaves*sizeof(PDOOutput));

    //init output structure
    OutputValues output = {0};
    output.app_mode = CYCLIC_SYNCHRONOUS_MODE;
    output.sign = 1;
    output.debug = debug;
    output.target_state = (CIA402State *)malloc(num_slaves*sizeof(CIA402State));

    //init profiler
    PositionProfileConfig *profile_config = (PositionProfileConfig *)malloc(num_slaves*sizeof(PositionProfileConfig));

    //init for all slaves
    for (int i = 0; i < num_slaves; i++) {

        /* Init pdos */
        pdo_output[i].controlword = 0;
        pdo_output[i].op_mode = OPMODE_CST;
        pdo_output[i].target_position = 0;
        pdo_output[i].target_torque = 0;
        pdo_output[i].target_velocity = 0;
        pdo_input[i].op_mode_display = 0;
        pdo_input[i].statusword = 0;

        //init output structure
        output.target_state[i] = CIASTATE_SWITCH_ON_DISABLED;

        //init profiler
        profile_config[i].profile_speed = profile_speed;
        profile_config[i].profile_acceleration = profile_acc;
        profile_config[i].profile_torque_acceleration = profile_torque_acc;

        profile_config[i].max_torque = 1000; //max torque is 1000
        profile_config[i].max_torque_acceleration = profile_torque_acc;
        profile_config[i].max_acceleration = profile_acc;
        profile_config[i].max_speed = 10000;
        profile_config[i].max_position = 0x7fffffff;
        profile_config[i].min_position = -0x7fffffff;
        profile_config[i].ticks_per_turn = 65536; //default value
        //try to find the correct ticks_per_turn in the sdo config
        for (int sensor_port=1; sensor_port<=3; sensor_port++) {
            //get sensor config
            int sensor_config = read_sdo(master, i, DICT_FEEDBACK_SENSOR_PORTS, sensor_port);
            if (sensor_config != 0) {
                int sensor_function = read_sdo(master, i, sensor_config, SUB_ENCODER_FUNCTION);
                //check sensor function
                if (sensor_function == 1 || sensor_function == 3) { //sensor functions 1 and 3 are motion control
                    profile_config[i].ticks_per_turn = read_sdo(master, i, sensor_config, SUB_ENCODER_RESOLUTION);
                    break;
                }
            }
        }
        init_position_profile_limits(&(profile_config[i].motion_profile),
                profile_config[i].max_torque, profile_config[i].max_torque_acceleration,
                profile_config[i].max_acceleration, profile_config[i].max_speed,
                profile_config[i].max_position, profile_config[i].min_position, profile_config[i].ticks_per_turn);
    }

    /********* ethercat start master **************/
    if (ecw_master_start(master) != 0) {
        qDebug() << "Error starting cyclic operation of master - giving up" << stderr;
        _abort = true;
    }
    /****************************************************/

    while(1){
        // Checks if the process should be aborted
        mutex.lock();
        bool abort = _abort;
        mutex.unlock();

        //ethercat communication
        ecw_master_cyclic_function(master);
        pdo_handler(master, pdo_input, pdo_output, -1);

  //      cyclic_synchronous_mode(pdo_output, pdo_input, num_slaves, &output, profile_config);

        if (abort) {
            qDebug()<<"Aborting ethercat process in Thread "<<thread()->currentThreadId();
          //  quit_mode(pdo_output, pdo_input, num_slaves);

            break;
        }

        // This will stupidly wait 1 msec doing nothing...
        QEventLoop loop;
        QTimer::singleShot(1, &loop, SLOT(quit()));
        loop.exec();
    }


    // Set _working to false, meaning the process can't be aborted anymore.
    mutex.lock();
    _working = false;
    mutex.unlock();

    qDebug()<<"Worker process finished in Thread "<<thread()->currentThreadId();
    //free resources
    ecw_master_stop(master);
    ecw_master_release(master);
    fclose(ecatlog);
    free(pdo_input);
    free(pdo_output);
    free(profile_config);
    free(output.target_state);

    //Once 60 sec passed, the finished signal is sent
    emit finished();
}
