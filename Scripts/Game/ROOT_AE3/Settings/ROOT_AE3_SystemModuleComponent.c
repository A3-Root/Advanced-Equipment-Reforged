[ComponentEditorProps(category: "Advanced Equipment", description: "AE3 System module: mission-wide Advanced Equipment settings")]
class ROOT_AE3_SystemModuleComponentClass : ScriptComponentClass
{
}

//! AE3 System module (Prefabs/ROOT_AE3/Systems/ROOT_AE3_SystemModule.et). Its settings replace the mission
//! header and Configs/ROOT_AE3/Settings.conf for the whole mission. Devices follow them where their own
//! value is "Setting default"; values set on a device in the World Editor or by the Game Master take priority.
//! Game Master: attributes window of the module (global settings keys) and the module's action panel (text
//! settings). The first module placed is active; further modules are inactive and cannot be edited. When the
//! active module is deleted, the next placed module becomes active, else the mission header or defaults apply.
//! Locality: server reads the settings; clients receive the client-relevant values (ROOT_AE3_EMsg.SETTINGS).
class ROOT_AE3_SystemModuleComponent : ScriptComponent
{
	[Attribute(desc: "Advanced Equipment settings of the mission. Empty = mission header or defaults at placement.")]
	protected ref ROOT_AE3_Settings m_Settings;

	protected static ROOT_AE3_SystemModuleComponent s_Active;
	protected static ref array<ROOT_AE3_SystemModuleComponent> s_aModules = {};

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (!GetGame().InPlayMode() || !Replication.IsServer())
			return;

		s_aModules.Insert(this);
		if (s_Active)
		{
			Print("[AE3] Only one AE3 System module is active; this one is inactive.", LogLevel.WARNING);
			return;
		}

		ActivateModule();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		GetGame().GetCallqueue().Remove(ROOT_AE3_SettingsManager.Broadcast);
		s_aModules.RemoveItem(this);
		if (s_Active == this)
		{
			s_Active = null;
			if (!ROOT_AE3_Manager.IsEnding())
			{
				if (!s_aModules.IsEmpty() && s_aModules[0])
				{
					s_aModules[0].ActivateModule();
				}
				else
				{
					ROOT_AE3_SettingsManager.Reset();
					GetGame().GetCallqueue().CallLater(ROOT_AE3_SettingsManager.Broadcast, 0, false);
				}
			}
		}

		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! Server: makes this module the settings source. Applied at once so systems started later read it.
	protected void ActivateModule()
	{
		s_Active = this;
		if (!m_Settings)
			m_Settings = ROOT_AE3_SettingsManager.Get();

		ROOT_AE3_SettingsManager.SetModuleSettings(m_Settings);
		GetGame().GetCallqueue().CallLater(ROOT_AE3_SettingsManager.Broadcast, 0, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Settings of the module; set for the active module, null for an inactive module without own settings.
	ROOT_AE3_Settings GetSettings()
	{
		return m_Settings;
	}

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_SystemModuleComponent GetActive()
	{
		return s_Active;
	}

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_SystemModuleComponent Of(IEntity ent)
	{
		if (!ent)
			return null;

		return ROOT_AE3_SystemModuleComponent.Cast(ent.FindComponent(ROOT_AE3_SystemModuleComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! True for the active module. Clients do not know which module is active and accept any module.
	static bool IsActiveModule(IEntity ent)
	{
		ROOT_AE3_SystemModuleComponent m = Of(ent);
		if (!m)
			return false;

		return !Replication.IsServer() || m == s_Active;
	}

	//------------------------------------------------------------------------------------------------
	//! Drops all state (game end).
	static void Reset()
	{
		s_Active = null;
		s_aModules.Clear();
	}
}
