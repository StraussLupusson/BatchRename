#ifndef BATCHRENAMEWIDGET_H
#define BATCHRENAMEWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QScopedPointer>
#include "renametask.h"
namespace Ui {
class BatchRenameWidget;
}

class BatchRenameWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BatchRenameWidget(QWidget *parent = nullptr);
    ~BatchRenameWidget();

private slots:
    void on_pushButton_addFile_clicked();

    void on_pushButton_start_clicked();

    void on_pushButton_clicked();

    void on_pushButton_clearFile_clicked();

    void on_pushButton_clearSuccessFiles_clicked();

    void showCustomui(bool arg);

protected:
    void dragEnterEvent(QDragEnterEvent* e);
    void dropEvent(QDropEvent* e);
private:
    void initUI();
    void dealDrapEvent(QStringList &filelists);
    void getRenameSetting(RenameSetting &data);
    RenameSettingType getCurrentRenameSettingType();

private:
    Ui::BatchRenameWidget *ui;
    QStandardItemModel *m_FilesModel;
    QStringList m_FileLists;
    BatchRenameResults m_RenameResult;
    RenameSetting m_RenameSetting;
    QScopedPointer<RenameTask> m_RenameTask;
};

#endif // BATCHRENAMEWIDGET_H
