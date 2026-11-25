//! Desktop window: title bar (drag to move), minimize, maximize, close, status bar, content frame.
class ROOT_AE3_Window : ROOT_AE3_UiListener
{
	protected static const int ID_TITLE = 1;
	protected static const int ID_CLOSE = 2;
	protected static const int ID_MAX = 3;
	protected static const int ID_MIN = 4;
	protected static const int ID_BODY = 5;
	protected static const float TITLE_H = 30;
	protected static const float STATUS_H = 22;

	ref ROOT_AE3_App m_App;
	ROOT_AE3_DesktopView m_Desktop;
	protected FrameWidget m_wFrame;
	protected FrameWidget m_wContent;
	protected TextWidget m_wStatus;
	protected float m_fX, m_fY, m_fW, m_fH;
	protected float m_fRestoreX, m_fRestoreY, m_fRestoreW, m_fRestoreH;
	protected bool m_bMax;
	protected bool m_bMin;
	protected bool m_bDragging;
	protected float m_fDragDX, m_fDragDY;

	//------------------------------------------------------------------------------------------------
	void ROOT_AE3_Window(ROOT_AE3_DesktopView desktop, ROOT_AE3_App app, float x, float y)
	{
		m_Desktop = desktop;
		m_App = app;
		app.m_Window = this;
		app.m_Desktop = desktop;
		vector size = app.GetSize();
		m_fX = x;
		m_fY = y;
		m_fW = size[0];
		m_fH = size[1];
		Create();
		app.Build();
	}

	//------------------------------------------------------------------------------------------------
	protected void Create()
	{
		m_wFrame = ROOT_AE3_UI.Frame(m_Desktop.GetWindowLayer(), m_fX, m_fY, m_fW, m_fH);
		ROOT_AE3_UI.Rect(m_wFrame, 0, 0, m_fW, m_fH, ROOT_AE3_UI.Rgb(0, 0, 0, 0.6), ID_BODY, this);
		ROOT_AE3_UI.Rect(m_wFrame, 1, 1, m_fW - 2, m_fH - 2, ROOT_AE3_Theme.Panel());
		ROOT_AE3_UI.Rect(m_wFrame, 1, 1, m_fW - 2, TITLE_H, ROOT_AE3_Theme.Bar(), ID_TITLE, this);
		ROOT_AE3_UI.Icon(m_wFrame, 8, 6, 18, m_App.GetIcon(), ROOT_AE3_Theme.Text());
		ROOT_AE3_UI.Text(m_wFrame, 32, 6, m_fW - 140, 20, m_App.GetTitle(), 16, ROOT_AE3_Theme.Text());
		ROOT_AE3_UI.Button(m_wFrame, m_fW - 96, 4, 28, 22, "_", ID_MIN, this, ROOT_AE3_Theme.Button(), ROOT_AE3_Theme.Text());
		ROOT_AE3_UI.Button(m_wFrame, m_fW - 64, 4, 28, 22, "[]", ID_MAX, this, ROOT_AE3_Theme.Button(), ROOT_AE3_Theme.Text());
		ROOT_AE3_UI.Button(m_wFrame, m_fW - 32, 4, 28, 22, "X", ID_CLOSE, this, ROOT_AE3_UI.Rgb(0.7, 0.15, 0.12), ROOT_AE3_UI.Rgb(1, 1, 1));
		m_wContent = ROOT_AE3_UI.Frame(m_wFrame, 1, TITLE_H + 1, m_fW - 2, m_fH - TITLE_H - STATUS_H - 2);
		ROOT_AE3_UI.Rect(m_wFrame, 1, m_fH - STATUS_H - 1, m_fW - 2, STATUS_H, ROOT_AE3_Theme.Bar());
		m_wStatus = ROOT_AE3_UI.Text(m_wFrame, 8, m_fH - STATUS_H + 2, m_fW - 16, 18, "", 13, ROOT_AE3_Theme.Muted());
	}

	//------------------------------------------------------------------------------------------------
	FrameWidget GetContent()
	{
		return m_wContent;
	}

	//------------------------------------------------------------------------------------------------
	float ContentWidth()
	{
		return m_fW - 2;
	}

	//------------------------------------------------------------------------------------------------
	float ContentHeight()
	{
		return m_fH - TITLE_H - STATUS_H - 2;
	}

	//------------------------------------------------------------------------------------------------
	void SetStatus(string text)
	{
		if (m_wStatus)
			m_wStatus.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	bool IsMinimized()
	{
		return m_bMin;
	}

	//------------------------------------------------------------------------------------------------
	void SetMinimized(bool min)
	{
		m_bMin = min;
		m_wFrame.SetVisible(!min);
		if (!min)
			BringToFront();
	}

	//------------------------------------------------------------------------------------------------
	void BringToFront()
	{
		m_wFrame.SetZOrder(m_Desktop.NextZ());
		m_Desktop.SetFocusWindow(this);
	}

	//------------------------------------------------------------------------------------------------
	//! Rebuilds the window at a new size.
	protected void Resize(float x, float y, float w, float h)
	{
		m_fX = x;
		m_fY = y;
		m_fW = w;
		m_fH = h;
		ReleaseHandlers();
		m_App.ReleaseHandlers();
		m_wFrame.RemoveFromHierarchy();
		Create();
		m_App.Redraw();
		BringToFront();
	}

	//------------------------------------------------------------------------------------------------
	void Destroy()
	{
		m_App.OnClose();
		m_App.ReleaseHandlers();
		ReleaseHandlers();
		if (m_wFrame)
			m_wFrame.RemoveFromHierarchy();
	}

	//------------------------------------------------------------------------------------------------
	void Update(float dt)
	{
		if (m_bDragging)
		{
			float mx, my;
			ROOT_AE3_UI.Mouse(mx, my);
			m_fX = mx - m_fDragDX;
			m_fY = Math.Max(0, my - m_fDragDY);
			FrameSlot.SetPos(m_wFrame, m_fX, m_fY);
			if (GetGame().GetInputManager().GetActionValue("MouseLeft") <= 0)
				m_bDragging = false;
		}

		m_App.Update(dt);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiMouseDown(Widget w, int id, int button)
	{
		BringToFront();
		if (id == ID_TITLE && button == 0 && !m_bMax)
		{
			float mx, my;
			ROOT_AE3_UI.Mouse(mx, my);
			m_fDragDX = mx - m_fX;
			m_fDragDY = my - m_fY;
			m_bDragging = true;
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiMouseUp(Widget w, int id, int button)
	{
		m_bDragging = false;
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void StopDrag()
	{
		m_bDragging = false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiDoubleClick(Widget w, int id)
	{
		if (id == ID_TITLE)
		{
			ToggleMax();
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void ToggleMax()
	{
		if (m_bMax)
		{
			m_bMax = false;
			Resize(m_fRestoreX, m_fRestoreY, m_fRestoreW, m_fRestoreH);
			return;
		}

		m_fRestoreX = m_fX;
		m_fRestoreY = m_fY;
		m_fRestoreW = m_fW;
		m_fRestoreH = m_fH;
		m_bMax = true;
		float sw, sh;
		m_Desktop.GetArea(sw, sh);
		Resize(0, 0, sw, sh);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		switch (id)
		{
			case ID_CLOSE: m_Desktop.CloseWindow(this); return true;
			case ID_MIN: SetMinimized(true); m_Desktop.RefreshTaskbar(); return true;
			case ID_MAX: ToggleMax(); return true;
		}

		return false;
	}
}
