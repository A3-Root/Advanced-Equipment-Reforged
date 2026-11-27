//! Access to the active settings. Server: AER System module, else mission header, else defaults; cached at first use.
//! Client: values received with the last session (see ROOT_AER_Settings.PackClient).
class ROOT_AER_SettingsManager
{
	static const ResourceName DEFAULTS = "{AE1EA802C914CA29}Configs/ROOT_AER/Settings.conf";
	protected static ref ROOT_AER_Settings s_Settings;

	//------------------------------------------------------------------------------------------------
	static ROOT_AER_Settings Get()
	{
		if (s_Settings)
			return s_Settings;

		SCR_MissionHeader header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
		if (header && header.m_ROOT_AER_Settings)
			s_Settings = header.m_ROOT_AER_Settings;
		else
			s_Settings = SCR_ConfigHelperT<ROOT_AER_Settings>.GetConfigObject(DEFAULTS);

		if (!s_Settings)
			s_Settings = new ROOT_AER_Settings();

		return s_Settings;
	}

	//------------------------------------------------------------------------------------------------
	//! Server: settings of the AER System module (ROOT_AER_SystemModuleComponent).
	static void SetModuleSettings(ROOT_AER_Settings settings)
	{
		if (settings)
			s_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	//! Server: sends the client-relevant values to every player (after a change at runtime).
	static void Broadcast()
	{
		array<int> players = {};
		GetGame().GetPlayerManager().GetPlayers(players);
		string packed = Get().PackClient();
		foreach (int playerId : players)
		{
			ROOT_AER_Net.SendToPlayer(playerId, ROOT_AER_EMsg.SETTINGS, packed);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Drops all state (game end).
	static void Reset()
	{
		s_Settings = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Client side: applies values sent by the server.
	static void ApplyClient(string packed)
	{
		Get().UnpackClient(packed);
	}
}
