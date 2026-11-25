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
	override vector GetSize()
	{
		return Vector(300, 120, 0);
	}

	//------------------------------------------------------------------------------------------------
	override void Build()
	{
		Label(10, 10, Width() - 20, 22, "Switching to the terminal...");
		ROOT_AE3_Client.Get().Send(ROOT_AE3_EMsg.TERM_KEY, "MODE:0");
	}

	//------------------------------------------------------------------------------------------------
	void Refresh()
	{
	}
}
