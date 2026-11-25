//! Access to the active settings. Server: mission header or defaults, cached at first use.
//! Client: values received with the last session (see ROOT_AE3_Settings.PackClient).
class ROOT_AE3_SettingsManager
{
	static const ResourceName DEFAULTS = "{AE1EA802C914CA29}Configs/ROOT_AE3/Settings.conf";
	protected static ref ROOT_AE3_Settings s_Settings;

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_Settings Get()
	{
		if (s_Settings)
			return s_Settings;

		SCR_MissionHeader header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
		if (header && header.m_ROOT_AE3_Settings)
			s_Settings = header.m_ROOT_AE3_Settings;
		else
			s_Settings = SCR_ConfigHelperT<ROOT_AE3_Settings>.GetConfigObject(DEFAULTS);

		if (!s_Settings)
			s_Settings = new ROOT_AE3_Settings();

		return s_Settings;
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
			ROOT_AE3_Net.SendToPlayer(playerId, ROOT_AE3_EMsg.SETTINGS, packed);
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
