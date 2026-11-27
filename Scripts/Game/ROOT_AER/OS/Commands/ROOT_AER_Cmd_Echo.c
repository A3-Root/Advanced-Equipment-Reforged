//! armaOS program 'echo': Print/output a line of text to stdout.
class ROOT_AER_Cmd_Echo : ROOT_AER_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "echo";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/echo";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Print/output a line of text to stdout.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage echo: 'echo [text]' prints the given text to stdout. 'echo text > file' writes it to a file, '>>' appends.";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AER_ShellContext ctx, array<string> args)
	{
		ctx.Out(ROOT_AER_Util.JoinFrom(args, 0));
	}
}
