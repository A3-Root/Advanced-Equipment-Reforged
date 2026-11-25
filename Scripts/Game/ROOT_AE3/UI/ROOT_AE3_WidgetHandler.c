//! Forwards widget events to a ROOT_AE3_UiListener with the widget user id.
class ROOT_AE3_WidgetHandler : ScriptedWidgetEventHandler
{
	ROOT_AE3_UiListener m_Listener;

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
		if (m_Listener)
			return m_Listener.OnUiDoubleClick(w, w.GetUserID());

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (m_Listener)
			return m_Listener.OnUiMouseDown(w, w.GetUserID(), button);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonUp(Widget w, int x, int y, int button)
	{
		if (m_Listener)
			return m_Listener.OnUiMouseUp(w, w.GetUserID(), button);

		return false;
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
		if (m_Listener)
			return m_Listener.OnUiChange(w, w.GetUserID(), finished);

		return false;
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
