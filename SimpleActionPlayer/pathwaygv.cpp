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
#include <QTimer>
#include <math.h>

#define RFID_BASEDEV    1

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

    //更新时钟
    m_pUpdateTimer = new QTimer();
    connect(m_pUpdateTimer,&QTimer::timeout,m_pScene,&QGraphicsScene::advance);
    m_pUpdateTimer->start(500);

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
        RfidGraphicsItem* it = (RfidGraphicsItem*)(m_listRfid.value(i));
        if(it->getNumber() == pos)
        {
            return it->getGraphicsPoint();
        }
    }

    //输入的点位没有对应的RFID位，返回QPoint(0,0)
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
        QStringList lineConfigList = QString(rfidList.value(i)).split(" ");
        if(lineConfigList.count() == 3)
        {
            int x = QString(lineConfigList.value(0)).toInt();
            int y = QString(lineConfigList.value(1)).toInt();
            int number = QString(lineConfigList.value(2)).toInt();
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
        QStringList lineConfigList = QString(carrierList.value(i)).split(" ");
        if(lineConfigList.count() == 5)
        {
            int _iNumber = i+1;
            int _iPos = QString(lineConfigList.value(1)).toInt();
            int _iStatus = QString(lineConfigList.value(3)).toInt();
            int _iEnabled = QString(lineConfigList.value(4)).toInt();

            if(_iEnabled != 1)
                continue;

            if(m_listRfid.count() <= 0)
            {
                QMessageBox::critical(this,tr("Pathway Carrier Error"),tr("尚未载入RFID点位"));
                return;
            }

            //点位检查
            if(((_iPos/RFID_BASEDEV) < 0) || ((_iPos/RFID_BASEDEV) > m_listRfid.count()))
            {
                QMessageBox::critical(this,tr("Pathway Carrier Error"),tr("Carrier图形位置错误"));
                return;
            }

            QPoint tempPoint = transPosnumToGraphicPoint(_iPos);
            m_listGraphicCarrier << new CarrierGraphicsItem(tempPoint,_iStatus,_iNumber);
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
    //由number确定是哪辆GraphicCarrier，直接设置它的status
    if(((number > 0) && number <= m_listGraphicCarrier.count()))
    {
        CarrierGraphicsItem* gcItem = m_listGraphicCarrier.value(number-1);
        gcItem->setStatus(status);

        //由pos找到对应RFID的GraphicPoint，赋值给GraphicCarrier
        gcItem->setGraphicsPoint(transPosnumToGraphicPoint(pos));

        //test 强制触发scene进行advance
        m_pScene->advance();
    }
}
