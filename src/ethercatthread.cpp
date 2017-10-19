#include "ethercatthread.h"
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


EthercatThread::EthercatThread(QObject *parent) :
    QObject(parent)
{
    working_ =false;
    abort_ = false;
    position_actual1_ = 0;
    position_actual2_ = 0;
    velocity_actual1_ = 0;
    velocity_actual2_ = 0;
    torque_actual_ = 0;
    selected_slave_id_ = 0;
}

void EthercatThread::requestWork()
{
    mutex_.lock();
    working_ = true;
    abort_ = false;
    qDebug()<<"Request worker start in Thread "<<thread()->currentThreadId();
    mutex_.unlock();

    emit workRequested();
}

void EthercatThread::abort()
{
    mutex_.lock();
    if (working_) {
        abort_ = true;
        qDebug()<<"Request worker aborting in Thread "<<thread()->currentThreadId();
    }
    mutex_.unlock();
}

void EthercatThread::set_torque_reference(int16_t torque_ref)
{
    mutex_.lock();
    torque_ref_ = torque_ref;
    mutex_.unlock();
    qDebug() << "value = " << torque_ref_;
}


void EthercatThread::set_op_mode(int op_mode)
{
    mutex_.lock();
    op_mode_ = op_mode;
    if ( op_mode != 0){
        req_cia402_state_ = CIASTATE_OP_ENABLED;
    }
    else{
        req_cia402_state_ = CIASTATE_SWITCH_ON_DISABLED;
    }
    mutex_.unlock();
    qDebug() << "req_cia402_state " << req_cia402_state_;
    qDebug() << "op_mode: " << op_mode_;
}

int EthercatThread::get_position1_actual()
{
    return position_actual1_;
}

int EthercatThread::get_position2_actual()
{
    return position_actual2_;
}

int EthercatThread::get_velocity1_actual()
{
    return velocity_actual1_;
}

int EthercatThread::get_velocity2_actual()
{
    return velocity_actual2_;
}

int EthercatThread::get_torque_actual()
{
    return torque_actual_;
}

bool EthercatThread::is_running()
{
    return working_;
}

void EthercatThread::select_slave(int slave_id)
{
    mutex_.lock();
    selected_slave_id_ = slave_id;
    mutex_.unlock();
    qDebug() << "select slave " << selected_slave_id_;
}

void EthercatThread::doWork()
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
        abort_ = true;
    }

    int num_slaves = ecw_master_slave_count(master);
    qDebug()<< num_slaves << " slaves found";
    emit numSlavesChanged(num_slaves);
    op_mode_ = 0;
    req_cia402_state_ = CIASTATE_NOT_READY;


    /* Init pdos */
    PDOInput  *pdo_input  = (PDOInput *)malloc(num_slaves*sizeof(PDOInput));
    PDOOutput *pdo_output = (PDOOutput *)malloc(num_slaves*sizeof(PDOOutput));

    //init output structure
    OutputValues output = {0};
    output.app_mode = CYCLIC_SYNCHRONOUS_MODE;
    output.sign = 1;
    output.debug = debug;
    output.target_state = (CIA402State *)malloc(num_slaves*sizeof(CIA402State));

    torque_ref_ = 0;

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
        abort_ = true;
    }
    /****************************************************/

    while(1){
        // Checks if the process should be aborted
        mutex_.lock();
        bool abort = abort_;
        mutex_.unlock();

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
        //FixMe: make a nice method
        mutex_.lock();
        position_actual1_ = pdo_input[selected_slave_id_].position_value;
        position_actual2_ = pdo_input[selected_slave_id_].secondary_position_value;
        velocity_actual1_ = pdo_input[selected_slave_id_].velocity_value;
        velocity_actual2_ = pdo_input[selected_slave_id_].secondary_velocity_value;
        torque_actual_ = pdo_input[selected_slave_id_].torque_value;
        mutex_.unlock();

        //manage user commands
        //ToDo
        pdo_output[selected_slave_id_].op_mode = op_mode_;
        output.target_state[selected_slave_id_] = req_cia402_state_;
        //reset profile
        //profile_config[_selected_slave_id].step = 1;
       // profile_config[_selected_slave_id].steps = 0;
        pdo_output[selected_slave_id_].target_torque = torque_ref_;

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
    mutex_.lock();
    working_ = false;
    mutex_.unlock();

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
