#include "batchrenamewidget.h"
#include "ui_batchrenamewidget.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QDebug>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDir>

BatchRenameWidget::BatchRenameWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BatchRenameWidget)
  , m_FilesModel(new QStandardItemModel(this))
  ,m_FileLists({})
  ,m_RenameTask(new RenameTask())
{
    ui->setupUi(this);
     initUI();
    setAcceptDrops(true);


    ui->tableView->setModel(m_FilesModel);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setDefaultSectionSize(ui->tableView->width()/6);

    ui->stackedWidget_selectFile->setCurrentIndex(0);
    //m_RenameResult.clear();

    connect(m_RenameTask.get(), &RenameTask::workFinished, this, [&]{
        ui->pushButton_start->setEnabled(false);
        ui->pushButton_clearFile->setEnabled(true);
         ui->pushButton_clearSuccessFiles->setEnabled(true);

        qDebug()<< "renametask all done";
    });



    connect(m_RenameTask.get(), &RenameTask::renameDone, this, [&](RenameResultItem result){
        bool isSucess = result.state == RenameState_Success;
        m_FilesModel->setData(m_FilesModel->index(result.id,2),result.newName);
         m_FilesModel->setData(m_FilesModel->index(result.id,3),isSucess ? "成功" : "失败");
         m_FilesModel->setData(m_FilesModel->index(result.id,4),result.filePath);
         if(isSucess)
            m_FilesModel->item(result.id,3)->setForeground(QBrush(Qt::green));
         else
            m_FilesModel->item(result.id,3)->setForeground(QBrush(Qt::red));
         //m_RenameResult << result;

    },Qt::QueuedConnection);

}

BatchRenameWidget::~BatchRenameWidget()
{
    delete ui;
}

void BatchRenameWidget::showCustomui(bool arg)
{
    ui->label_13->setVisible(arg);
    ui->label_14->setVisible(arg);
    ui->lineEdit_leftstr->setVisible(arg);
    ui->lineEdit_rightstr->setVisible(arg);
}

void BatchRenameWidget::initUI()
{
    ui->stackedWidget_Renamesetting->setCurrentIndex(0);
    ui->radioButton_rename->setChecked(true);
    ui->checkBox_isaddnum->setChecked(true);
    ui->pushButton_start->setEnabled(false);
    ui->pushButton_clearFile->setEnabled(false);
    ui->pushButton_clearSuccessFiles->setEnabled(false);
    showCustomui(false);
    connect(ui->comboBox_numtype, QOverload<int>::of(&QComboBox::activated),[=](int index){
        if(index == 4)
            showCustomui(true);
        else
            showCustomui(false);

    });

    connect(ui->radioButton_rename,&QRadioButton::clicked,ui->stackedWidget_Renamesetting, [&]{
        ui->stackedWidget_Renamesetting->setCurrentIndex(0);
        ui->checkBox_isaddnum->setChecked(true);
        ui->checkBox_isaddnum->setEnabled(false);
    });
    connect(ui->radioButton_replace,&QRadioButton::clicked,ui->stackedWidget_Renamesetting, [&]{
        ui->stackedWidget_Renamesetting->setCurrentIndex(1);
         ui->checkBox_isaddnum->setEnabled(true);
    });
    connect(ui->radioButton_insert,&QRadioButton::clicked,ui->stackedWidget_Renamesetting, [&]{
        ui->stackedWidget_Renamesetting->setCurrentIndex(2);
         ui->checkBox_isaddnum->setEnabled(true);
    });
    connect(ui->checkBox_isaddnum,&QCheckBox::stateChanged, this, [&]{
        ui->groupBox_NumSetting->setEnabled(ui->checkBox_isaddnum->isChecked());
    });
}
void BatchRenameWidget::on_pushButton_addFile_clicked()
{
    QList<QUrl> fileslists = QFileDialog::getOpenFileUrls(
                              this,
                              "Select one or more files to open",
                              QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                              tr("all Documents (*.txt *.pdf *.doc *.ppt)"));

    QStringList filelists;
    foreach(QUrl url, fileslists)
    {
        QString filepath =  url.toLocalFile();
       //qDebug() <<filepath;
       if(!m_FileLists.contains(filepath))
            filelists << filepath;
    }
    dealDrapEvent(filelists);

}

void BatchRenameWidget::dragEnterEvent(QDragEnterEvent *e)
{
    if(e->mimeData()->hasUrls())
        {
            e->acceptProposedAction();
        }
        else
        {
            e->ignore();
        }

}

void BatchRenameWidget::dropEvent(QDropEvent *e)
{
    if(e->mimeData()->hasUrls())
       {

           QList<QUrl> list = e->mimeData()->urls();
           QStringList filelists;
           filelists.clear();
           for(int i = 0; i < list.count(); i++)
           {
               if(!m_FileLists.contains(list[i].toLocalFile()))
                filelists << list[i].toLocalFile();

           }
           dealDrapEvent(filelists);
       }
       else
       {
           e->ignore();
    }
}



void BatchRenameWidget::dealDrapEvent( QStringList &filelists)
{

    if(filelists.isEmpty())
        return;
    for(int i = 0; i< filelists.size(); i++)
    {
        QString filepath = filelists.at(i);
        m_FileLists << filepath;
        QFileInfo info(filepath);
        QList<QStandardItem*> add_items;
        add_items << new QStandardItem(QString::number(i));
        add_items << new QStandardItem(info.fileName());
        add_items << new QStandardItem("");
        add_items << new QStandardItem("待转换");
        add_items << new QStandardItem(QFileInfo(filepath).path());
        m_FilesModel->appendRow(add_items);
    }
//    ui->tableView->resizeRowsToContents();
//    ui->tableView->resizeColumnsToContents();
    ui->stackedWidget_selectFile->setCurrentIndex(1);

    ui->pushButton_start->setEnabled(true);
    ui->pushButton_clearFile->setEnabled(true);

    m_FilesModel->setHorizontalHeaderLabels({"序号","旧文件名", "新文件名", "状态", "路径"});

}

void BatchRenameWidget::getRenameSetting(RenameSetting &data)
{
    data.isAddNum = ui->checkBox_isaddnum->isChecked();
    data.type = getCurrentRenameSettingType();
    data.renameTo = ui->lineEdit_rename->text();

    data.addNumSetting.seprater = ui->lineEdit_seprator->text();
    data.addNumSetting.addNumType = static_cast<RenameSettingAddNumType>(ui->comboBox_numtype->currentIndex());
    data.addNumSetting.seprater = ui->lineEdit_seprator->text();
    data.addNumSetting.addNumPosition = static_cast<OrientationType>(ui->comboBox_numpos->currentIndex());
    data.addNumSetting.beginNum = ui->spinBox->value();
    data.addNumSetting.CustomLeftStr = ui->lineEdit_leftstr->text();
    data.addNumSetting.CustomRightStr = ui->lineEdit_rightstr->text();

    data.insertSetting.index = ui->spinBox_insertIndex->value();
    data.insertSetting.text = ui->lineEdit_insertFiled->text();
    data.replaceSetting.oldField = ui->lineEdit_srcField->text();
     data.replaceSetting.newField = ui->lineEdit_dstField->text();
     data.replaceSetting.caseSensitive = ui->checkBox_cases->isChecked();
}

RenameSettingType BatchRenameWidget::getCurrentRenameSettingType()
{
   if( ui->radioButton_rename->isChecked())
       return RenameSettingType::RenameSetting_Rename;
   if( ui->radioButton_replace->isChecked())
       return RenameSettingType::RenameSetting_Replace;
   if( ui->radioButton_insert->isChecked())
       return RenameSettingType::RenameSetting_Insert;
   return  RenameSettingType::RenameSetting_Rename;
}



void BatchRenameWidget::on_pushButton_start_clicked()
{
    RenameSetting setting;
    getRenameSetting(setting);
    QPair<QStringList, RenameSetting> pair = qMakePair(m_FileLists, setting);
    m_RenameTask->addTask(pair);
    m_RenameTask->start();

    ui->pushButton_start->setEnabled(false);

    //qDebug() << "on_pushButton_start_clicked";
}

void BatchRenameWidget::on_pushButton_clicked()
{
    on_pushButton_addFile_clicked();
}

void BatchRenameWidget::on_pushButton_clearFile_clicked()
{
    m_FilesModel->clear();
    m_FileLists.clear();
    ui->stackedWidget_selectFile->setCurrentIndex(0);
}

void BatchRenameWidget::on_pushButton_clearSuccessFiles_clicked()
{
//   foreach(auto var, m_RenameResult)
//   {
//      if( var.state == RenameState_Success)
//      {
//          //m_FileLists.removeAll(var.filePath + QDir::separator() + var.oldNmae);

//      }

//   }
//   ui->pushButton_start->setEnabled(true);
//   ui->pushButton_clearFile->setEnabled(true);
}
