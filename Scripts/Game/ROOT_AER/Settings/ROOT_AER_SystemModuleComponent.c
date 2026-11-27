[ComponentEditorProps(category: "Advanced Equipment Reforged", description: "AER System module: mission-wide Advanced Equipment Reforged settings")]
class ROOT_AER_SystemModuleComponentClass : ScriptComponentClass
{
}

//! AER System module (Prefabs/ROOT_AER/Systems/ROOT_AER_SystemModule.et). Its settings replace the mission
//! header and Configs/ROOT_AER/Settings.conf for the whole mission. Devices follow them where their own
//! value is "Setting default"; values set on a device in the World Editor or by the Game Master take priority.
//! Game Master: attributes window of the module (global settings keys) and the module's action panel (text
//! settings). The first module placed is active; further modules are inactive and cannot be edited. When the
//! active module is deleted, the next placed module becomes active, else the mission header or defaults apply.
//! Locality: server reads the settings; clients receive the client-relevant values (ROOT_AER_EMsg.SETTINGS).
class ROOT_AER_SystemModuleComponent : ScriptComponent
{
	[Attribute(desc: "Advanced Equipment Reforged settings of the mission. Empty = mission header or defaults at placement.")]
	protected ref ROOT_AER_Settings m_Settings;

	protected static ROOT_AER_SystemModuleComponent s_Active;
	protected static ref array<ROOT_AER_SystemModuleComponent> s_aModules = {};

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (!GetGame().InPlayMode() || !Replication.IsServer())
			return;

		s_aModules.Insert(this);
		if (s_Active)
		{
			Print("[AER] Only one AER System module is active; this one is inactive.", LogLevel.WARNING);
			return;
		}

		ActivateModule();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		GetGame().GetCallqueue().Remove(ROOT_AER_SettingsManager.Broadcast);
		s_aModules.RemoveItem(this);
		if (s_Active == this)
		{
			s_Active = null;
			if (!ROOT_AER_Manager.IsEnding())
			{
				if (!s_aModules.IsEmpty() && s_aModules[0])
				{
					s_aModules[0].ActivateModule();
				}
				else
				{
					ROOT_AER_SettingsManager.Reset();
					GetGame().GetCallqueue().CallLater(ROOT_AER_SettingsManager.Broadcast, 0, false);
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
			m_Settings = ROOT_AER_SettingsManager.Get();

		ROOT_AER_SettingsManager.SetModuleSettings(m_Settings);
		GetGame().GetCallqueue().CallLater(ROOT_AER_SettingsManager.Broadcast, 0, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Settings of the module; set for the active module, null for an inactive module without own settings.
	ROOT_AER_Settings GetSettings()
	{
		return m_Settings;
	}

	//------------------------------------------------------------------------------------------------
	static ROOT_AER_SystemModuleComponent GetActive()
	{
		return s_Active;
	}

	//------------------------------------------------------------------------------------------------
	static ROOT_AER_SystemModuleComponent Of(IEntity ent)
	{
		if (!ent)
			return null;

		return ROOT_AER_SystemModuleComponent.Cast(ent.FindComponent(ROOT_AER_SystemModuleComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! True for the active module. Clients do not know which module is active and accept any module.
	static bool IsActiveModule(IEntity ent)
	{
		ROOT_AER_SystemModuleComponent m = Of(ent);
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
