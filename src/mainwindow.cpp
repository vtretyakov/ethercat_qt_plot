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
    connect(o_ecat_thread, SIGNAL(numSlavesChanged(int)), this, SLOT(update_slaves_number(int)));
    connect(o_ecat_thread, SIGNAL(workRequested()), thread, SLOT(start()));
    connect(thread, SIGNAL(started()), o_ecat_thread, SLOT(doWork()));
    connect(o_ecat_thread, SIGNAL(finished()), thread, SLOT(quit()), Qt::DirectConnection);

    connect(ui->sb_torque_ref, SIGNAL(editingFinished()), this, SLOT(update_torque_ref()));
    connect(ui->cb_torque_mode, SIGNAL(stateChanged(int)), this, SLOT(select_op_mode_cst(int)));
    connect(ui->pb_freeze_plot, SIGNAL(clicked()), this, SLOT(freeze_plot()));

    connect(ui->cmbbx_slave_id, SIGNAL(currentIndexChanged(int)), this, SLOT(select_slave(int)));

    //variables initialization
    _toggle_freeze = false;
    _ref_value = 0;
    _x_range = 8;

    // Plot

    ui->widget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                    QCP::iSelectLegend | QCP::iSelectPlottables);
    ui->widget->addGraph(); // blue line
    ui->widget->graph(0)->setPen(QPen(Qt::blue));
    ui->widget->graph(0)->setAntialiasedFill(false);
    ui->widget->addGraph(); // red line
    ui->widget->graph(1)->setPen(QPen(Qt::red));
    ui->widget->graph(1)->setAntialiasedFill(false);
    ui->widget->addGraph(); // red line
    ui->widget->graph(2)->setPen(QPen(Qt::green));
    ui->widget->graph(2)->setAntialiasedFill(false);
    ui->widget->addGraph(); // red line
    ui->widget->graph(3)->setPen(QPen(Qt::DotLine));
    ui->widget->graph(3)->setAntialiasedFill(false);

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%%z");
    //timeTicker->setTimeFormat("%h:%m:%s");
    ui->widget->axisRect()->setupFullAxesBox();
    ui->widget->yAxis->setRange(-1000.0, 1000.0);


    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->widget->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->widget->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->widget->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->widget->yAxis2, SLOT(setRange(QCPRange)));


    // connect slot that ties some axis selections together (especially opposite axes):
    connect(ui->widget, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));

    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(ui->widget, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
    connect(ui->widget, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel(QWheelEvent*)));


    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    connect(&_dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    _dataTimer.start(0); // Interval 0 means to refresh as fast as possible

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
    _ref_value = ui->sb_torque_ref->value();
    o_ecat_thread->set_torque_reference((int16_t)_ref_value);
}

void MainWindow::select_op_mode_cst(int state)
{

    int op_mode = 0;
    if (state == 2){
        op_mode = 10;
    }
    o_ecat_thread->set_op_mode(op_mode);
}

void MainWindow::freeze_plot()
{
    _toggle_freeze ^=1;
    if (_toggle_freeze){
        ui->pb_freeze_plot->setText("Continue");
    }
    else{
        ui->pb_freeze_plot->setText("Pause Plot");
    }
}

void MainWindow::select_slave(const int &index)
{
    o_ecat_thread->select_slave(index);
}

void MainWindow::update_slaves_number(const int &value)
{
    if (value > 1){
        ui->label->setText("Found " + QString::number(value) + " slaves");
    }
    else {
        ui->label->setText("Found " + QString::number(value) + " slave");
    }
    for (int i = 0; i < value; i++){
        ui->cmbbx_slave_id->addItem("Node " + QString::number(i));
    }
}


void MainWindow::mouseWheel(QWheelEvent * wheel_event)
{
  // if an axis is selected, only allow the direction of that axis to be zoomed
  // if no axis is selected, both directions may be zoomed

  if (ui->widget->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
  {
    ui->widget->axisRect()->setRangeZoom(ui->widget->xAxis->orientation());
    _x_range += wheel_event->delta()/480.0;
    if (_x_range < 1){
        _x_range = 1;
    }
    qDebug() << "x_range" << _x_range;
  }
  else if (ui->widget->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
  {
    ui->widget->axisRect()->setRangeZoom(ui->widget->yAxis->orientation());
  }
  else
  {
    ui->widget->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
  }
}


void MainWindow::mousePress()
{
  // if an axis is selected, only allow the direction of that axis to be dragged
  // if no axis is selected, both directions may be dragged

  if (ui->widget->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->widget->axisRect()->setRangeDrag(ui->widget->xAxis->orientation());
  else if (ui->widget->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->widget->axisRect()->setRangeDrag(ui->widget->yAxis->orientation());
  else
    ui->widget->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}


void MainWindow::selectionChanged()
{
  /*
   normally, axis base line, axis tick labels and axis labels are selectable separately, but we want
   the user only to be able to select the axis as a whole, so we tie the selected states of the tick labels
   and the axis base line together. However, the axis label shall be selectable individually.

   The selection state of the left and right axes shall be synchronized as well as the state of the
   bottom and top axes.

   Further, we want to synchronize the selection of the graphs with the selection state of the respective
   legend item belonging to that graph. So the user can select a graph by either clicking on the graph itself
   or on its legend item.
  */

  // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
  if (ui->widget->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->widget->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
      ui->widget->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->widget->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
  {
    ui->widget->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    ui->widget->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }
  // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
  if (ui->widget->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->widget->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
      ui->widget->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->widget->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
  {
    ui->widget->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    ui->widget->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }

  // synchronize selection of graphs with selection of corresponding legend items:
  for (int i=0; i<ui->widget->graphCount(); ++i)
  {
    QCPGraph *graph = ui->widget->graph(i);
    QCPPlottableLegendItem *item = ui->widget->legend->itemWithPlottable(graph);
    if (item->selected() || graph->selected())
    {
      item->setSelected(true);
      graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
    }
  }
}

void MainWindow::realtimeDataSlot()
{
  //static QTime time(QTime::currentTime());
  // calculate two new data points:
  //FixMe: find a better timer. QElapsedTimer is suggested.
  //double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
  double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
  static double lastPointKey = 0;
  if (key-lastPointKey > 0.001) // at most add point every 1 ms
  {
    // add data to lines:
    ui->widget->graph(0)->addData(key, o_ecat_thread->get_position1_actual());
    ui->widget->graph(1)->addData(key, o_ecat_thread->get_torque_actual());
    ui->widget->graph(2)->addData(key, o_ecat_thread->get_velocity1_actual());
    ui->widget->graph(3)->addData(key, _ref_value);


    // rescale value (vertical) axis to fit the current data:
    //ui->customPlot->graph(0)->rescaleValueAxis();
    //ui->customPlot->graph(1)->rescaleValueAxis(true);
    lastPointKey = key;
  }

  if(!_toggle_freeze){
      // make key axis range scroll with the data (at a constant range size of 8):
      ui->widget->xAxis->setRange(key, _x_range, Qt::AlignRight);
      ui->widget->replot();
  }

  // calculate frames per second:
  static double lastFpsKey;
  static int frameCount;
  ++frameCount;
  if (key-lastFpsKey > 2) // average fps over 2 seconds
  {
    ui->statusBar->showMessage(
          QString("%1 FPS, Total Data points: %2")
          .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
          .arg(ui->widget->graph(0)->data()->size()+ui->widget->graph(1)->data()->size())
          , 0);
    lastFpsKey = key;
    frameCount = 0;
  }
}
