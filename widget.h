#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QThread>

#include "colorsensoraccess.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private:
    Ui::Widget *ui;

    QThread sensorThread;
    ColorSensorAccess *colorSensor;
};

#endif // WIDGET_H
