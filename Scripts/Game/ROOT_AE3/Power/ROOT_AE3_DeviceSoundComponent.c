[ComponentEditorProps(category: "Advanced Equipment", description: "Plays a looping sound event of the entity SoundComponent while the device is on")]
class ROOT_AE3_DeviceSoundComponentClass : ScriptComponentClass
{
}

//! Positional running sound (generator engine, computer hum). Clients only; follows the replicated power state.
class ROOT_AE3_DeviceSoundComponent : ScriptComponent
{
	[Attribute("SOUND_GENERATOR_LP", desc: "Looping event of the SoundComponent played while on")]
	protected string m_sLoopEvent;

	protected AudioHandle m_Loop = AudioHandle.Invalid;

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (!GetGame().InPlayMode() || System.IsConsoleApp())
			return;

		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(owner);
		if (p)
			p.GetOnStateChanged().Insert(OnPower);

		GetGame().GetCallqueue().CallLater(Sync, 1000, false);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		GetGame().GetCallqueue().Remove(Sync);
		Stop();
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
		if (newState == ROOT_AE3_EPowerState.ON)
			Play();
		else
			Stop();
	}

	//------------------------------------------------------------------------------------------------
	protected void Play()
	{
		if (m_Loop != AudioHandle.Invalid)
			return;

		SoundComponent snd = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (snd)
			m_Loop = snd.SoundEvent(m_sLoopEvent);
	}

	//------------------------------------------------------------------------------------------------
	protected void Stop()
	{
		if (m_Loop == AudioHandle.Invalid)
			return;

		SoundComponent snd = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (snd)
			snd.Terminate(m_Loop);

		m_Loop = AudioHandle.Invalid;
	}
}
