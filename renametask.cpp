#include "renametask.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QThread>
RenameTask::RenameTask(QObject* parent) :
    QThread (parent)
  ,m_filePaths({})
  ,m_setting({})
  ,m_newPaths({})
{
    qRegisterMetaType<RenameResultItem>("RenameResultItem");
}
RenameTask::~RenameTask()
{
}

void RenameTask::addTask(QPair<QStringList, RenameSetting> &data)
{
    qDebug() << "addtask";
    m_filePaths =  data.first;
    m_setting = data.second;
    m_CurNum = m_setting.addNumSetting.beginNum;

}

void RenameTask::run()
{
    doWork();
}

QString RenameTask::getNewFileName(QString &oldName)
{
   QFileInfo info(oldName);
   QString filename = info.completeBaseName();
   QString newfile;
   switch ( m_setting.type) {

   case RenameSetting_Rename:
   {
       newfile =  getOnlyRenameFileName();
   }

       break;
   case RenameSetting_Replace:
       newfile = getReplaceFileName(filename);
       break;
   case RenameSetting_Insert:
       newfile = getInsertFileName(filename);
       break;
   }
   return info.path()+QDir::separator() + newfile + '.' + info.suffix();
}

void RenameTask::addNum2FileName(QString &newfile)
{
    if(m_setting.addNumSetting.addNumPosition == In_back)
        newfile+= m_setting.addNumSetting.seprater;
    else
        newfile.prepend( m_setting.addNumSetting.seprater);

    switch (m_setting.addNumSetting.addNumType) {
    case RenameSettingAddNum_Plain:
        m_setting.addNumSetting.addNumPosition == In_back ? newfile += QString::number(m_CurNum++) : newfile.prepend(QString::number(m_CurNum++));
        break;
    case RenameSettingAddNum_SmallBrackets:
        m_setting.addNumSetting.addNumPosition == In_back ? newfile += QString("(%1)").arg(m_CurNum++) : newfile.prepend(QString("(%1)").arg(m_CurNum++));
        break;
    case RenameSettingAddNum_MediumBrackets:
        m_setting.addNumSetting.addNumPosition == In_back ? newfile += QString("[%1]").arg(m_CurNum++) : newfile.prepend(QString("[%1]").arg(m_CurNum++));
        break;
    case RenameSettingAddNum_BigBrackets:
        m_setting.addNumSetting.addNumPosition == In_back ? newfile += QString("{%1}").arg(m_CurNum++) : newfile.prepend(QString("{%1}").arg(m_CurNum++));
        break;
    default:
        break;
    }
}

QString RenameTask::getOnlyRenameFileName()
{

    QString newfile =  m_setting.renameTo;
    if(m_setting.isAddNum)
        addNum2FileName(newfile);
    return newfile;
}

QString RenameTask::getReplaceFileName(QString &oldfilename)
{
    QString newfile = oldfilename;

    if(m_setting.replaceSetting.oldField != m_setting.replaceSetting.newField)
    {
        qDebug() << "case: "<< m_setting.replaceSetting.caseSensitive;

         newfile.replace(m_setting.replaceSetting.oldField, m_setting.replaceSetting.newField,
                         m_setting.replaceSetting.caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
    }
    qDebug() << newfile;
    if(m_setting.isAddNum)
        addNum2FileName(newfile);
    return newfile;
}

QString RenameTask::getInsertFileName(QString &oldfilename)
{
    QString newfile = oldfilename;
    if(m_setting.insertSetting.index <= newfile.size())
        newfile.insert(m_setting.insertSetting.index-1,m_setting.insertSetting.text);
    if(m_setting.isAddNum)
        addNum2FileName(newfile);
    return newfile;
}

void RenameTask::doWork()
{
    // todo
    if(m_filePaths.isEmpty())
        return;
    int i = 0;
    while(!m_filePaths.isEmpty()) //
    {
        QString filepath = m_filePaths.front();
        QFileInfo info(filepath);
        m_filePaths.pop_front();

        QString newfilepath = getNewFileName(filepath);
        qDebug() << "newfilepath:" << newfilepath;
        bool ret = QFile::rename(filepath, newfilepath);
        qDebug() << "renameOK:" << ret;
        RenameResultItem item;
        item.id = i++;
        item.filePath = info.path();
        item.state = ret ? RenameState_Success :RenameState_Failed;
        item.oldNmae = info.fileName();
        item.newName = ret ? QFileInfo(newfilepath).fileName() : "";
        emit renameDone(item);
    }

    emit workFinished();
}
