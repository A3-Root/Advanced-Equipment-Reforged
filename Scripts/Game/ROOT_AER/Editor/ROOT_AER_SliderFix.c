//! SCR_EventHandlerComponent.OnChange invokes its untyped invoker with the widget only; OnValueChanged takes
//! (Widget, bool) and logs "Incompatible parameter" on every slider change. The slider listens through a
//! one-argument handler instead.
modded class SCR_SliderComponent
{
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		if (!m_Handler)
			return;

		m_Handler.GetOnChange().Remove(OnValueChanged);
		m_Handler.GetOnChange().Insert(ROOT_AER_OnChange);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_Handler)
			m_Handler.GetOnChange().Remove(ROOT_AER_OnChange);

		super.HandlerDeattached(w);
	}

	//------------------------------------------------------------------------------------------------
	protected void ROOT_AER_OnChange(Widget w)
	{
		OnValueChanged(w, true);
	}
}
