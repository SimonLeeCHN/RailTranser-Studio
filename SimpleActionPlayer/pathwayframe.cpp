#include <QPainter>
#include <QFont>
#include "pathwayframe.h"

PathwayFrame::PathwayFrame(QWidget *parent) : QFrame(parent)
{
    m_bPathwayLoaded = false;
}

void PathwayFrame::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if(m_bPathwayLoaded == false)
    {
        //画板背景
        QPainter painter(this);
        painter.setPen(QColor(60,60,60));

        QFont font;
        font.setFamily("Microsoft YaHei");
        font.setPointSize(90);
        painter.setFont(font);

        painter.drawText(rect(),Qt::AlignCenter,"ACS");
    }
}
