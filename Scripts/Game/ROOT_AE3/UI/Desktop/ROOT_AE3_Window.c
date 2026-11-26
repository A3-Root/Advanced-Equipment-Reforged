//! Desktop window: title bar (drag to move, drop on the top edge to maximize, on a side edge to fill that half),
//! minimize, maximize, close, status bar, resize grip at the bottom right corner, content frame.
class ROOT_AE3_Window : ROOT_AE3_UiListener
{
	protected static const int ID_TITLE = 1;
	protected static const int ID_CLOSE = 2;
	protected static const int ID_MAX = 3;
	protected static const int ID_MIN = 4;
	protected static const int ID_BODY = 5;
	protected static const int ID_GRIP = 6;
	protected static const float TITLE_H = 34;
	protected static const float STATUS_H = 24;
	protected static const float MIN_W = 320;
	protected static const float MIN_H = 220;
	protected static const float FADE_TIME = 0.12;

	ref ROOT_AE3_App m_App;
	ROOT_AE3_DesktopView m_Desktop;
	protected FrameWidget m_wFrame;
	protected FrameWidget m_wContent;
	protected FrameWidget m_wGhost;
	protected TextWidget m_wStatus;
	protected TextWidget m_wTitle;
	protected float m_fX, m_fY, m_fW, m_fH;
	protected float m_fRestoreX, m_fRestoreY, m_fRestoreW, m_fRestoreH;
	protected bool m_bMax;
	protected bool m_bMin;
	protected bool m_bDragging;
	protected bool m_bResizing;
	protected bool m_bDragPending;
	protected float m_fPressX, m_fPressY;
	protected float m_fDragDX, m_fDragDY;
	protected float m_fGhostW, m_fGhostH;
	protected float m_fFade;

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
		m_wFrame.SetOpacity(0);
		app.Build();
	}

	//------------------------------------------------------------------------------------------------
	protected void Create()
	{
		m_wFrame = ROOT_AE3_UI.Frame(m_Desktop.GetWindowLayer(), m_fX, m_fY, m_fW, m_fH, false);
		if (!m_bMax)
			ROOT_AE3_UI.Shadow(m_wFrame, 0, 0, m_fW, m_fH, 10);

		ROOT_AE3_UI.Rect(m_wFrame, 0, 0, m_fW, m_fH, ROOT_AE3_Theme.Border(), ID_BODY, this);
		ROOT_AE3_UI.Rect(m_wFrame, 1, 1, m_fW - 2, m_fH - 2, ROOT_AE3_Theme.Panel());
		ROOT_AE3_UI.Rect(m_wFrame, 1, 1, m_fW - 2, TITLE_H, ROOT_AE3_Theme.Title(), ID_TITLE, this);
		ROOT_AE3_UI.Icon(m_wFrame, 12, 9, 18, m_App.GetIcon());
		m_wTitle = ROOT_AE3_UI.Text(m_wFrame, 40, 9, m_fW - 200, 18, m_App.GetTitle(), 14, ROOT_AE3_Theme.Text());
		string maxIcon = "g_max";
		if (m_bMax)
			maxIcon = "g_restore";

		ROOT_AE3_UI.IconButton(m_wFrame, m_fW - 139, 1, 46, TITLE_H, "g_min", ID_MIN, this, 12);
		ROOT_AE3_UI.IconButton(m_wFrame, m_fW - 93, 1, 46, TITLE_H, maxIcon, ID_MAX, this, 12);
		ROOT_AE3_UI.IconButton(m_wFrame, m_fW - 47, 1, 46, TITLE_H, "g_close", ID_CLOSE, this, 13, null, null, ROOT_AE3_Theme.Danger());
		m_wContent = ROOT_AE3_UI.Frame(m_wFrame, 1, TITLE_H + 1, m_fW - 2, m_fH - TITLE_H - STATUS_H - 2);
		ROOT_AE3_UI.Rect(m_wFrame, 1, m_fH - STATUS_H - 1, m_fW - 2, 1, ROOT_AE3_Theme.Border());
		m_wStatus = ROOT_AE3_UI.Text(m_wFrame, 12, m_fH - STATUS_H + 3, m_fW - 40, 18, "", 13, ROOT_AE3_Theme.Muted());
		if (!m_bMax)
		{
			ROOT_AE3_UI.Rect(m_wFrame, m_fW - 18, m_fH - 18, 17, 17, ROOT_AE3_UI.Rgb(0, 0, 0, 0), ID_GRIP, this);
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j <= i; j++)
				{
					ROOT_AE3_UI.Rect(m_wFrame, m_fW - 6 - j * 4, m_fH - 14 + i * 4, 2, 2, ROOT_AE3_Theme.Muted());
				}
			}
		}
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
	void SetTitle(string text)
	{
		if (m_wTitle)
			m_wTitle.SetText(text);
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
		else
			m_Desktop.SetFocusWindow(null);
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
		if (m_wGhost)
			m_wGhost.RemoveFromHierarchy();

		m_wGhost = null;
		if (m_wFrame)
			m_wFrame.RemoveFromHierarchy();

		Create();
		m_wFrame.SetOpacity(1);
		m_App.Redraw();
		BringToFront();
	}

	//------------------------------------------------------------------------------------------------
	//! Desktop resized: before the desktop rebuild, the widgets are dropped; app state stays.
	void Detach()
	{
		m_bDragging = false;
		m_bDragPending = false;
		m_bResizing = false;
		ReleaseHandlers();
		m_App.ReleaseHandlers();
		if (m_wGhost)
			m_wGhost.RemoveFromHierarchy();

		if (m_wFrame)
			m_wFrame.RemoveFromHierarchy();

		m_wGhost = null;
		m_wFrame = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Desktop resized: rebuilds the window in the new window layer, kept inside the window area.
	void Relayout()
	{
		float aw, ah;
		m_Desktop.GetArea(aw, ah);
		if (m_bMax)
		{
			m_fRestoreW = Math.Min(m_fRestoreW, aw);
			m_fRestoreH = Math.Min(m_fRestoreH, ah);
			m_fRestoreX = Math.Clamp(m_fRestoreX, 0, aw - m_fRestoreW);
			m_fRestoreY = Math.Clamp(m_fRestoreY, 0, ah - m_fRestoreH);
			Resize(0, 0, aw, ah);
		}
		else
		{
			float w = Math.Min(m_fW, aw);
			float h = Math.Min(m_fH, ah);
			Resize(Math.Clamp(m_fX, 0, aw - w), Math.Clamp(m_fY, 0, ah - h), w, h);
		}

		if (m_bMin)
			m_wFrame.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	void Destroy()
	{
		m_App.OnClose();
		m_App.ReleaseHandlers();
		ReleaseHandlers();
		if (m_wGhost)
			m_wGhost.RemoveFromHierarchy();

		if (m_wFrame)
			m_wFrame.RemoveFromHierarchy();
	}

	//------------------------------------------------------------------------------------------------
	void Update(float dt)
	{
		if (m_fFade < 1)
		{
			m_fFade = Math.Min(1, m_fFade + dt / FADE_TIME);
			m_wFrame.SetOpacity(m_fFade);
		}

		if (m_bDragPending)
			UpdatePending();

		if (m_bDragging || m_bResizing)
		{
			float mx, my;
			m_Desktop.MouseLocal(mx, my);
			float aw, ah;
			m_Desktop.GetArea(aw, ah);
			if (m_bDragging)
			{
				m_fX = Math.Clamp(mx - m_fDragDX, 80 - m_fW, aw - 80);
				m_fY = Math.Clamp(my - m_fDragDY, 0, ah - TITLE_H);
				FrameSlot.SetPos(m_wFrame, m_fX, m_fY);
			}
			else
			{
				m_fGhostW = Math.Clamp(mx - m_fX + m_fDragDX, MIN_W, aw - m_fX);
				m_fGhostH = Math.Clamp(my - m_fY + m_fDragDY, MIN_H, ah - m_fY);
				DrawGhost();
			}

			if (GetGame().GetInputManager().GetActionValue("MouseLeft") <= 0)
				EndDrag(mx, my);
		}

		m_App.Update(dt);
	}

	//------------------------------------------------------------------------------------------------
	//! Title of a maximized window pressed: restores and starts dragging once the mouse moves.
	protected void UpdatePending()
	{
		if (GetGame().GetInputManager().GetActionValue("MouseLeft") <= 0)
		{
			m_bDragPending = false;
			return;
		}

		float mx, my;
		m_Desktop.MouseLocal(mx, my);
		if (Math.AbsFloat(mx - m_fPressX) + Math.AbsFloat(my - m_fPressY) < 8)
			return;

		m_bDragPending = false;
		float rel = (mx - m_fX) / m_fW;
		m_bMax = false;
		Resize(mx - m_fRestoreW * rel, 0, m_fRestoreW, m_fRestoreH);
		m_fDragDX = mx - m_fX;
		m_fDragDY = my - m_fY;
		m_bDragging = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Outline of the new size while the grip is dragged.
	protected void DrawGhost()
	{
		if (!m_wGhost)
		{
			m_wGhost = ROOT_AE3_UI.Frame(m_Desktop.GetWindowLayer(), m_fX, m_fY, m_fGhostW, m_fGhostH, false);
			m_wGhost.SetZOrder(m_Desktop.NextZ());
		}

		ROOT_AE3_UI.Clear(m_wGhost);
		FrameSlot.SetSize(m_wGhost, m_fGhostW, m_fGhostH);
		Color c = ROOT_AE3_Theme.Accent();
		ROOT_AE3_UI.Rect(m_wGhost, 0, 0, m_fGhostW, m_fGhostH, ROOT_AE3_UI.Rgb(c.R(), c.G(), c.B(), 0.12));
		ROOT_AE3_UI.Rect(m_wGhost, 0, 0, m_fGhostW, 2, c);
		ROOT_AE3_UI.Rect(m_wGhost, 0, m_fGhostH - 2, m_fGhostW, 2, c);
		ROOT_AE3_UI.Rect(m_wGhost, 0, 0, 2, m_fGhostH, c);
		ROOT_AE3_UI.Rect(m_wGhost, m_fGhostW - 2, 0, 2, m_fGhostH, c);
	}

	//------------------------------------------------------------------------------------------------
	protected void EndDrag(float mx, float my)
	{
		if (m_bResizing)
		{
			m_bResizing = false;
			if (m_wGhost)
			{
				m_wGhost.RemoveFromHierarchy();
				m_wGhost = null;
			}

			Resize(m_fX, m_fY, m_fGhostW, m_fGhostH);
			return;
		}

		m_bDragging = false;
		float aw, ah;
		m_Desktop.GetArea(aw, ah);
		if (my <= 2)
			Snap(0, 0, aw, ah);
		else if (mx <= 2)
			Snap(0, 0, aw * 0.5, ah);
		else if (mx >= aw - 2)
			Snap(aw * 0.5, 0, aw * 0.5, ah);
	}

	//------------------------------------------------------------------------------------------------
	//! Maximized or half screen; the previous size is restored by the maximize button or by dragging.
	protected void Snap(float x, float y, float w, float h)
	{
		if (!m_bMax)
		{
			m_fRestoreX = m_fX;
			m_fRestoreY = m_fY;
			m_fRestoreW = m_fW;
			m_fRestoreH = m_fH;
		}

		m_bMax = true;
		Resize(x, y, w, h);
	}

	//------------------------------------------------------------------------------------------------
	void StopDrag()
	{
		m_bDragging = false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiMouseDown(Widget w, int id, int button)
	{
		BringToFront();
		if (button != 0)
			return false;

		float mx, my;
		m_Desktop.MouseLocal(mx, my);
		if (id == ID_TITLE)
		{
			if (m_bMax)
			{
				m_fPressX = mx;
				m_fPressY = my;
				m_bDragPending = true;
				return true;
			}

			m_fDragDX = mx - m_fX;
			m_fDragDY = my - m_fY;
			m_bDragging = true;
			return true;
		}

		if (id == ID_GRIP)
		{
			m_fDragDX = m_fX + m_fW - mx;
			m_fDragDY = m_fY + m_fH - my;
			m_fGhostW = m_fW;
			m_fGhostH = m_fH;
			m_bResizing = true;
			return true;
		}

		return false;
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
	void ToggleMax()
	{
		m_bDragging = false;
		m_bDragPending = false;
		if (m_bMax)
		{
			m_bMax = false;
			Resize(m_fRestoreX, m_fRestoreY, m_fRestoreW, m_fRestoreH);
			return;
		}

		float sw, sh;
		m_Desktop.GetArea(sw, sh);
		Snap(0, 0, sw, sh);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (button != 0)
			return false;

		switch (id)
		{
			case ID_CLOSE: m_Desktop.CloseWindow(this); return true;
			case ID_MIN: SetMinimized(true); m_Desktop.RefreshTaskbar(); return true;
			case ID_MAX: ToggleMax(); return true;
		}

		return false;
	}
}
