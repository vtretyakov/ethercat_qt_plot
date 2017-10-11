#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // The thread and the o_ecat_thread are created in the constructor so it is always safe to delete them.
    thread = new QThread();
    o_ecat_thread = new CEthercatThread();

    o_ecat_thread->moveToThread(thread);
    connect(o_ecat_thread, SIGNAL(numSlavesChanged(QString)), ui->label, SLOT(setText(QString)));
    connect(o_ecat_thread, SIGNAL(workRequested()), thread, SLOT(start()));
    connect(thread, SIGNAL(started()), o_ecat_thread, SLOT(doWork()));
    connect(o_ecat_thread, SIGNAL(finished()), thread, SLOT(quit()), Qt::DirectConnection);

    connect(ui->sb_torque_ref, SIGNAL(editingFinished()), this, SLOT(update_torque_ref()));
    connect(ui->cb_torque_mode, SIGNAL(stateChanged(int)), this, SLOT(select_op_mode_cst(int)));
}

MainWindow::~MainWindow()
{
    o_ecat_thread->abort();
    thread->wait();
    qDebug()<<"Deleting thread and o_ecat_thread in Thread "<<this->QObject::thread()->currentThreadId();
    delete thread;
    delete o_ecat_thread;

    delete ui;
}

void MainWindow::on_startButton_clicked()
{
    // To avoid having two threads running simultaneously, the previous thread is aborted.
    o_ecat_thread->abort();
    thread->wait(); // If the thread is not running, this will immediately return.

    o_ecat_thread->requestWork();
}


void MainWindow::on_stopButton_clicked()
{
    // To avoid having two threads running simultaneously, the previous thread is aborted.
    o_ecat_thread->abort();
    thread->wait(); // If the thread is not running, this will immediately return.

}

void MainWindow::update_torque_ref()
{
    int16_t ref[] = {0};
    ref[0] = (int16_t)ui->sb_torque_ref->value();
    o_ecat_thread->set_torque_reference(ref);
}

void MainWindow::select_op_mode_cst(int state)
{

    int op_mode[] = {0};
    if (state == 2){
        op_mode[0] = 10;
    }
    o_ecat_thread->set_op_mode(op_mode);
}
