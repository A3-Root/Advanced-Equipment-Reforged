//! armaOS program 'su': Switch to another account.
class ROOT_AER_Cmd_Su : ROOT_AER_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "su";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/su";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Switch to another account.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage su: 'su [USER]' switches the session to USER, or to root when no account is given. The current user must be root or listed in /etc/sudoers. Use exit to return to the previous account.";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AER_ShellContext ctx, array<string> args)
	{
		string current = ctx.m_sUser;
		if (!ctx.m_Exec.IsSudoer(current))
		{
			ctx.Err(string.Format("su: permission denied for user '%1'", current));
			ctx.Log("/var/log/auth.log", string.Format("su: denied for user '%1'", current));
			return;
		}

		string target = "root";
		if (!args.IsEmpty() && !args[0].Trim().IsEmpty())
			target = args[0].Trim();

		if (target == current)
			return;

		if (!ctx.m_Exec.HasUser(target))
		{
			ctx.Err(string.Format("su: user '%1' does not exist", target));
			return;
		}

		ctx.Log("/var/log/auth.log", string.Format("su: user '%1' switched to '%2'", current, target));
		string home = ROOT_AER_Filesystem.HomeOf(target);
		if (!ctx.Fs().Exists(home))
			home = "/";

		ROOT_AER_TerminalState t = ctx.m_Term;
		if (t.InSsh())
		{
			t.m_aSshSu.Insert(t.m_sSshUser);
			t.m_aSshSuCwd.Insert(t.m_sSshCwd);
			t.m_sSshUser = target;
			t.m_sSshCwd = home;
			return;
		}

		t.m_aSuUsers.Insert(target);
		t.m_aSuCwds.Insert(t.m_sCwd);
		t.m_sCwd = home;
	}
}
