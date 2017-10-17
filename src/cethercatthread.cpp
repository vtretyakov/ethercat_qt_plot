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

void CEthercatThread::set_torque_reference(int16_t torque_ref[])
{
    mutex.lock();
    _torque_ref = torque_ref[0];
    mutex.unlock();
    qDebug() << "value = " << _torque_ref;
}


void CEthercatThread::set_op_mode(int op_mode)
{
    mutex.lock();
    _op_mode = op_mode;
    // qDebug() << "op_mode: " << _op_mode[0] << "  " << op_mode[0];
    if ( op_mode != 0){
        _req_cia402_state = CIASTATE_OP_ENABLED;
    }
    else{
        _req_cia402_state = CIASTATE_SWITCH_ON_DISABLED;
    }
    mutex.unlock();
    qDebug() << "_op_mode " << &_op_mode;
    qDebug() << "_req_cia402_state " << &_req_cia402_state;
    qDebug() << "op_mode: " << _op_mode << "  " << op_mode;
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
    emit numSlavesChanged("slaves: " + QString::number(num_slaves));
    _op_mode = 0;
    _req_cia402_state = CIASTATE_NOT_READY;


    /* Init pdos */
    PDOInput  *pdo_input  = (PDOInput *)malloc(num_slaves*sizeof(PDOInput));
    PDOOutput *pdo_output = (PDOOutput *)malloc(num_slaves*sizeof(PDOOutput));

    //init output structure
    OutputValues output = {0};
    output.app_mode = CYCLIC_SYNCHRONOUS_MODE;
    output.sign = 1;
    output.debug = debug;
    output.target_state = (CIA402State *)malloc(num_slaves*sizeof(CIA402State));

    _torque_ref = 0;

    //init profiler
    PositionProfileConfig *profile_config = (PositionProfileConfig *)malloc(num_slaves*sizeof(PositionProfileConfig));

    //init for all slaves
    for (int i = 0; i < num_slaves; i++) {

        /* Init pdos */
        pdo_output[i].controlword = 0;
        pdo_output[i].op_mode = 0;//OPMODE_CST
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

        //FixMe: implement the cyclic mode
        //init slaves, set all slaves to opmode CST CIASTATE_SWITCH_ON_DISABLED
        if (output.init == 0) {
            output.init = 1;
            for (int i=0; i<num_slaves; i++) {
                CIA402State current_state = cia402_read_state(pdo_input[i].statusword);
                // if the fault is only CIA402_ERROR_CODE_COMMUNICATION we reset it
                // if the slave is not in CIASTATE_SWITCH_ON_DISABLED or CIASTATE_FAULT we put it in CIASTATE_SWITCH_ON_DISABLED
                if ( (current_state == CIASTATE_FAULT && pdo_input[i].user_miso == CIA402_ERROR_CODE_COMMUNICATION) ||
                     (current_state != CIASTATE_FAULT && current_state != CIASTATE_SWITCH_ON_DISABLED) )
                {
                    pdo_output[i].controlword = cia402_go_to_state(CIASTATE_SWITCH_ON_DISABLED, current_state, pdo_output[i].controlword, 0);
                    output.init = 0;
                }
            }
        }

        //Display data
        //ToDo

        //manage user commands
        //ToDo
        pdo_output[0].op_mode = _op_mode;
        output.target_state[0] = _req_cia402_state;
        //reset profile
        //profile_config[0].step = 1;
       // profile_config[0].steps = 0;
        pdo_output[0].target_torque = _torque_ref;

        //manage slaves state machines and opmode
  //      if (output.manual != 1) {
            state_machine_control(pdo_output, pdo_input, num_slaves, &output);
  //      }

        //use profile to generate a target for position/velocity
        //target_generate(profile_config, pdo_output, pdo_input, num_slaves);

        //cyclic_synchronous_mode(pdo_output, pdo_input, num_slaves, &output, profile_config);

        if (abort) {
            qDebug()<<"Aborting ethercat process in Thread "<<thread()->currentThreadId();
            quit_mode(pdo_output, pdo_input, num_slaves);

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
