//! armaOS program 'whoami': Returns the current user.
class ROOT_AE3_Cmd_Whoami : ROOT_AE3_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "whoami";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/whoami";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Returns the current user.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage whoami: 'whoami' returns the current user.";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AE3_ShellContext ctx, array<string> args)
	{
		ctx.Out(ctx.m_sUser);
	}
}
