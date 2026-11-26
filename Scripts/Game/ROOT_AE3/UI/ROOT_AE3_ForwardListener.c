//! Forwards widget events to another listener; lets a widget group drop its handlers without touching the owner's.
class ROOT_AE3_ForwardListener : ROOT_AE3_UiListener
{
	ROOT_AE3_UiListener m_Target;

	//------------------------------------------------------------------------------------------------
	void ROOT_AE3_ForwardListener(ROOT_AE3_UiListener target)
	{
		m_Target = target;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		return m_Target && m_Target.OnUiClick(w, id, button);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiDoubleClick(Widget w, int id)
	{
		return m_Target && m_Target.OnUiDoubleClick(w, id);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiMouseDown(Widget w, int id, int button)
	{
		return m_Target && m_Target.OnUiMouseDown(w, id, button);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiMouseUp(Widget w, int id, int button)
	{
		return m_Target && m_Target.OnUiMouseUp(w, id, button);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiWheel(Widget w, int id, int wheel)
	{
		return m_Target && m_Target.OnUiWheel(w, id, wheel);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiChange(Widget w, int id, bool finished)
	{
		return m_Target && m_Target.OnUiChange(w, id, finished);
	}
}
