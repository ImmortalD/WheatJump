
// WheatJumpDlg.h: 头文件
//

#pragma once
#include "afxwin.h"


// CWheatJumpDlg 对话框
class CWheatJumpDlg : public CDialog
{
// 构造
public:
	CWheatJumpDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WHEATJUMP_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
private:
	void Jump();
	void GetStartPonit(Bitmap &bitmap);
	void DrawSharp(CDC &cdc,Bitmap &bitmap);
	void SyncWechat();
	void FindAndShowStartPoint();
	void SHowStatus(CDC*);
	Point *m_pPointStart;
	Point *m_pPointEnd;
	AdbHelper *m_pAdbHelper;
public:
	afx_msg void OnBnClickedButtonSyncWechatPic();
	afx_msg void OnBnClickedButtonFindStartPoint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedButtonChangeSize();
	virtual void PostNcDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
