// HistoryTrafficCalendarDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "TrafficMonitor.h"
#include "HistoryTrafficCalendarDlg.h"
#include "afxdialogex.h"


// CHistoryTrafficCalendarDlg 对话框

IMPLEMENT_DYNAMIC(CHistoryTrafficCalendarDlg, CTabDlg)

CHistoryTrafficCalendarDlg::CHistoryTrafficCalendarDlg(deque<HistoryTraffic>& history_traffics, CWnd* pParent /*=nullptr*/)
	: CTabDlg(IDD_HISTORY_TRAFFIC_CALENDAR_DIALOG, pParent), m_history_traffics(history_traffics)
{

}

CHistoryTrafficCalendarDlg::~CHistoryTrafficCalendarDlg()
{
}

void CHistoryTrafficCalendarDlg::SetDayTraffic()
{
	for (int i{}; i < CALENDAR_HEIGHT; i++)
	{
		for (int j{}; j < CALENDAR_WIDTH; j++)
		{
			HistoryTraffic history_traffic;
			history_traffic.year = m_year;
			history_traffic.month = m_month;
			history_traffic.day = m_calendar[i][j].day;
			//使用二分法查找日历中每一个日期的流量
			if (history_traffic.day > 0)
			{
				if (std::binary_search(m_history_traffics.begin(), m_history_traffics.end(), history_traffic, HistoryTraffic::DateGreater))
				{
					auto iter = std::lower_bound(m_history_traffics.begin(), m_history_traffics.end(), history_traffic, HistoryTraffic::DateGreater);
					if(iter != m_history_traffics.end())
						m_calendar[i][j].traffic = iter->kBytes;
				}
			}
		}
	}
}

void CHistoryTrafficCalendarDlg::MonthSelectChanged()
{
	CCalendarHelper::GetCalendar(m_year, m_month, m_calendar);
	SetDayTraffic();
	CalculateMonthTotalTraffic();
	InvalidateRect(m_draw_rect);
}

void CHistoryTrafficCalendarDlg::CalculateMonthTotalTraffic()
{
	m_month_total_traffic = 0;
	for (int i{}; i < CALENDAR_HEIGHT; i++)
	{
		for (int j{}; j < CALENDAR_WIDTH; j++)
		{
			m_month_total_traffic += m_calendar[i][j].traffic;
		}
	}
}

void CHistoryTrafficCalendarDlg::SetComboSel()
{
	int cnt{};
	int year_selected;
	for (int i{ m_year_max }; i >= m_year_min; i--)
	{
		if (i == m_year)
			year_selected = cnt;
		cnt++;
	}
	m_year_combo.SetCurSel(year_selected);
	m_month_combo.SetCurSel(m_month - 1);
}

void CHistoryTrafficCalendarDlg::DoDataExchange(CDataExchange* pDX)
{
	CTabDlg::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_YEAR_COMBO, m_year_combo);
	DDX_Control(pDX, IDC_MONTH_COMBO, m_month_combo);
}


BEGIN_MESSAGE_MAP(CHistoryTrafficCalendarDlg, CTabDlg)
	ON_WM_PAINT()
	ON_CBN_SELCHANGE(IDC_YEAR_COMBO, &CHistoryTrafficCalendarDlg::OnCbnSelchangeYearCombo)
	ON_CBN_SELCHANGE(IDC_MONTH_COMBO, &CHistoryTrafficCalendarDlg::OnCbnSelchangeMonthCombo)
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_PREVIOUS_BUTTON, &CHistoryTrafficCalendarDlg::OnBnClickedPreviousButton)
	ON_BN_CLICKED(IDC_NEXT_BUTTON, &CHistoryTrafficCalendarDlg::OnBnClickedNextButton)
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()


// CHistoryTrafficCalendarDlg 消息处理程序


BOOL CHistoryTrafficCalendarDlg::OnInitDialog()
{
	CTabDlg::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_year = m_history_traffics[0].year;
	m_month = m_history_traffics[0].month;
	CCalendarHelper::GetCalendar(m_year, m_month, m_calendar);
	SetDayTraffic();
	CalculateMonthTotalTraffic();

	//初始化Combo Box
	m_year_max = m_history_traffics[0].year;
	m_year_min = m_history_traffics.back().year;
	for (int i{ m_year_max }; i >= m_year_min; i--)
	{
		m_year_combo.AddString(CCommon::IntToString(i));
	}
	m_year_combo.SetCurSel(0);
	for (int i{ 1 }; i <= 12; i++)
	{
		m_month_combo.AddString(CCommon::IntToString(i));
	}
	m_month_combo.SetCurSel(m_month - 1);

	//初始化鼠标提示
	m_tool_tips.Create(this, TTS_ALWAYSTIP | TTS_NOPREFIX);
	m_tool_tips.SetMaxTipWidth(800);		//为鼠标提示设置一个最大宽度，以允许其换行
	m_tool_tips.AddTool(this, _T(""));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CHistoryTrafficCalendarDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CTabDlg::OnPaint()
	const int width = theApp.DPI(40);		//日历中每一个格子的宽度
	const int height = theApp.DPI(30);		//日历中第一个格子的高度
	m_draw_rect.left = m_start_x;
	m_draw_rect.top = m_start_y;
	m_draw_rect.right = m_draw_rect.left + (CALENDAR_WIDTH*width);
	m_draw_rect.bottom = m_draw_rect.top + ((CALENDAR_HEIGHT + 1)*height) + theApp.DPI(20);

	//设置缓冲的DC
	CDC MemDC;
	CBitmap MemBitmap;
	MemDC.CreateCompatibleDC(NULL);
	MemBitmap.CreateCompatibleBitmap(&dc, m_draw_rect.Width(), m_draw_rect.Height());
	MemDC.SelectObject(&MemBitmap);
	//绘图
	CDrawCommon draw;
	draw.Create(&MemDC, this);
	draw.FillRect(CRect(0, 0, m_draw_rect.Width(), m_draw_rect.Height()), RGB(255, 255, 255));		//填充白色背景色
	CRect rect{};
	rect.left = 0;
	rect.top = 0;
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;
	//画星期的行
	for (int i{}; i < CALENDAR_WIDTH; i++)
	{
		rect.MoveToX(i * width);
		
		if (i == 0 || i == 6)
			draw.SetBackColor(RGB(217, 86, 86));
		else if (i % 2 == 0)
			draw.SetBackColor(RGB(1, 84, 151));
		else
			draw.SetBackColor(RGB(1, 107, 191));
		CString str;
		switch (i)
		{
		case 0:
			str.LoadString(IDS_SUNDAY);
			break;
		case 1:
			str.LoadString(IDS_MONDAY);
			break;
		case 2:
			str.LoadString(IDS_TUESDAY);
			break;
		case 3:
			str.LoadString(IDS_WEDNESDAY);
			break;
		case 4:
			str.LoadString(IDS_THURSDAY);
			break;
		case 5:
			str.LoadString(IDS_FRIDAY);
			break;
		case 6:
			str.LoadString(IDS_SATURDAY);
			break;
		}
		draw.DrawWindowText(rect, str, RGB(255, 255, 255), Alignment::CENTER, true);
	}

	//绘制日历
	for (int i{}; i < CALENDAR_HEIGHT; i++)
	{
		for (int j{}; j < CALENDAR_WIDTH; j++)
		{
			//设置日历中每一天的矩形的位置
			rect.MoveToXY(j * width, height + theApp.DPI(2) + i * height);
			//保存矩形的位置
			m_calendar[i][j].rect = rect;
			//绘制日期的数据
			//绘制格子的背景颜色
			if (j == 0 || j == 6)		//是周末时
			{
				if ((i + j) % 2 == 0)
					draw.SetBackColor(RGB(250, 234, 234));
				else
					draw.SetBackColor(RGB(252, 242, 242));
			}
			else
			{
				if ((i + j) % 2 == 0)
					draw.SetBackColor(RGB(226, 241, 254));
				else
					draw.SetBackColor(RGB(236, 246, 254));
			}
			draw.FillRectWithBackColor(rect);
			//绘制格子上的日期的数字
			CRect day_rect{ rect };
			day_rect.bottom -= (rect.Height() / 2);
			COLORREF text_color;
			if (j == 0 || j == 6)
				text_color = RGB(131, 29, 28);
			else
				text_color = RGB(0, 57, 107);
			if (m_calendar[i][j].day != 0)
				draw.DrawWindowText(day_rect, CCommon::IntToString(m_calendar[i][j].day), text_color, Alignment::CENTER, true);
			//在今天的日期上画一个矩形框
			COLORREF frame_color;
			if (j == 0 || j == 6)
				frame_color = RGB(218, 91, 91);
			else
				frame_color = RGB(1, 133, 238);
			if (m_year == m_history_traffics[0].year && m_month == m_history_traffics[0].month && m_calendar[i][j].day == m_history_traffics[0].day)
				draw.DrawRectOutLine(rect, frame_color, theApp.DPI(2));

			//绘制指示流量大小的矩形
			COLORREF color;
			if (m_calendar[i][j].traffic < 102400)		//流量小于100MB时绘制蓝色
				color = TRAFFIC_COLOR_BLUE;
			else if (m_calendar[i][j].traffic < 1024 * 1024)	//流量小于1GB时绘制绿色
				color = TRAFFIC_COLOR_GREEN;
			else if (m_calendar[i][j].traffic < 10 * 1024 * 1024)	//流量小于10GB时绘制黄色
				color = TRAFFIC_COLOR_YELLOE;
			else		//流量大于10GB时绘制红色
				color = TRAFFIC_COLOR_RED;
			if (m_calendar[i][j].traffic > 0)
			{
				CRect traffic_rect;
				traffic_rect.left = rect.left + theApp.DPI(14);
				traffic_rect.right = traffic_rect.left + theApp.DPI(12);
				traffic_rect.top = rect.top + theApp.DPI(16);
				traffic_rect.bottom = traffic_rect.top + theApp.DPI(12);
				draw.FillRect(traffic_rect, color);
			}
		}
	}
	//画当前月总流量
	CString info = CCommon::LoadText(IDS_CURRENT_MONTH_TOTAL_TRAFFIC, CCommon::KBytesToStringL(m_month_total_traffic));
	CRect info_rect;
	info_rect.left = 0;
	info_rect.top = height * (CALENDAR_HEIGHT + 1) + theApp.DPI(5);
	info_rect.right = rect.left + (CALENDAR_WIDTH*width);
	info_rect.bottom = info_rect.top + theApp.DPI(15);
	draw.SetBackColor(RGB(255, 255, 255));
	draw.DrawWindowText(info_rect, info, RGB(0, 57, 107), Alignment::LEFT, true);

	//将缓冲区DC中的图像拷贝到屏幕中显示
	dc.BitBlt(m_start_x, m_start_y, m_draw_rect.Width(), m_draw_rect.Height(), &MemDC, 0, 0, SRCCOPY);
	MemBitmap.DeleteObject();
	MemDC.DeleteDC();
}


void CHistoryTrafficCalendarDlg::OnCbnSelchangeYearCombo()
{
	// TODO: 在此添加控件通知处理程序代码
	int index = m_year_combo.GetCurSel();
	CString str;
	m_year_combo.GetLBText(index, str);
	m_year = _ttoi(str);
	MonthSelectChanged();
}


void CHistoryTrafficCalendarDlg::OnCbnSelchangeMonthCombo()
{
	// TODO: 在此添加控件通知处理程序代码
	m_month = m_month_combo.GetCurSel() + 1;
	MonthSelectChanged();
}


void CHistoryTrafficCalendarDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	int tip_day{};		//要显示鼠标提示的日期
	unsigned int tip_traffic{};
	static int last_tip_day{};
	//查找鼠标指针在哪个日期的矩形内
	for (int i{}; i < CALENDAR_HEIGHT; i++)
	{
		for (int j{}; j < CALENDAR_WIDTH; j++)
		{
			CRect rect{ m_calendar[i][j].rect };
			rect.OffsetRect(m_start_x, m_start_y);
			if (rect.PtInRect(point))
			{
				tip_day = m_calendar[i][j].day;
				tip_traffic = m_calendar[i][j].traffic;
			}
		}
	}
	bool show_tip = (tip_day > 0);
	if (show_tip && last_tip_day != tip_day)
	{
		CString tip_info;
		tip_info.Format(_T("%d/%d/%d\r\n"), m_year, m_month, tip_day);
		tip_info += CCommon::LoadText(IDS_TRAFFIC_USED1);
		tip_info += CCommon::KBytesToString(tip_traffic);
		m_tool_tips.AddTool(this, tip_info);
		m_tool_tips.Pop();
		last_tip_day = tip_day;
	}
	if (!show_tip)
	{
		m_tool_tips.AddTool(this, _T(""));
		m_tool_tips.Pop();
		last_tip_day = 0;
	}

	CTabDlg::OnMouseMove(nFlags, point);
}


BOOL CHistoryTrafficCalendarDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (m_tool_tips.GetSafeHwnd() != 0)
	{
		m_tool_tips.RelayEvent(pMsg);
	}

	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_LEFT)
		{
			OnBnClickedPreviousButton();
			return TRUE;
		}
		if (pMsg->wParam == VK_RIGHT)
		{
			OnBnClickedNextButton();
			return TRUE;
		}
	}

	return CTabDlg::PreTranslateMessage(pMsg);
}


void CHistoryTrafficCalendarDlg::OnBnClickedPreviousButton()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_year == m_year_min && m_month == 1)
		return;
	m_month--;
	if (m_month <= 0)
	{
		m_month = 12;
		m_year--;
	}
	SetComboSel();
	MonthSelectChanged();
}


void CHistoryTrafficCalendarDlg::OnBnClickedNextButton()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_year == m_year_max && m_month == 12)
		return;
	m_month++;
	if (m_month > 12)
	{
		m_month = 1;
		m_year++;
	}
	SetComboSel();
	MonthSelectChanged();
}


BOOL CHistoryTrafficCalendarDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//通过鼠标滚轮翻页
	if (zDelta > 0)
	{
		OnBnClickedPreviousButton();
	}
	if (zDelta < 0)
	{
		OnBnClickedNextButton();
	}

	return CTabDlg::OnMouseWheel(nFlags, zDelta, pt);
}
