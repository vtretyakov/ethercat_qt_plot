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

    /********* ethercat init **************/

    FILE *ecatlog = fopen("./ecat.log", "w");
    Ethercat_Master_t *master = ecw_master_init(0 /* master id */, ecatlog);

    if (master == NULL) {
        qDebug() << "Cannot initialize master " << stderr;
    }

    int num_slaves = ecw_master_slave_count(master);
    qDebug()<< num_slaves << " slaves found";
    emit valueChanged("slaves: " + QString::number(num_slaves));

    // Activate master and start operation
    if (ecw_master_start(master) != 0) {
        qDebug() << "Error starting cyclic operation of master - giving up" << stderr;
    }

    /* Init pdos */
    PDOInput  *pdo_input  = (PDOInput *)malloc(num_slaves*sizeof(PDOInput));
    PDOOutput *pdo_output = (PDOOutput *)malloc(num_slaves*sizeof(PDOOutput));

    //init profiler
    PositionProfileConfig *profile_config = (PositionProfileConfig *)malloc(num_slaves*sizeof(PositionProfileConfig));

    while(1){
        // Checks if the process should be aborted
        mutex.lock();
        bool abort = _abort;
        mutex.unlock();

        if (abort) {
            qDebug()<<"Aborting ethercat process in Thread "<<thread()->currentThreadId();
            break;
        }

        // This will stupidly wait 1 sec doing nothing...
        QEventLoop loop;
        QTimer::singleShot(1000, &loop, SLOT(quit()));
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

    //Once 60 sec passed, the finished signal is sent
    emit finished();
}
