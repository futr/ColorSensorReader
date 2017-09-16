#include "graph.h"

Graph::Graph(QWidget *parent) : QWidget(parent),
    bar( new QScrollBar( this ) ),
    layout( new QVBoxLayout( this ) ),
    label( new QLabel( this ) ),
    wave( new WaveGraphWidget( this ) )
{
    // Setup
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::MinimumExpanding );

    bar->setOrientation( Qt::Horizontal );
    label->setText( "text" );
    label->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Maximum );

    wave->setMinimumSize( 0, 110 );
    wave->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::MinimumExpanding );

    // Layout
    setLayout( layout );
    layout->addWidget( label );
    layout->addWidget( wave );
    layout->addWidget( bar );
    layout->setSpacing( 1 );
    layout->setContentsMargins( 1, 1, 1, 1 );

    // Connect
    connect( wave, SIGNAL(rangeChanged(int,int)), bar, SLOT(setRange(int,int)), Qt::DirectConnection );
    connect( bar, SIGNAL(valueChanged(int)), wave, SLOT(setHeadIndex(int)), Qt::DirectConnection );
    connect( wave, SIGNAL(headChanged(int)), bar, SLOT(setValue(int)), Qt::DirectConnection );
}

void Graph::setLabel(const QString &text)
{
    label->setText( text );
}

void Graph::setPrintMode(bool enable)
{
    // 印刷用の表示になる
    bar->setVisible( !enable );
}

