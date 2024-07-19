#include "Inject_.h"
#include <vector>
#include <Windows.h>
#include <TlHelp32.h>
#include <qdebug.h>

static DWORD d;

extern "C"
{
#include "./release/XEDParse.h"
#pragma comment(lib,"./release/XEDParse_x64.lib")
}
void Table(Ui::Inject_Class ui)
{
    ui.comboBox->clear();
    HANDLE hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcess != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 proc = { 0 };
        proc.dwSize = sizeof(proc);
        BOOL nRet = Process32First(hProcess, &proc);
        while (nRet)
        {
            QString s = QString("%1|%2")
                .arg(proc.th32ProcessID).arg(QString::fromWCharArray(proc.szExeFile));
            ui.comboBox->addItem(s);
            nRet = Process32Next(hProcess, &proc);
        }
    }
}
DWORD GetHwndByPid(DWORD dwProcessPid)
{
    HWND h = GetTopWindow(0);
    DWORD retHwnd = NULL;
    while (h)
    {
        DWORD pid;
        DWORD dwThreadId = GetWindowThreadProcessId(h, &pid);
        if (dwThreadId != 0)
        {
            if (d == pid && GetParent(h) == NULL && ::IsWindowVisible(h))
            {
                retHwnd = dwThreadId;
            }
        }
        h = GetNextWindow(h, GW_HWNDNEXT);
    }
    return retHwnd;
}
Inject_::Inject_(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    ui.pushButton->connect(ui.pushButton, &QPushButton::clicked, [=] {
        QStringList list = ui.textEdit->toPlainText().split("\n");
        XEDPARSE parse;
        std::vector<unsigned char> vec;
        for (QString s : list)
        {
            memset(&parse, 0, sizeof(parse));
            parse.x64 = false;
            parse.cip = 0;
            QByteArray ba = s.toLatin1();
            char* a = ba.data();
            printf("汇编:%s\n机器码:", a);
            strcpy(parse.instr, a);
            XEDParseAssemble(&parse);
            for (int i = 0; i < parse.dest_size; i++)
            {
                printf("%.2x ", parse.dest[i]);
                vec.push_back(parse.dest[i]);
            }
            printf("\n");
        }
        vec.push_back((unsigned char)0xC3);
        printf("OpCode:");
        BYTE* bytes = new BYTE[vec.size() + 1];
        int ii = 0;
        for (ii = 0; ii < vec.size(); ii++)
        {
            bytes[ii] = vec.at(ii);
            //printf("%.2x ", bytes[ii++]);
        }
        printf("\n");
        vec.clear();
        //获取到了机器码字节数组，接下来获取PID准备进行进行小操作
        QByteArray q = ui.comboBox->currentText().split("|")[0].toLatin1();
        d = q.toULongLong();
        printf("need Inject ->%lu...\n", d);
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, d);
        if (hProcess != INVALID_HANDLE_VALUE)
        {
            printf("准备写入内存字节:%d\n", ii);
            LPVOID Virtual_Add = VirtualAllocEx(hProcess, NULL, ii, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
            if (Virtual_Add != NULL)
            {
                SIZE_T OldProject = 0;
                if (WriteProcessMemory(hProcess, Virtual_Add, bytes, ii, &OldProject))
                {
                    if (CreateRemoteThread(hProcess,
                        NULL,
                        0,
                        (LPTHREAD_START_ROUTINE)Virtual_Add,
                        NULL,
                        0,
                        NULL))
                    {
                        delete[] bytes;
                        CloseHandle(hProcess);
                        printf("remoteThread Inject...\n");
                    }
                    else
                        MessageBoxA(0, "创建远程线程错误", "错误", 0);
                }
                else
                    MessageBoxA(0, "写入进程内存错误", "错误", 0);
            }
            else
                MessageBoxA(0, "创建虚拟内存错误", "错误", 0);

        }
        else
            MessageBoxA(0, "打开失败", "提示", 0);
        });
    connect(ui.pushButton_2, &QPushButton::clicked, [=] {
        Table(ui);
        });
    Table(ui);
}

Inject_::~Inject_()
{}
