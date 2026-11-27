//! Switches the session to the terminal.
class ROOT_AER_AppTerminal : ROOT_AER_App
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
		ROOT_AER_Client.Get().Send(ROOT_AER_EMsg.TERM_KEY, "MODE:0");
		return true;
	}
}
