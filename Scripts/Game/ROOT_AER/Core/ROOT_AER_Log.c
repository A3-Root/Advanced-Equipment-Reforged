//! Log output gated by the debug settings.
class ROOT_AER_Log
{
	//------------------------------------------------------------------------------------------------
	static void Debug(string msg)
	{
		if (ROOT_AER_SettingsManager.Get().m_bDebugMode)
			Print("[AER] " + msg, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	static void Net(string msg)
	{
		if (ROOT_AER_SettingsManager.Get().m_bNetworkDebug)
			Print("[AER net] " + msg, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	static void Warn(string msg)
	{
		Print("[AER] " + msg, LogLevel.WARNING);
	}
}
