//! armaOS program. Installed as a file whose content is "#!ae3 <name>" (for example /bin/ls).
//! Removing the file removes the command from that computer.
class ROOT_AE3_Command
{
	//------------------------------------------------------------------------------------------------
	string GetName()
	{
		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Install path, for example "/bin/ls".
	string GetPath()
	{
		return "/bin/" + GetName();
	}

	//------------------------------------------------------------------------------------------------
	string GetDescription()
	{
		return "";
	}

	//------------------------------------------------------------------------------------------------
	string GetManual()
	{
		return GetDescription();
	}

	//------------------------------------------------------------------------------------------------
	//! False: blocked inside SSH sessions (interactive and graphical programs).
	bool IsSshCompatible()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void Run(ROOT_AE3_ShellContext ctx, array<string> args)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Prints the manual and marks the context as failed.
	protected void Usage(ROOT_AE3_ShellContext ctx)
	{
		ctx.Err(GetManual());
	}
}
