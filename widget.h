#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QThread>
#include <QButtonGroup>
#include <QPainter>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>

#include "colorsensoraccess.h"
#include "graph.h"

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

    QButtonGroup intTimeGroup, gainGroup;

    QThread sensorThread;
    ColorSensorAccess *colorSensor;

public slots:
    void setData( ColorSensorAccess::ColorData data );
    void setDataToGraph( ColorSensorAccess::ColorData data );
    void statusMessage( QString str );
    void clearGraph();
    void setGraphXScale( int scale );
    void setGraphXSGrid( int grid );

signals:
    void doReading( bool continuously );
    void stopReading( void );

private slots:
    void enableSensorButtons( bool enable = true );

    void on_openButton_clicked();

    void on_initializeButton_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

    void on_readSensorButton_clicked();

    void on_readSensorContButton_clicked();

    void on_stopReadingButton_clicked();

    void on_closeSensorButton_clicked();

    void on_clearGraphButton_clicked();

    void on_saveGraphButton_clicked();

private:
    void setColorLabel( ColorSensorAccess::ColorData data );
};

#endif // WIDGET_H
