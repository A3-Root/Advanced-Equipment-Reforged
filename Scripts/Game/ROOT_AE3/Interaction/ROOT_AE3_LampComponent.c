[ComponentEditorProps(category: "Advanced Equipment", description: "Lights the lamp while its power component is on")]
class ROOT_AE3_LampComponentClass : ScriptComponentClass
{
}

//! Couples SCR_BaseInteractiveLightComponent to ROOT_AE3_PowerComponent. Runs on every machine with a
//! display (power state is replicated), so each machine lights its own light entities. Dedicated servers
//! create no lights.
class ROOT_AE3_LampComponent : ScriptComponent
{
	protected ROOT_AE3_PowerComponent m_Power;
	protected SCR_BaseInteractiveLightComponent m_Light;

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (!GetGame().InPlayMode() || System.IsConsoleApp())
			return;

		m_Light = SCR_BaseInteractiveLightComponent.Cast(owner.FindComponent(SCR_BaseInteractiveLightComponent));
		m_Power = ROOT_AE3_PowerComponent.Of(owner);
		if (!m_Light || !m_Power)
			return;

		m_Power.GetOnStateChanged().Insert(OnPower);
		GetGame().GetCallqueue().CallLater(Sync, 500, false);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		GetGame().GetCallqueue().Remove(Sync);
		if (m_Power)
			m_Power.GetOnStateChanged().Remove(OnPower);

		m_Power = null;
		m_Light = null;
		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	protected void Sync()
	{
		if (m_Power)
			Apply(m_Power.GetState());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPower(ROOT_AE3_PowerComponent power, int oldState, int newState)
	{
		Apply(newState);
	}

	//------------------------------------------------------------------------------------------------
	protected void Apply(int state)
	{
		IEntity owner = GetOwner();
		if (!m_Light || !owner || owner.IsDeleted())
			return;

		m_Light.ToggleLight(state == ROOT_AE3_EPowerState.ON);
	}
}
