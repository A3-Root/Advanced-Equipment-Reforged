//! Forwards clicks to another listener; lets a widget group drop its handlers without touching the owner's.
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
	override bool OnUiMouseUp(Widget w, int id, int button)
	{
		return m_Target && m_Target.OnUiMouseUp(w, id, button);
	}
}
