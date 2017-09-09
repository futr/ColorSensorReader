#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    // Construct and move to worker thread a sensor class
    colorSensor = new ColorSensorAccess;
    colorSensor->moveToThread( &sensorThread );
}

Widget::~Widget()
{
    // Stop a sensor thread
    sensorThread.quit();
    sensorThread.wait();

    delete ui;
}
