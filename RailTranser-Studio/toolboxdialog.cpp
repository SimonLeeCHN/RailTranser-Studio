#include "toolboxdialog.h"
#include "ui_toolboxdialog.h"
#include <QMessageBox>

ToolBoxDialog::ToolBoxDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ToolBoxDialog)
{
    ui->setupUi(this);

    this->initWindowStyle();


}

ToolBoxDialog::~ToolBoxDialog()
{
    delete ui;
}

void ToolBoxDialog::initWindowStyle()
{
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void ToolBoxDialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)

    this->setVisible(false);
}

void ToolBoxDialog::on_BTN_ClearDirectCmd_clicked()
{
    ui->PTE_DirectCmd->clear();
}

void ToolBoxDialog::on_BTN_StartDirectCmd_clicked()
{
    QString _plainText = ui->PTE_DirectCmd->toPlainText();
    QList<QString> _plainList = _plainText.split("\n");
    QList<QString> _cmdList;

    for(int _index = 0 ; _index < _plainList.count() ; _index++)
    {
        QString _tempString = _plainList.value(_index);

        //为空，看下一语句
        if(_tempString.isEmpty())
            continue;

        //去除首尾空格
        _tempString.trimmed();

        //将plainlist语句复制到cmdlist
        _cmdList.append(_tempString);
    }

    //在首尾加入STA与END
    _cmdList.prepend("STA");
    _cmdList.append("END");

    emit RequestActionPlayerLoadList(_cmdList);
    emit RequestActionPlayerDoNextStep();
}
