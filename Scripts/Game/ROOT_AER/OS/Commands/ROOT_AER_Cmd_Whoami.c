//! armaOS program 'whoami': Returns the current user.
class ROOT_AER_Cmd_Whoami : ROOT_AER_Command
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
	override void Run(ROOT_AER_ShellContext ctx, array<string> args)
	{
		ctx.Out(ctx.m_sUser);
	}
}
