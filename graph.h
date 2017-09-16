#ifndef GRAPH_H
#define GRAPH_H

#include <QWidget>
#include <QScrollBar>
#include <QLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QString>
#include "wavegraphwidget.h"

class Graph : public QWidget
{
    Q_OBJECT
public:
    explicit Graph(QWidget *parent = 0);

    void setLabel( const QString &text );

signals:

public slots:
    void setPrintMode( bool enable );

public:
    QScrollBar *bar;
    QVBoxLayout *layout;
    QLabel *label;

public:
    WaveGraphWidget *wave;
};

#endif // GRAPH_H
