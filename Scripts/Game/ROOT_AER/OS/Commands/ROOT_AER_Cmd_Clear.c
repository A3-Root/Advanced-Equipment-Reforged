//! armaOS program 'clear': Clears the terminal window.
class ROOT_AER_Cmd_Clear : ROOT_AER_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "clear";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/clear";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Clears the terminal window.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage clear: 'clear' deletes most of the displayed text.";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AER_ShellContext ctx, array<string> args)
	{
		ctx.m_Session.Clear();
	}
}
