//! Switches the session to the terminal.
class ROOT_AE3_AppTerminal : ROOT_AE3_App
{
	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Terminal";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "terminal";
	}

	//------------------------------------------------------------------------------------------------
	//! Opens no window.
	override bool Launch()
	{
		ROOT_AE3_Client.Get().Send(ROOT_AE3_EMsg.TERM_KEY, "MODE:0");
		return true;
	}
}
