#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#pragma execution_character_set("utf-8")    // 防止界面乱码

#include <QMainWindow>
#include <QMouseEvent>
#include <windows.h>
#include "tlhelp32.h"
#include <QDebug>
#include <QStack>
#include <QStandardPaths>
#include <QFileDialog>
#include <QDateTime>
#include <QMessageBox>
#include <QDragEnterEvent>
#include <QMimeData>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    // 使窗口支持拖动移动
    void mousePressEvent(QMouseEvent *event)
    {
        if (event->button() == Qt::LeftButton) {
            m_startPoint = frameGeometry().topLeft() - event->globalPos();
        }
    }
    void mouseMoveEvent(QMouseEvent *event)
    {
        this->move(event->globalPos() + m_startPoint);
    }
    friend void DbgOutput(char* szBuf);

private slots:
    void on_btnOpenFileDiag_clicked();

    void on_btnFresh_clicked();

    void on_btnClose_clicked();

    void on_btnMinimize_clicked();

    void on_btnLoad_clicked();

private:
    QPoint m_startPoint;

protected:
    void dragEnterEvent(QDragEnterEvent* e);	// 筛选拖拽事件
    void dropEvent(QDropEvent* e);				// 处理拖拽事件
};

DWORD GetPid(QString ProcessName);
bool LoadDll(DWORD dwProcessID, char* szDllPathName);


#endif // MAINWINDOW_H
