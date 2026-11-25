[ComponentEditorProps(category: "Advanced Equipment", description: "Lights the lamp while its power component is on")]
class ROOT_AE3_LampComponentClass : ScriptComponentClass
{
}

//! Couples SCR_BaseInteractiveLightComponent to ROOT_AE3_PowerComponent. Runs on every machine
//! (power state is replicated), so each machine lights its own light entities.
class ROOT_AE3_LampComponent : ScriptComponent
{
	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (!GetGame().InPlayMode())
			return;

		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(owner);
		if (p)
			p.GetOnStateChanged().Insert(OnPower);

		GetGame().GetCallqueue().CallLater(Sync, 500, false);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		GetGame().GetCallqueue().Remove(Sync);
		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	protected void Sync()
	{
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(GetOwner());
		if (p)
			OnPower(p, 0, p.GetState());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPower(ROOT_AE3_PowerComponent power, int oldState, int newState)
	{
		SCR_BaseInteractiveLightComponent light = SCR_BaseInteractiveLightComponent.Cast(GetOwner().FindComponent(SCR_BaseInteractiveLightComponent));
		if (light)
			light.ToggleLight(newState == ROOT_AE3_EPowerState.ON);
	}
}
