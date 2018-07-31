#include "pathwaygv.h"
#include "rfidgraphicsitem.h"
#include "carriergraphicsitem.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QApplication>
#include <QMessageBox>
#include <QWheelEvent>

#define RFID_BASEDEV    100

PathwayGV::PathwayGV(QWidget *parent) :
                QGraphicsView(parent)
{
    //初始化scene
    m_pScene = new QGraphicsScene();
    m_pScene->setBackgroundBrush(QBrush(SCENE_BACKGROUND_COLOR));
    this->setScene(m_pScene);
    this->setDragMode(QGraphicsView::ScrollHandDrag);

    //初始化背景文字
    m_pBackgroundTextItem = new QGraphicsSimpleTextItem("ACS");
    QFont font;
    font.setFamily("Microsoft YaHei");
    font.setPointSize(90);
    m_pBackgroundTextItem->setFont(font);
    m_pBackgroundTextItem->setBrush(QColor(68, 69, 73));
    m_pScene->addItem(m_pBackgroundTextItem);

}

PathwayGV::~PathwayGV()
{
    delete m_pScene;
}

void PathwayGV::wheelEvent(QWheelEvent *event)
{
    //实现滚轮缩放大小

    this->centerOn(event->pos());

    if(event->delta() > 0)
        this->scale(pow(1.1, 2), pow(1.1, 2));
    else
        this->scale(pow(1/1.1, 2), pow(1/1.1, 2));

    QGraphicsView::wheelEvent(event);

}

QPoint PathwayGV::transPosnumToGraphicPoint(int pos)
{
    //由RFID号找到其RFID的GraphicPoint
    if(m_listRfid.count() <= 0)
    {
        qDebug()<<"没有载入RFID，转换失败";
        return QPoint(0,0);
    }

    for(int i = 0;i < m_listRfid.count();i++)
    {
        RfidGraphicsItem* it = (RfidGraphicsItem*)(m_listRfid.at(i));
        if(it->getNumber() == pos)
        {
            return it->getGraphicsPoint();
        }
    }
    qDebug()<<"找不到需求转换点";
    return QPoint(0,0);
}

void PathwayGV::initPathwayRfid(QList<QString> rfidList)
{
    //移除背景文字
    m_pScene->removeItem(m_pBackgroundTextItem);

    //先将轨道item压入scene，否则会在rfid上层绘制
    m_pPathwayPath = new QGraphicsPathItem;
    m_pScene->addItem(m_pPathwayPath);

    //绘制RFID点位
    QPainterPath pathwayPath;
    for(int i = 0 ; i < rfidList.count() ; i++)
    {
        QStringList lineConfigList = QString(rfidList.at(i)).split(" ");
        if(lineConfigList.count() == 3)
        {
            int x = QString(lineConfigList.at(0)).toInt();
            int y = QString(lineConfigList.at(1)).toInt();
            int number = QString(lineConfigList.at(2)).toInt();
            m_listRfid << new RfidGraphicsItem(QPoint(x,y),number);
            m_pScene->addItem(m_listRfid.last());

            if(i == 0)
                pathwayPath.moveTo(x,y);
            else
                pathwayPath.lineTo(x,y);
        }
        else
        {
            QMessageBox::critical(this,tr("Pathway RFID Error"),tr("RFID 数据单错误"));
            return;
        }
    }

    //绘制轨道
    m_pPathwayPath->setPath(pathwayPath);
    m_pPathwayPath->setPen(QPen(QBrush(Qt::white),10));
}

void PathwayGV::initGraphicCarrier(QList<QString> carrierList)
{
    //初始化Graphics Carrier

    for(int i = 0;i < carrierList.count() ; i++)
    {
        QStringList lineConfigList = QString(carrierList.at(i)).split(" ");
        if(lineConfigList.count() == 4)
        {
            int number = i+1;
            int pos = QString(lineConfigList.at(1)).toInt();
            int status = QString(lineConfigList.at(3)).toInt();

            if(m_listRfid.count() <= 0)
            {
                QMessageBox::critical(this,tr("Pathway Carrier Error"),tr("尚未载入RFID点位"));
                return;
            }

            //点位检查
            if(((pos/RFID_BASEDEV) < 0) || ((pos/RFID_BASEDEV) > m_listRfid.count()))
            {
                QMessageBox::critical(this,tr("Pathway Carrier Error"),tr("Carrier图形位置错误"));
                return;
            }

            QPoint tempPoint = transPosnumToGraphicPoint(pos);
            m_listGraphicCarrier << new CarrierGraphicsItem(tempPoint,status,number);
            m_pScene->addItem(m_listGraphicCarrier.last());
        }
        else
        {
            QMessageBox::critical(this,tr("Pathway Carrier Error"),tr("Carrier图形单错误"));
            return;
        }
    }
}

/*      SLOT        */
void PathwayGV::onUpdateGraphicCarrier(int number, int status, int pos)
{
    if(pos <= 0)
        return;

    //由number确定是哪辆GraphicCarrier，直接设置它的status
    CarrierGraphicsItem* gcItem = m_listGraphicCarrier.at(number-1);
    gcItem->setStatus(status);

    //由pos找到对应RFID的GraphicPoint，赋值给GraphicCarrier
    gcItem->setGraphicsPoint(transPosnumToGraphicPoint(pos));

}
