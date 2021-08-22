#ifndef RENAMETASK_H
#define RENAMETASK_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QList>
#include <QStringList>
#include <QThread>
enum RenameState{
    RenameState_Success = 0,
    RenameState_Dealing,
    RenameState_Failed
};
struct RenameResultItem
{
    int id;
    QString oldNmae;
    QString newName;
    RenameState state;
    QString filePath; //path + name
};
typedef QList<RenameResultItem> BatchRenameResults;

enum RenameSettingType{
    RenameSetting_Rename = 0,
   RenameSetting_Replace,
    RenameSetting_Insert
};

enum RenameSettingAddNumType{
    RenameSettingAddNum_Plain = 0,
    RenameSettingAddNum_SmallBrackets,
    RenameSettingAddNum_MediumBrackets,
    RenameSettingAddNum_BigBrackets,
    RenameSettingAddNum_Custom

};

enum OrientationType
{

    In_back = 0,
    In_Front = 1
};
struct AddNumSetting
{
    RenameSettingAddNumType addNumType;
    QString CustomLeftStr;
    QString CustomRightStr;
    OrientationType addNumPosition; // 0:尾部 1：头部
    QString seprater;
    int beginNum;
};
struct ReplaceSetting
{
    QString oldField;
    QString newField;
    bool caseSensitive;
};
struct InsertSetting
{
    int index;
    OrientationType orient;
    QString text;
};
struct RenameSetting
{
    RenameSettingType type;
    QString renameTo; // 直接重命名，默认带编号
    bool isAddNum;
    AddNumSetting addNumSetting;
    ReplaceSetting replaceSetting;
    InsertSetting insertSetting;
};

class RenameTask:public QThread
{
    Q_OBJECT
public:
    RenameTask(QObject* parent = nullptr);
    ~RenameTask();
    void addTask(QPair<QStringList, RenameSetting> &data);
protected:
    void run() override;


signals:
    void workFinished();
    void workStart();
    void renameDone(RenameResultItem result);
private:
    QString getNewFileName(QString &oldName);
     void addNum2FileName(QString &newfile);
    QString getOnlyRenameFileName();
    QString getReplaceFileName(QString &oldfilename);
    QString getInsertFileName(QString &oldfilename);
    void doWork();
private:
    QStringList m_filePaths;

    RenameSetting m_setting;
    int m_CurNum;
    QStringList m_newPaths;


};

#endif // RENAMETASK_H
