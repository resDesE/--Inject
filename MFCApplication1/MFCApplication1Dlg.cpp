
// MFCApplication1Dlg.cpp: 实现文件
//

#include "stdafx.h"
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCApplication1Dlg 对话框



CMFCApplication1Dlg::CMFCApplication1Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCAPPLICATION1_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMFCApplication1Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCApplication1Dlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMFCApplication1Dlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CMFCApplication1Dlg 消息处理程序

BOOL CMFCApplication1Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCApplication1Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCApplication1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMFCApplication1Dlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!GetInjectDllPath())
	{
		return;
	}
	if (!GetDestAppName())
	{
		return;
	}

	DWORD dwPid = 0;
	dwPid = OnGetPid((char*)m_szDestAppName.GetBuffer());
	OnInject(dwPid, (char*)m_szDllPath.GetString());
}

void CMFCApplication1Dlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!GetInjectDllPath())
	{
		return;
	}
	if (!GetDestAppName())
	{
		return;
	}

	PROCESSENTRY32* pe32 = GetProcessPE((char*)m_szDestAppName.GetBuffer());
	if (pe32 == 0)
	{
		theApp.ShowAppMessageBox(NULL, "进程不存在", NULL, NULL);
		return;
	}
	UnInject(pe32, (char*)m_szDllPath.GetString());
}

PROCESSENTRY32* CMFCApplication1Dlg::GetProcessPE(char* szProcessName)
{
	BOOL bRet = FALSE;
	//PROCESSENTRY32 pe32;
	HANDLE hSnap;

	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	m_pe32.dwSize = sizeof(m_pe32);
	bRet = Process32First(hSnap, &m_pe32);
	while (bRet)
	{
		if (strcmp((char*)_bstr_t(m_pe32.szExeFile), szProcessName) == 0)
		{
			return &m_pe32;
		}
		bRet = Process32Next(hSnap, &m_pe32);
	}
	return 0;
}

DWORD CMFCApplication1Dlg::OnGetPid(char * szProcessName)
{
	PROCESSENTRY32 *pProcessEntry = GetProcessPE(szProcessName);
	if (pProcessEntry == 0)
	{
		theApp.ShowAppMessageBox(NULL, "进程不存在", NULL, NULL);
		return 0;
	}
	return pProcessEntry->th32ProcessID;
}

MODULEENTRY32* CMFCApplication1Dlg::GetModule(char* szDllModuleName, DWORD prcID)
{
	HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, prcID);
	//MODULEENTRY32 me32;
	m_me32.dwSize = sizeof(MODULEENTRY32);
	BOOL bRet = FALSE;
	bRet = Module32First(hModuleSnap, &m_me32);

	/*FILE* pFile;
	fopen_s(&pFile, "F:\\vs17project\\MFCApplication1\\dllrd.txt", "w+");*/
	while (bRet)
	{
		// 写入文件
		
		/*fwrite(m_me32.szExePath, MAX_PATH, 1, pFile);
		fwrite("\n", 1, 1, pFile);*/
		// end
		if (_tcsicmp(m_me32.szExePath, szDllModuleName) == 0)
		{
			//fclose(pFile);
			return &m_me32;
		}
		bRet = Module32Next(hModuleSnap, &m_me32);
	}
	//fclose(pFile);
	return 0;
}

void CMFCApplication1Dlg::OnInject(DWORD dwPid, char* szDllName)
{
	if(dwPid == 0 || strlen(szDllName) == 0)
	{
		return;
	}

	char *pFunName = "LoadLibraryA";

	//打开目标进程  
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);

	if (hProcess == NULL)
	{
		return;
	}

	//计算欲注入DLL文件完整路径的长度  
	int nDllLen = strlen(szDllName) + sizeof(char);

	//在目标进程申请一块长度为nDllLen大小的内存空间  
	PVOID pDllAddr = VirtualAllocEx(hProcess, NULL, nDllLen, MEM_COMMIT, PAGE_READWRITE);

	if (pDllAddr == NULL)
	{
		CloseHandle(hProcess);
		return;
	}

	DWORD dwWriteNum = 0;

	//将欲注入DLL文件的完整路径写入目标进程中申请的空间内  
	WriteProcessMemory(hProcess, pDllAddr, szDllName, nDllLen, &dwWriteNum);

	//获得LoadLibraryA()函数的地址
	FARPROC pFunAddr = GetProcAddress(GetModuleHandle("kernel32.dll"), pFunName);
	
	//创建远程线程  
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pFunAddr, pDllAddr, 0, NULL);
	auto er = GetLastError();
	WaitForSingleObject(hThread, INFINITE);

	CloseHandle(hThread);
	CloseHandle(hProcess);

	theApp.ShowAppMessageBox(NULL, "注入成功", NULL, NULL);
}

void CMFCApplication1Dlg::UnInject(PROCESSENTRY32* pProcessEntry, char *szDllName)
{
	if (strlen(szDllName) == 0)
	{
		return;
	}

	char *pFunName = "FreeLibrary";

	//打开目标进程  
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pProcessEntry->th32ProcessID);

	if (hProcess == NULL)
	{
		theApp.ShowAppMessageBox(NULL, "进程打开失败", NULL, NULL);
		return;
	}

	MODULEENTRY32 *me32 = GetModule(szDllName, pProcessEntry->th32ProcessID);
	if (me32 == 0)
	{
		theApp.ShowAppMessageBox(NULL, "此进程不存在该模块", NULL, NULL);
		return;
	}

	//获得LoadLibraryA()函数的地址
	FARPROC pFunAddr = GetProcAddress(GetModuleHandle("kernel32.dll"), pFunName);

	//创建远程线程  
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pFunAddr, me32->modBaseAddr, 0, NULL);
	auto er = GetLastError();
	WaitForSingleObject(hThread, INFINITE);

	CloseHandle(hThread);
	CloseHandle(hProcess);

	theApp.ShowAppMessageBox(NULL, "卸载成功", NULL, NULL);
}

BOOL CMFCApplication1Dlg::GetInjectDllPath()
{
	m_szDllPath.Empty();
	GetDlgItem(IDC_MFCEDITBROWSE1)->GetWindowText(m_szDllPath);
	if (m_szDllPath.IsEmpty())
	{
		theApp.ShowAppMessageBox(NULL, " 注入的DLL路径不存在", NULL, NULL);
		return FALSE;
	}
	return TRUE;
}

BOOL CMFCApplication1Dlg::GetDestAppName()
{
	m_szDestAppName.Empty();
	GetDlgItem(IDC_EDIT1)->GetWindowText(m_szDestAppName);
	if (m_szDestAppName.IsEmpty())
	{
		theApp.ShowAppMessageBox(NULL, "目标进程不存在", NULL, NULL);
		return FALSE;
	}
	return TRUE;
}
