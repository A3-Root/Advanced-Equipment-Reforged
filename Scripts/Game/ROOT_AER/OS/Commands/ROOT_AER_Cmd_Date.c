//! armaOS program 'date': Prints the date.
class ROOT_AER_Cmd_Date : ROOT_AER_Command
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
	override void Run(ROOT_AER_ShellContext ctx, array<string> args)
	{
		ctx.Out(ROOT_AER_Util.Now());
	}
}
