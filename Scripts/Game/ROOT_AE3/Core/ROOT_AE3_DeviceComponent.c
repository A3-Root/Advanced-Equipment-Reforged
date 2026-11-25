[ComponentEditorProps(category: "Advanced Equipment", description: "Advanced Equipment device: type, name and registration")]
class ROOT_AE3_DeviceComponentClass : ScriptComponentClass
{
}

//! Base component of every Advanced Equipment device. Registers the device with ROOT_AE3_Manager.
//! Locality: exists on server and clients; registry and state changes on the authority only.
class ROOT_AE3_DeviceComponent : ScriptComponent
{
	[Attribute("0", uiwidget: UIWidgets.ComboBox, desc: "Device type", enums: ParamEnumArray.FromEnum(ROOT_AE3_EDeviceType), category: "Advanced Equipment")]
	protected ROOT_AE3_EDeviceType m_eType;

	[Attribute("", desc: "Device name; empty = settings fallback", category: "Advanced Equipment")]
	protected string m_sDeviceName;

	protected RplComponent m_RplComponent;

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!GetGame().InPlayMode())
			return;

		ROOT_AE3_Manager.Register(this);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		ROOT_AE3_Manager.Unregister(this);
		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AE3_EDeviceType GetDeviceType()
	{
		return m_eType;
	}

	//------------------------------------------------------------------------------------------------
	bool IsComputer()
	{
		return m_eType == ROOT_AE3_EDeviceType.LAPTOP || m_eType == ROOT_AE3_EDeviceType.SERVER || m_eType == ROOT_AE3_EDeviceType.DESKTOP;
	}

	//------------------------------------------------------------------------------------------------
	string GetDeviceName()
	{
		if (!m_sDeviceName.IsEmpty())
			return m_sDeviceName;

		ROOT_AE3_Settings s = ROOT_AE3_SettingsManager.Get();
		if (s.m_iDeviceNameFallback == 1)
		{
			EntityPrefabData pd = GetOwner().GetPrefabData();
			if (pd)
			{
				string prefab = pd.GetPrefabName();
				if (!prefab.IsEmpty())
					return FilePath.StripExtension(FilePath.StripPath(prefab));
			}
		}

		return GenericName(m_eType);
	}

	//------------------------------------------------------------------------------------------------
	static string GenericName(ROOT_AE3_EDeviceType type)
	{
		switch (type)
		{
			case ROOT_AE3_EDeviceType.LAPTOP: return "Laptop";
			case ROOT_AE3_EDeviceType.SERVER: return "Server";
			case ROOT_AE3_EDeviceType.DESKTOP: return "Computer";
			case ROOT_AE3_EDeviceType.ROUTER: return "Router";
			case ROOT_AE3_EDeviceType.USB_DRIVE: return "Flash Drive";
			case ROOT_AE3_EDeviceType.GENERATOR: return "Generator";
			case ROOT_AE3_EDeviceType.BATTERY: return "Battery";
			case ROOT_AE3_EDeviceType.SOLAR: return "Solar Panel";
			case ROOT_AE3_EDeviceType.LAMP: return "Lamp";
		}

		return "Device";
	}

	//------------------------------------------------------------------------------------------------
	RplId GetRplId()
	{
		if (!m_RplComponent)
			return RplId.Invalid();

		return Replication.FindItemId(m_RplComponent);
	}

	//------------------------------------------------------------------------------------------------
	bool IsAuthority()
	{
		return !m_RplComponent || !m_RplComponent.IsProxy();
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: plays a device sound for nearby players.
	void BroadcastSound(int soundId)
	{
		float now = GetGame().GetWorld().GetWorldTime() / 1000.0;
		if (now - m_fLastSound < 0.5)
			return;

		m_fLastSound = now;
		Rpc(RpcDo_Sound, soundId);
		RpcDo_Sound(soundId);
	}

	protected float m_fLastSound = -10;

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	protected void RpcDo_Sound(int soundId)
	{
		if (System.IsConsoleApp())
			return;

		IEntity me = SCR_PlayerController.GetLocalControlledEntity();
		if (!me || vector.Distance(me.GetOrigin(), GetOwner().GetOrigin()) > ROOT_AE3_Sound.RANGE)
			return;

		ROOT_AE3_Sound.PlayAt(GetOwner(), soundId);
	}

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_DeviceComponent FromRplId(RplId id)
	{
		RplComponent rpl = RplComponent.Cast(Replication.FindItem(id));
		if (!rpl)
			return null;

		return ROOT_AE3_DeviceComponent.Cast(rpl.GetEntity().FindComponent(ROOT_AE3_DeviceComponent));
	}

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_DeviceComponent Of(IEntity ent)
	{
		if (!ent)
			return null;

		return ROOT_AE3_DeviceComponent.Cast(ent.FindComponent(ROOT_AE3_DeviceComponent));
	}
}
