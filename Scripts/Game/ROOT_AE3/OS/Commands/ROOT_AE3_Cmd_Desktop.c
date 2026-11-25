//! armaOS program 'desktop': Switch from the terminal to the GUI desktop interface.
class ROOT_AE3_Cmd_Desktop : ROOT_AE3_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "desktop";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/desktop";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Switch from the terminal to the GUI desktop interface.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage: desktop\n\nCloses the terminal and opens the graphical desktop on this device. Requires GUI interface access.";
	}

	//------------------------------------------------------------------------------------------------
	override bool IsSshCompatible()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AE3_ShellContext ctx, array<string> args)
	{
		ctx.m_Session.SwitchMode(1);
	}
}
