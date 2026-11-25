//! armaOS program 'date': Prints the date.
class ROOT_AE3_Cmd_Date : ROOT_AE3_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "date";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/date";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Prints the date.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage date: 'date' prints the actual date in format YYYY-MM-DD HH:MM:SS";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AE3_ShellContext ctx, array<string> args)
	{
		ctx.Out(ROOT_AE3_Util.Now());
	}
}
