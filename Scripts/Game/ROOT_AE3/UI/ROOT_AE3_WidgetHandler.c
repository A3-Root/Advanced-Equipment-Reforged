//! Forwards widget events to a ROOT_AE3_UiListener with the widget user id.
//! OnClick is raised by the engine for button widgets only; for image widgets (ROOT_AE3_UI.Rect / Button) a press
//! and release inside the widget is reported as a click, a second click within DOUBLE_CLICK_MS as a double click.
//! Buttons (SetButton) highlight on hover and press and play the click sound.
//! OnChange is not forwarded while s_bQuiet is set (ROOT_AE3_UI.SetEdit) or while another OnChange is being handled.
class ROOT_AE3_WidgetHandler : ScriptedWidgetEventHandler
{
	static const int DOUBLE_CLICK_MS = 450;
	static bool s_bQuiet;
	protected static bool s_bInChange;
	protected static int s_iLastClick = -100000;
	protected static int s_iLastClickId = -1;
	protected static ROOT_AE3_UiListener s_LastClickListener;

	ROOT_AE3_UiListener m_Listener;
	protected bool m_bPressed;
	protected bool m_bButton;
	protected ref Color m_Base;

	//------------------------------------------------------------------------------------------------
	//! Marks the widget as a button with the given background color.
	void SetButton(Color base)
	{
		m_bButton = true;
		m_Base = new Color(base.R(), base.G(), base.B(), base.A());
	}

	//------------------------------------------------------------------------------------------------
	protected void Tint(Widget w, float add)
	{
		if (!m_bButton || !w)
			return;

		if (add == 0)
		{
			w.SetColor(m_Base);
			return;
		}

		// Transparent buttons (menu rows) get a white overlay.
		float a = Math.Max(m_Base.A(), 0.14 + add);
		if (m_Base.A() < 0.1)
			w.SetColor(new Color(1, 1, 1, a));
		else
			w.SetColor(new Color(Math.Clamp(m_Base.R() + add, 0, 1), Math.Clamp(m_Base.G() + add, 0, 1), Math.Clamp(m_Base.B() + add, 0, 1), a));
	}

	//------------------------------------------------------------------------------------------------
	protected static bool UnderCursor(Widget w)
	{
		float x, y, sx, sy;
		w.GetScreenPos(x, y);
		w.GetScreenSize(sx, sy);
		int mx, my;
		WidgetManager.GetMousePos(mx, my);
		return mx >= x && my >= y && mx <= x + sx && my <= y + sy;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (m_Listener)
			return m_Listener.OnUiClick(w, w.GetUserID(), button);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		// Image widgets get the double click from OnMouseButtonUp.
		if (w && m_Listener && !ImageWidget.Cast(w))
			return m_Listener.OnUiDoubleClick(w, w.GetUserID());

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		Tint(w, 0.12);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		Tint(w, 0);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (!w)
			return false;

		if (ImageWidget.Cast(w))
			m_bPressed = true;

		Tint(w, 0.25);
		if (m_Listener)
			return m_Listener.OnUiMouseDown(w, w.GetUserID(), button);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonUp(Widget w, int x, int y, int button)
	{
		bool pressed = m_bPressed;
		m_bPressed = false;
		if (!w || !m_Listener)
			return false;

		bool handled = m_Listener.OnUiMouseUp(w, w.GetUserID(), button);
		if (!pressed || !UnderCursor(w))
		{
			Tint(w, 0);
			return handled;
		}

		Tint(w, 0.12);
		if (m_bButton)
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.CLICK);

		int id = w.GetUserID();
		int now = System.GetTickCount();
		// Tracked per listener and id: listeners may rebuild the clicked widget between the two clicks.
		bool doubleClick = button == 0 && s_LastClickListener == m_Listener && s_iLastClickId == id && now - s_iLastClick <= DOUBLE_CLICK_MS;
		if (doubleClick)
		{
			s_LastClickListener = null;
		}
		else
		{
			s_LastClickListener = m_Listener;
			s_iLastClickId = id;
			s_iLastClick = now;
		}

		// The listener may remove this widget; only the saved id is used after these calls.
		ROOT_AE3_UiListener listener = m_Listener;
		handled = listener.OnUiClick(w, id, button) || handled;
		if (doubleClick)
			handled = listener.OnUiDoubleClick(w, id) || handled;

		return handled;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseWheel(Widget w, int x, int y, int wheel)
	{
		if (m_Listener)
			return m_Listener.OnUiWheel(w, w.GetUserID(), wheel);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnChange(Widget w, bool finished)
	{
		if (!w || !m_Listener || s_bQuiet || s_bInChange)
			return false;

		s_bInChange = true;
		bool handled = m_Listener.OnUiChange(w, w.GetUserID(), finished);
		s_bInChange = false;
		return handled;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnLinkClick(Widget w, int x, int y, int button, string link)
	{
		if (m_Listener)
			return m_Listener.OnUiLink(w, w.GetUserID(), link);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnChar(Widget w, int charCode)
	{
		if (m_Listener)
			return m_Listener.OnUiChar(w, w.GetUserID(), charCode);

		return false;
	}
}
