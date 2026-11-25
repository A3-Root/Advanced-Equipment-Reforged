//! armaOS program 'history': Prints the terminal history.
class ROOT_AE3_Cmd_History : ROOT_AE3_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "history";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/history";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Prints the terminal history.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage history: 'history' lists last commands since the start of the computer.";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AE3_ShellContext ctx, array<string> args)
	{
		array<string> h = ctx.m_Session.m_Computer.GetHistory(ctx.m_Term.CurrentUser());
		foreach (int i, string l : h)
		{
			ctx.Out(ROOT_AE3_Util.PadLeft((i + 1).ToString(), 4) + "  " + l);
		}
	}
}
