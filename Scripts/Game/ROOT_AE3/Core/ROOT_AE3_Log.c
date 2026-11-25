//! Log output gated by the debug settings.
class ROOT_AE3_Log
{
	//------------------------------------------------------------------------------------------------
	static void Debug(string msg)
	{
		if (ROOT_AE3_SettingsManager.Get().m_bDebugMode)
			Print("[AE3] " + msg, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	static void Net(string msg)
	{
		if (ROOT_AE3_SettingsManager.Get().m_bNetworkDebug)
			Print("[AE3 net] " + msg, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	static void Warn(string msg)
	{
		Print("[AE3] " + msg, LogLevel.WARNING);
	}
}
