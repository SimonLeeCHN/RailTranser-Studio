#include "pathwaygv.h"
#include "rfidgraphicsitem.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QApplication>
#include <QMessageBox>
#include <QWheelEvent>

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

void PathwayGV::initPathwayRfid(QList<QString> rfidList)
{
    //移除背景文字
    m_pScene->removeItem(m_pBackgroundTextItem);

    for(int i = 0 ; i < rfidList.count() ; i++)
    {
        QStringList lineConfigList = QString(rfidList.at(i)).split(" ");
        if(lineConfigList.count() == 3)
        {
            int x = QString(lineConfigList.at(0)).toInt();
            int y = QString(lineConfigList.at(1)).toInt();
            int number = QString(lineConfigList.at(2)).toInt();
            m_pScene->addItem(new RfidGraphicsItem(QPoint(x,y),number));
        }
        else
        {
            QMessageBox::critical(this,tr("Pathway RFID Error"),tr("RFID 数据单错误"));
            return;
        }
    }

    //test
//    QList<QGraphicsItem*> tempList = m_pScene->items();
//    qDebug()<<(RfidGraphicsItem*)(tempList.at(2))->getNumber();

}


