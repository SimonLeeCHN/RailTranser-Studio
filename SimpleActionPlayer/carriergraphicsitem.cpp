#include "carriergraphicsitem.h"
#include <QPainter>
#include <QDebug>

#define CARRIERGI_WIDTH             20
#define CARRIERGI_HEIGHT            20
#define CARRIERGI_X                 10
#define CARRIERGI_Y                 10
#define CARRIERGI_PEN_SIZE          10

#define CARRIERGI_COLOR_ERROR       QColor(225,67,75)
#define CARRIERGI_COLOR_STANDBY     QColor(67,122,225)
#define CARRIERGI_COLOR_RUNNING     QColor(67,225,123)
#define CARRIERGI_COLOR_MISSING     QColor(0,0,0)

#define CARRIERGI_NUMBER_COLOR      QColor(255,255,255)
#define CARRIERGI_NUMBER_SIZE       10

#define CARRIERGI_STATUS_ERROR      1
#define CARRIERGI_STATUS_STANDBY    2
#define CARRIERGI_STATUS_RUNNING    3
#define CARRIERGI_STATUS_MISSING    4

CarrierGraphicsItem::CarrierGraphicsItem()
{
    m_qpGraphicsPoint = QPoint(0,0);
    m_iStatus = 2;
    m_iNumber = 1;

    this->setPos(m_qpGraphicsPoint);
}

CarrierGraphicsItem::CarrierGraphicsItem(QPoint point, int status, int number)
{
    m_qpGraphicsPoint = point;
    m_iStatus = status;
    m_iNumber = number;

    this->setPos(m_qpGraphicsPoint);
}

void CarrierGraphicsItem::setGraphicsPoint(QPoint point)
{
    m_qpGraphicsPoint = point;
}

void CarrierGraphicsItem::setNumber(int number)
{
    m_iNumber = number;
}

void CarrierGraphicsItem::setStatus(int status)
{
    m_iStatus = status;
}

CarrierGraphicsItem::~CarrierGraphicsItem()
{

}

QRectF CarrierGraphicsItem::boundingRect() const
{
    qreal adjust = 0.5;
    return QRectF(CARRIERGI_X - adjust,CARRIERGI_Y - adjust,
                  CARRIERGI_WIDTH + adjust,CARRIERGI_HEIGHT + adjust);
}

QPainterPath CarrierGraphicsItem::shape() const
{
    QPainterPath tempPath;
    tempPath.addRect(CARRIERGI_X,CARRIERGI_Y,CARRIERGI_WIDTH,CARRIERGI_HEIGHT);
    return tempPath;
}

void CarrierGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    //draw Graphics Carrier

//    if(m_qpGraphicsPoint == QPoint(0,0))
//    {
//        //    (m_qpGraphicsPoint.x() == 0) && (m_qpGraphicsPoint.y() == 0)
//        return;
//    }

    //外形
    switch (m_iStatus)
    {
        case CARRIERGI_STATUS_ERROR:
            painter->setBrush(CARRIERGI_COLOR_ERROR);
            painter->setPen(QPen(QBrush(CARRIERGI_COLOR_ERROR),1));
            break;
        case CARRIERGI_STATUS_RUNNING:
            painter->setBrush(CARRIERGI_COLOR_RUNNING);
            painter->setPen(QPen(QBrush(CARRIERGI_COLOR_RUNNING),1));
            break;
        case CARRIERGI_STATUS_STANDBY:
            painter->setBrush(CARRIERGI_COLOR_STANDBY);
            painter->setPen(QPen(QBrush(CARRIERGI_COLOR_STANDBY),1));
            break;
        case CARRIERGI_STATUS_MISSING:
            painter->setBrush(CARRIERGI_COLOR_MISSING);
            painter->setPen(QPen(QBrush(CARRIERGI_COLOR_MISSING),1));
            break;
        default:
            break;
    }
    painter->drawEllipse(CARRIERGI_X,CARRIERGI_Y,CARRIERGI_WIDTH,CARRIERGI_HEIGHT);

    //编号
    painter->setPen(CARRIERGI_NUMBER_COLOR);
    QFont tempFont;
    tempFont.setFamily("Bauhaus 93");
    tempFont.setPointSize(CARRIERGI_NUMBER_SIZE);
    painter->setFont(tempFont);
    painter->drawText(QRect(CARRIERGI_X,CARRIERGI_Y,CARRIERGI_WIDTH,CARRIERGI_HEIGHT),
                      Qt::AlignCenter,
                      QString("%1").arg(QString::number(this->m_iNumber)));

}

void CarrierGraphicsItem::advance(int step)
{
    if(!step)
        return;

    this->setPos(m_qpGraphicsPoint);
    qDebug()<<"GraphicCarrier advance";
}

