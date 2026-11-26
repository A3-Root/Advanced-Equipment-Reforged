//! Receiver of widget events created by ROOT_AE3_UI. Keeps its widget handlers alive.
class ROOT_AE3_UiListener
{
	protected ref array<ref ROOT_AE3_WidgetHandler> m_aHandlers = {};

	//------------------------------------------------------------------------------------------------
	void KeepHandler(ROOT_AE3_WidgetHandler h)
	{
		m_aHandlers.Insert(h);
	}

	//------------------------------------------------------------------------------------------------
	void ReleaseHandlers()
	{
		m_aHandlers.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Listener that finally receives the events; double clicks are matched against it.
	ROOT_AE3_UiListener ClickTarget()
	{
		return this;
	}

	//------------------------------------------------------------------------------------------------
	//! button: 0 left, 1 right.
	bool OnUiClick(Widget w, int id, int button)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool OnUiDoubleClick(Widget w, int id)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool OnUiMouseDown(Widget w, int id, int button)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool OnUiMouseUp(Widget w, int id, int button)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool OnUiWheel(Widget w, int id, int wheel)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! finished = Enter pressed in an edit box.
	bool OnUiChange(Widget w, int id, bool finished)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool OnUiLink(Widget w, int id, string link)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool OnUiChar(Widget w, int id, int charCode)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Context menu entry chosen (ROOT_AE3_DesktopView.ShowMenu).
	void OnMenuItem(int id)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Dialog closed (ROOT_AE3_DesktopView.ShowDialog); ok is false for Cancel.
	void OnDialog(string tag, bool ok, string value)
	{
	}
}
