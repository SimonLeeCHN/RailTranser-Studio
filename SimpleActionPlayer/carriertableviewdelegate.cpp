#include "carriertableviewdelegate.h"
#include <QComboBox>

/*  SpeedBoxDelegate    */
SpeedBoxDelegate::SpeedBoxDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    //Just leave it block
}

QWidget *SpeedBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &/* index */) const
{
    //TODO:实现combobox生成填充
    QComboBox* pEditor = new QComboBox(parent);
    QStringList tempList = {"低速","中低速","中速","中高速","高速"};
    pEditor->addItems(tempList);

    return pEditor;
}

void  SpeedBoxDelegate::setEditorData(QWidget *editor,  const  QModelIndex &index)  const
{
    //TODO:实现双击后默认combobox默认选项
    QString text = index.model()->data(index, Qt::EditRole).toString();
    QComboBox *comboBox =  static_cast <QComboBox*>(editor);
    int  tindex = comboBox->findText(text);
    comboBox->setCurrentIndex(tindex);
}

void  SpeedBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
    const  QModelIndex &index)  const
{
    //TODO:实现选中后填入到model中的数据
    QComboBox *comboBox =  static_cast <QComboBox*>(editor);
    QString text = comboBox->currentText();
    model->setData(index, text, Qt::EditRole);
}

void  SpeedBoxDelegate::updateEditorGeometry(QWidget *editor,
    const  QStyleOptionViewItem &option,  const  QModelIndex &index) const
{
    Q_UNUSED(index)

    editor->setGeometry(option.rect);
}


/*  DirectionBoxDelegate    */
DirectionBoxDelegate::DirectionBoxDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    //Just leave it block
}

QWidget *DirectionBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &/* index */) const
{
    //TODO:实现combobox生成填充
    QComboBox* pEditor = new QComboBox(parent);
    QStringList tempList = {"顺时针","逆时针"};
    pEditor->addItems(tempList);

    return pEditor;
}

void  DirectionBoxDelegate::setEditorData(QWidget *editor,  const  QModelIndex &index)  const
{
    //TODO:实现双击后默认combobox默认选项
    QString text = index.model()->data(index, Qt::EditRole).toString();
    QComboBox *comboBox =  static_cast <QComboBox*>(editor);
    int  tindex = comboBox->findText(text);
    comboBox->setCurrentIndex(tindex);
}

void  DirectionBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
    const  QModelIndex &index)  const
{
    //TODO:实现选中后填入到model中的数据
    QComboBox *comboBox =  static_cast <QComboBox*>(editor);
    QString text = comboBox->currentText();
    model->setData(index, text, Qt::EditRole);
}

void  DirectionBoxDelegate::updateEditorGeometry(QWidget *editor,
    const  QStyleOptionViewItem &option,  const  QModelIndex &index) const
{
    Q_UNUSED(index)

    editor->setGeometry(option.rect);
}


/*  ModeBoxDelegate    */
ModeBoxDelegate::ModeBoxDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    //Just leave it block
}

QWidget *ModeBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &/* index */) const
{
    //TODO:实现combobox生成填充
    QComboBox* pEditor = new QComboBox(parent);
    QStringList tempList = {"S曲线","T曲线","定常曲线"};
    pEditor->addItems(tempList);

    return pEditor;
}

void  ModeBoxDelegate::setEditorData(QWidget *editor,  const  QModelIndex &index)  const
{
    //TODO:实现双击后默认combobox默认选项
    QString text = index.model()->data(index, Qt::EditRole).toString();
    QComboBox *comboBox =  static_cast <QComboBox*>(editor);
    int  tindex = comboBox->findText(text);
    comboBox->setCurrentIndex(tindex);
}

void  ModeBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
    const  QModelIndex &index)  const
{
    //TODO:实现选中后填入到model中的数据
    QComboBox *comboBox =  static_cast <QComboBox*>(editor);
    QString text = comboBox->currentText();
    model->setData(index, text, Qt::EditRole);
}

void  ModeBoxDelegate::updateEditorGeometry(QWidget *editor,
    const  QStyleOptionViewItem &option,  const  QModelIndex &index) const
{
    Q_UNUSED(index)

    editor->setGeometry(option.rect);
}


/*  EnableBoxDelegate    */
EnableBoxDelegate::EnableBoxDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    //Just leave it block
}

QWidget *EnableBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &/* index */) const
{
    //TODO:实现combobox生成填充
    QComboBox* pEditor = new QComboBox(parent);
    QStringList tempList = {"启用","停用"};
    pEditor->addItems(tempList);

    return pEditor;
}

void  EnableBoxDelegate::setEditorData(QWidget *editor,  const  QModelIndex &index)  const
{
    //TODO:实现双击后默认combobox默认选项
    QString text = index.model()->data(index, Qt::EditRole).toString();
    QComboBox *comboBox =  static_cast <QComboBox*>(editor);
    int  tindex = comboBox->findText(text);
    comboBox->setCurrentIndex(tindex);
}

void  EnableBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
    const  QModelIndex &index)  const
{
    //TODO:实现选中后填入到model中的数据
    QComboBox *comboBox =  static_cast <QComboBox*>(editor);
    QString text = comboBox->currentText();
    model->setData(index, text, Qt::EditRole);
}

void  EnableBoxDelegate::updateEditorGeometry(QWidget *editor,
    const  QStyleOptionViewItem &option,  const  QModelIndex &index) const
{
    Q_UNUSED(index)

    editor->setGeometry(option.rect);
}
