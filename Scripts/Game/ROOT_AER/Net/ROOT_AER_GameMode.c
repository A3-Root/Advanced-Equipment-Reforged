//! Ends Advanced Equipment Reforged sessions of leaving players.
modded class SCR_BaseGameMode
{
	//------------------------------------------------------------------------------------------------
	override void OnPlayerConnected(int playerId)
	{
		super.OnPlayerConnected(playerId);
		if (Replication.IsServer())
			GetGame().GetCallqueue().CallLater(ROOT_AER_SendSettings, 3000, false, playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Client-relevant settings (screen mirror range, viewers) for players who have not opened a computer yet.
	protected void ROOT_AER_SendSettings(int playerId)
	{
		ROOT_AER_Net.SendToPlayer(playerId, ROOT_AER_EMsg.SETTINGS, ROOT_AER_SettingsManager.Get().PackClient());
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		if (Replication.IsServer())
			ROOT_AER_Manager.Get().OnPlayerDisconnected(playerId);

		super.OnPlayerDisconnected(playerId, cause, timeout);
	}
}
