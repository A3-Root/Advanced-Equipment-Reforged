//! armaOS program 'exit': Log out of the user session.
class ROOT_AE3_Cmd_Exit : ROOT_AE3_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "exit";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/sbin/exit";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Log out of the user session.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage exit: 'exit' returns to the previous account after su, or brings you back to login screen.";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AE3_ShellContext ctx, array<string> args)
	{
		ROOT_AE3_TerminalState t = ctx.m_Term;
		if (!t.m_aSuUsers.IsEmpty())
		{
			int last = t.m_aSuUsers.Count() - 1;
			t.m_sCwd = t.m_aSuCwds[last];
			t.m_aSuUsers.Remove(last);
			t.m_aSuCwds.Remove(last);
			return;
		}

		ROOT_AE3_Shell.Logout(ctx.m_Session);
	}
}
