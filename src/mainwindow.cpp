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
    connect(o_ecat_thread, SIGNAL(valueChanged(QString)), ui->label, SLOT(setText(QString)));
    connect(o_ecat_thread, SIGNAL(workRequested()), thread, SLOT(start()));
    connect(thread, SIGNAL(started()), o_ecat_thread, SLOT(doWork()));
    connect(o_ecat_thread, SIGNAL(finished()), thread, SLOT(quit()), Qt::DirectConnection);
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
