#include "rfidgraphicsitem.h"
#include <QDebug>

RfidGraphicsItem::RfidGraphicsItem()
{
    m_qpGraphicsPoint = QPoint(0,0);
    m_iNumber = 0;
}


RfidGraphicsItem::RfidGraphicsItem(QPoint point, int number)
{
    m_qpGraphicsPoint = point;
    m_iNumber = number;
}

void RfidGraphicsItem::setGraphicsPoint(QPoint point)
{
    m_qpGraphicsPoint = point;
}

void RfidGraphicsItem::setNumber(int number)
{
    m_iNumber = number;
}

RfidGraphicsItem::~RfidGraphicsItem()
{

}

QRectF RfidGraphicsItem::boundingRect() const
{
    qreal adjust = 0.5;
    return QRectF(RFIDGI_X - adjust,RFIDGI_Y - adjust,
                  RFIDGI_WIDTH + adjust,RFIDGI_HEIGHT + adjust);
}

QPainterPath RfidGraphicsItem::shape() const
{
    QPainterPath tempPath;
    tempPath.addRect(RFIDGI_X,RFIDGI_Y,RFIDGI_WIDTH,RFIDGI_HEIGHT);
    return tempPath;

}

void RfidGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    //draw Graphics RFID

    //外形
    painter->setBrush(RFIDGI_BACKGROUND_COLOR);
    painter->setPen(QPen(QBrush(RFIDGI_BACKGROUND_COLOR),1));
    painter->drawRoundedRect(RFIDGI_X,RFIDGI_Y,RFIDGI_WIDTH,RFIDGI_HEIGHT,8.0,6.0);

    //编号
    painter->setPen(RFIDGI_PEN_COLOR);
    QFont tempFont;
    tempFont.setFamily("Consolas");
    tempFont.setPointSize(RFIDGI_PEN_SIZE);

    painter->setFont(tempFont);
    painter->drawText(QRect(RFIDGI_X,RFIDGI_Y,RFIDGI_WIDTH,RFIDGI_HEIGHT),
                      Qt::AlignCenter,
                      QString("%1").arg(QString::number(this->m_iNumber)));

    this->setPos(m_qpGraphicsPoint);
}

