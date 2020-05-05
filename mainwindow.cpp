#include "mainwindow.h"
#include "ui_mainwindow.h"

Ui::MainWindow *ui;

MainWindow::MainWindow(QWidget *parent):QMainWindow(parent)
{
    ui = new Ui::MainWindow;
    ui->setupUi(this);
    ui->lbeTitle->setText("InJector");
    setWindowFlags(Qt::FramelessWindowHint);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void DbgOutput(char* szBuf)
{
    QDateTime curTime(QDateTime::currentDateTime());
    QString hms = curTime.toString("hh:mm:ss");
    QString info = QString::fromLocal8Bit(szBuf);
    qDebug() << "info=" << info;
    ui->tbrRunLog->append(hms + QString(" ") + info);
}

void MainWindow::on_btnOpenFileDiag_clicked()
{
    QString path_desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);   //获取桌面路径
    QString path_dll = QFileDialog::getOpenFileName(this, "请选择您要注入的dll", path_desktop, "*.dll");
    if(!path_dll.isEmpty())
        ui->edtDllPath->setText(path_dll);
}

// 刷新进程
void MainWindow::on_btnFresh_clicked()
{
    PROCESSENTRY32 pe32 = { 0 };
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    pe32.dwSize = sizeof(PROCESSENTRY32);
    QStack<QString> vStack;  // 用来装进程名
    if (Process32First(hSnap, &pe32))
    {
        do
        {
            QString ExeName = QString::fromWCharArray(pe32.szExeFile);
            vStack.push(ExeName);
        } while (Process32Next(hSnap, &pe32));
    }
    while(!vStack.isEmpty())
    {
        QString ExeName = vStack.pop();
        ui->lstProcess->addItem(ExeName);
    }
    CloseHandle(hSnap);
}

// 关闭窗口
void MainWindow::on_btnClose_clicked()
{
    QApplication::exit();
}

// 最小化窗口
void MainWindow::on_btnMinimize_clicked()
{
    showMinimized();
}

// 开始注入
void MainWindow::on_btnLoad_clicked()
{
    QString path_desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);   //获取桌面路径
    DWORD pid = GetPid(ui->lstProcess->currentItem()->text());
    QString path_dll = ui->edtDllPath->text();
    if (pid == NULL || path_dll.isEmpty())
        QMessageBox::warning(this, "警告", "dll路径或进程ID不得为空");
    else
    {
        QByteArray arr = path_dll.toUtf8();
        char* szPath_dll = arr.data();
        bool bRet = LoadDll(pid, szPath_dll);
        if (bRet == TRUE)
            DbgOutput("提示信息:注入成功!");

        else
            DbgOutput("提示信息:注入失败!");
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    e->acceptProposedAction();	// 放行，否则不会执行dropEvent()函数
}

void MainWindow::dropEvent(QDropEvent *e)
{
    //获取文件路径 (QString)
    QList<QUrl> urls = e->mimeData()->urls();
    if (urls.isEmpty()) return;
    QString qStrPath = urls.first().toLocalFile();
    ui->edtDllPath->setText(qStrPath);
}

DWORD GetPid(QString ProcessName)
{
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    PROCESSENTRY32 pe32 = { 0 };
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnap, &pe32))
    {
        do
        {
            QString ExeName = QString::fromWCharArray(pe32.szExeFile);
            if (ProcessName.compare(ExeName) == 0)
            {
                CloseHandle(hSnap);
                return (DWORD)pe32.th32ProcessID;
            }
        } while (Process32Next(hSnap, &pe32));
    }
    CloseHandle(hSnap);
    return 0;
}

bool LoadDll(DWORD dwProcessID, char* szDllPathName)
{
    BOOL bRet = 0;
    HANDLE hProcess = 0;
    HANDLE hThread = 0;
    DWORD dwLength = 0;
    DWORD dwLoadAddr = 0;
    LPVOID lpAllocAddr = 0;
    HMODULE hModule = 0;

    // 1 获取进程句柄
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessID);
    if (hProcess == NULL)
    {
        DbgOutput("OpenProcess Error!\n");
        return FALSE;
    }
    // 2 计算dll路径名称长度
    dwLength = strlen(szDllPathName) + 1;
    // 3 在目标进程分配内存
    lpAllocAddr = VirtualAllocEx(hProcess, NULL, dwLength, MEM_COMMIT, PAGE_READWRITE);
    if (lpAllocAddr == NULL)
    {
        DbgOutput("VirtualAllocEx Error!\n");
        CloseHandle(hProcess);
        return FALSE;
    }
    // 4 写入dll路径名到目标进程内存
    bRet = WriteProcessMemory(hProcess, lpAllocAddr, szDllPathName, dwLength, NULL);
    if (!bRet)
    {
        DbgOutput("WriteProcessMemory Error!\n");
        CloseHandle(hProcess);
        return FALSE;
    }
    // 5 获取模块地址
    hModule = GetModuleHandleA("Kernel32.dll");
    if (!bRet)
    {
        DbgOutput("GetModuleHandleA Error!\n");
        CloseHandle(hProcess);
        return FALSE;
    }
    // 6 获取LoadLibraryA函数地址
    dwLoadAddr = (DWORD)GetProcAddress(hModule, "LoadLibraryA");
    if (!dwLoadAddr)
    {
        DbgOutput("GetProcAddress Error!\n");
        CloseHandle(hProcess);
        CloseHandle(hModule);
        return FALSE;
    }
    // 7 创建远程线程,加载dll
    hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)dwLoadAddr, lpAllocAddr, 0, NULL);
    if (!hThread)
    {
        DbgOutput("CreateRemoteThread Error!\n");
        CloseHandle(hProcess);
        CloseHandle(hModule);
        return FALSE;
    }
    // 8 等待远程线程执行完毕,并关闭进程句柄
    WaitForSingleObject(hThread, -1);
    CloseHandle(hProcess);
    DbgOutput("注入成功!");
    return TRUE;
}
