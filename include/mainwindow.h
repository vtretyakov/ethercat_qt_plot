#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QTimer>
#include "cethercatthread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    /**
     * @brief Thread object which will let us manipulate the running thread
     */
    QThread *thread;
    /**
     * @brief Object which contains methods that should be runned in another thread
     */
    CEthercatThread *o_ecat_thread;
    QTimer _dataTimer;
    bool _toggle_freeze;
    int _ref_value;
    float _x_range;

private slots:
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void update_torque_ref();
    void select_op_mode_cst(int state);
    void realtimeDataSlot();
    void freeze_plot();
    void select_slave(const int &index);
    void update_slaves_number(const int &value);
    void mouseWheel(QWheelEvent * wheel_event);
    void mousePress();
    void selectionChanged();
};

#endif // MAINWINDOW_H
