﻿
// WheatJumpDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "WheatJump.h"
#include "WheatJumpDlg.h"
#include "afxdialogex.h"
// using namespace Gdiplus;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define CAPTION L"微信跳一跳V1.0"

// X,Y方向放大比例
float g_scaleX, g_scaleY;

// 手机截屏保存在电脑位置
WCHAR *g_szPicPath = L"./data/jump.png";
// 手机截屏保存在手机的位置
WCHAR *g_szRemotePath = L"/sdcard/jump.png";
// adb.exe程序位置
WCHAR *g_szAdbPath = L"./data/adb.exe";

// 电脑显示手机截屏图片的宽度
UINT g_nPicWidth = 540;
// 电脑显示手机截屏图片的高度
UINT g_nPicHeight = 960;

// 程序窗口宽度
UINT g_nWidth = 700;
// 程序窗口高度
UINT g_nHeight = 1020;

// 是否是小窗口
BOOL g_bSmall = FALSE;

UINT nCheckTimerId = 1;
// GDI+ token
ULONG_PTR           g_GdiplusToken;
void InitScale(HWND hWnd/*, Bitmap &bitmap*/);


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CWheatJumpDlg 对话框
CWheatJumpDlg::CWheatJumpDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_WHEATJUMP_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWheatJumpDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWheatJumpDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()

	ON_BN_CLICKED(IDC_BUTTON_SYNC_WECHAT_PIC, &CWheatJumpDlg::OnBnClickedButtonSyncWechatPic)
	ON_BN_CLICKED(IDC_BUTTON_FIND_START_POINT, &CWheatJumpDlg::OnBnClickedButtonFindStartPoint)
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_SIZE, &CWheatJumpDlg::OnBnClickedButtonChangeSize)
	ON_WM_CLOSE()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CWheatJumpDlg 消息处理程序
BOOL CWheatJumpDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	// 初始化GDI+
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&g_GdiplusToken, &gdiplusStartupInput, NULL);
	// 设置窗口大小
	this->MoveWindow(0, 0, g_nWidth, g_nHeight);
	// 初始化AdbHelper
	this->m_pAdbHelper = new AdbHelper(g_szAdbPath);
	// 初始化起始点和结束点
	this->m_pPointEnd = new Gdiplus::Point(0, 0);
	this->m_pPointStart = new Gdiplus::Point(0, 0);

	this->SetWindowTextW(CAPTION);
	this->SetTimer(nCheckTimerId, 4000, NULL);
	this->OnTimer(nCheckTimerId);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CWheatJumpDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 计算图片缩放
void InitScale(HWND hWnd)
{
	Bitmap bitmap(g_szPicPath);
	g_scaleX = (float)g_nPicWidth / bitmap.GetWidth();
	g_scaleY = (float)g_nPicHeight / bitmap.GetHeight();
}

// 以x,y为中心画十字光标
void DrawFlag(Graphics &graphics, Pen &pen, INT x, INT y)
{
	graphics.DrawLine(&pen, x - 10, y, x + 10, y);
	graphics.DrawLine(&pen, x, y - 10, x, y + 10);
}

// 窗口绘图主要部分
void CWheatJumpDlg::DrawSharp(CDC &cdc, Bitmap &bitmap)
{
	CDC memDC;
	// 兼容内存DC
	memDC.CreateCompatibleDC(&cdc);  
	CBitmap CompatibleBitmap;
 
	// 兼容位图
	CompatibleBitmap.CreateCompatibleBitmap(&cdc, g_nPicWidth, g_nPicHeight);
	// 载入该位图  
    memDC.SelectObject(&CompatibleBitmap);

	// 通过控件句柄创建一个Graphics对象，则其后续操作可认为作用在CompatibleBitmap上了。  
	Graphics graphics(memDC.GetSafeHdc());
	// 将需要的部分背景图片载入
	graphics.DrawImage(&bitmap, 0, 0, g_nPicWidth, g_nPicHeight );

	// 画笔，最后一个参数，画笔大小
	Pen newPen(Color(255, 0, 0), 2);
	// 画第一个点
	DrawFlag(graphics, newPen, this->m_pPointStart->X, this->m_pPointStart->Y);

	newPen.SetColor(Color(255, 255, 0));
	// 画第二个点
	DrawFlag(graphics, newPen, this->m_pPointEnd->X, this->m_pPointEnd->Y);

	newPen.SetColor(Color(0, 0, 0));
	// 画2个点的连线
	if (this->m_pPointStart->X > 0 && this->m_pPointStart->Y > 0 &&
		this->m_pPointEnd->X > 0 && this->m_pPointEnd > 0)
	{
		graphics.DrawLine(&newPen,
			this->m_pPointStart->X,
			this->m_pPointStart->Y,
			this->m_pPointEnd->X,
			this->m_pPointEnd->Y);
	}

	this->SHowStatus(&memDC);

	// 将内存DC内容输出到控件上
	cdc.BitBlt(0, 0, g_nPicWidth, g_nPicHeight, &memDC, 0, 0, SRCCOPY);  

	graphics.ReleaseHDC(cdc.GetSafeHdc());
	memDC.DeleteDC();
	
	return;
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。
void CWheatJumpDlg::OnPaint()
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
		PAINTSTRUCT ps;
		Bitmap bitmap(g_szPicPath);

		CDC *cdc = BeginPaint(&ps);
		// 绘图
		this->DrawSharp(*cdc, bitmap);

		this->EndPaint(&ps);
	}
}

// 获取微信跳的一个点(小人上的点)
void CWheatJumpDlg::GetStartPonit(Bitmap &bitmap)
{
	// 小人的颜色
	static int r = 56;
	static int g = 56;
	static int b = 98;

	this->m_pPointStart->X = this->m_pPointStart->Y = 0;
	Color color;
	
	for (int h = 500; h <bitmap.GetHeight() - 50; h += 5) {
		for (int w = 50; w < bitmap.GetWidth() - 50; w += 5) {
			bitmap.GetPixel(w, h, &color);

			if (abs(r - color.GetR()) < 5
				&& abs(g - color.GetG()) < 5
				&& abs(b - color.GetB()) < 5) {
				this->m_pPointStart->X = g_scaleX * w - 2;
				this->m_pPointStart->Y = g_scaleY * h + 2;
				//return;
			}
		}
	}

}
//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CWheatJumpDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 同步微信跳一跳屏幕图片
void CWheatJumpDlg::SyncWechat()
{
	this->m_pAdbHelper->Screenshot(g_szRemotePath);
	this->m_pAdbHelper->Copy(g_szRemotePath, g_szPicPath);
	this->FindAndShowStartPoint();
}

void CWheatJumpDlg::FindAndShowStartPoint()
{
	InitScale(this->m_hWnd);
	Bitmap bitmap(g_szPicPath);
	this->GetStartPonit(bitmap);
	this->m_pPointEnd->X = this->m_pPointEnd->Y = 0;
	InvalidateRect(NULL, FALSE);
}

// 鼠标左键按下
void CWheatJumpDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (point.x < g_nPicWidth)
	{
		this->m_pPointEnd->X = point.x;
		this->m_pPointEnd->Y = point.y;
		InvalidateRect(NULL, FALSE);

		if (this->m_pPointStart->X > 0 && this->m_pPointStart->Y > 0 &&
			this->m_pPointEnd->X > 0 && this->m_pPointEnd > 0)
		{
			this->Jump();
			this->SyncWechat();
		}

	}

}

// 邮件按下
void CWheatJumpDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (point.x < g_nPicWidth)
	{
		this->m_pPointStart->X = point.x;
		this->m_pPointStart->Y = point.y;
		InvalidateRect(NULL, FALSE);
	}

}


// 开始跳
void CWheatJumpDlg::Jump()
{
	float distance = sqrt(pow((this->m_pPointStart->X - this->m_pPointEnd->X), 2)
		+ pow((this->m_pPointStart->Y - this->m_pPointEnd->Y), 2));

	RECT rt;
	int  time = (int)(distance* 2.175f * 675 / (g_nPicWidth));
	wchar_t args[100] = {};

	int x = rand() % 20 + 100;
	int y = rand() % 20 + 110;

	swprintf_s(args, 100, L" shell input touchscreen swipe %d %d %d %d %d", x, y, x, y, time);
	TRACE(args);

	this->m_pAdbHelper->Exec(args);
	::Sleep(time + 300);
}

// 同步手机屏幕
void CWheatJumpDlg::OnBnClickedButtonSyncWechatPic()
{
	this->SyncWechat();
}


void CWheatJumpDlg::OnBnClickedButtonFindStartPoint()
{
	FindAndShowStartPoint();
}

// 显示状态
void CWheatJumpDlg::SHowStatus(CDC *cdc)
{
	InitScale(this->m_hWnd);

	RECT rect = {0};
	rect.left = 9;
	rect.top = 9;
	rect.right = g_nPicWidth - 9;
	rect.bottom = 29;

	cdc->SetBkMode(TRANSPARENT);

	cdc->SetTextColor(RGB(255, 0, 0));
	CFont font;
	UINT fontSize = 20;
	font.CreateFont(fontSize,        //   nHeight     
		0,                           //   nWidth     
		0,                           //   nEscapement   
		0,                           //   nOrientation     
		FW_NORMAL,                   //   nWeight     
		FALSE,                       //   bItalic     
		FALSE,                       //   bUnderline     
		0,                           //   cStrikeOut     
		ANSI_CHARSET,                //   nCharSet     
		OUT_DEFAULT_PRECIS,          //   nOutPrecision     
		CLIP_DEFAULT_PRECIS,         //   nClipPrecision     
		DEFAULT_QUALITY,             //   nQuality     
		DEFAULT_PITCH | FF_SWISS,    //   nPitchAndFamily       
		_T("宋体"));

	CFont *oldFont = cdc->SelectObject(&font);

	WCHAR szShow[255] = {};
	
	//cdc->DrawText(szShow, &rect, DT_EDITCONTROL);

	//rect.top += fontSize;
	swprintf_s(szShow, _T("scaleX:%.2f  scaleY:%.2f"), g_scaleX, g_scaleY);
	cdc->DrawText(szShow, &rect, DT_SINGLELINE);


	rect.top += fontSize;
rect.bottom += fontSize;
swprintf_s(szShow, _T("Point:%d,%d -> %d,%d"),
	this->m_pPointStart->X, this->m_pPointStart->Y, this->m_pPointEnd->X, this->m_pPointEnd->Y);
cdc->DrawText(szShow, &rect, DT_SINGLELINE);

rect.top += fontSize;
rect.bottom += fontSize;
if (this->m_pPointStart->X > 0 && this->m_pPointEnd->X > 0 &&
	this->m_pPointStart->Y > 0 && this->m_pPointEnd->Y > 0) {

	float distance = sqrt(pow((this->m_pPointStart->X - this->m_pPointEnd->X), 2)
		+ pow((this->m_pPointStart->Y - this->m_pPointEnd->Y), 2));

	int  time = (int)(distance* 2.175f * 675 / (g_nPicWidth));

	swprintf_s(szShow, _T("distance:%.2f presstime:%dms"), distance, time);
}
else {
	swprintf_s(szShow, _T("distance:- presstime:-"));
}


cdc->DrawText(szShow, &rect, DT_SINGLELINE);
cdc->SelectObject(oldFont);

}

// 鼠标移动消息
void CWheatJumpDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (point.x + 10 < g_nPicWidth)
	{
		this->m_pPointEnd->X = point.x;
		this->m_pPointEnd->Y = point.y;
		InvalidateRect(NULL, FALSE);
	}

	CDialog::OnMouseMove(nFlags, point);
}

// 改变窗口大小
void CWheatJumpDlg::OnBnClickedButtonChangeSize()
{
	if (g_bSmall)
	{
		g_bSmall = FALSE;

		float scale = 4.0f / 3;
		g_nPicWidth *= scale;
		g_nPicHeight *= scale;

		g_nHeight *= scale;
		//	g_nWidth *= scale;

	}
	else
	{
		g_bSmall = TRUE;

		float scale = 3.0 / 4;
		g_nPicWidth *= scale;
		g_nPicHeight *= scale;

		g_nHeight *= scale;
		//	g_nWidth *= scale;
	}

	TRACE("\ng_nPicWidth=%d,g_nPicHeight:%d\ng_nWidth:%d,g_nHeight:%d\n",
		g_nPicWidth, g_nPicHeight, g_nWidth, g_nHeight);


	// 设置窗口大小
	RECT rect;
	GetWindowRect(&rect);
	this->MoveWindow(rect.left, rect.top, g_nWidth, g_nHeight);

	// 窗口重绘
	InvalidateRect(NULL, TRUE);
}


void CWheatJumpDlg::PostNcDestroy()
{
	TRACE("关闭.g_GdiplusToken=%d", g_GdiplusToken);
	GdiplusShutdown(g_GdiplusToken);
	if (NULL != this->m_pAdbHelper)
	{
		delete this->m_pAdbHelper;
	}
	CDialog::PostNcDestroy();
}


void CWheatJumpDlg::OnTimer(UINT_PTR nIDEvent)
{
#ifdef DEBUG
	CString s;
	s.Append(CAPTION);
	s.Append( _T("  手机分辨率信息:"));

	this->m_pAdbHelper->Exec(_T(" shell wm size"), &s);
	TRACE(L"返回值:%s\n", s.GetString());
	 

	this->SetWindowTextW(s);
#endif
//	CDialog::OnTimer(nIDEvent);
}
