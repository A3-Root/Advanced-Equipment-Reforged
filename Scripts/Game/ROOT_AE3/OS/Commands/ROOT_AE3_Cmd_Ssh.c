//! armaOS program 'ssh': Open a remote shell session on another computer.
class ROOT_AE3_Cmd_Ssh : ROOT_AE3_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "ssh";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/sbin/ssh";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Open a remote shell session on another computer.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage ssh: 'ssh USER@IP PASSWORD' connects to the remote computer. Entered commands run on the remote machine; 'exit' ends the session. Interactive programs are blocked.";
	}

	//------------------------------------------------------------------------------------------------
	override bool IsSshCompatible()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AE3_ShellContext ctx, array<string> args)
	{
		if (args.Count() != 2 || !args[0].Contains("@"))
		{
			Usage(ctx);
			return;
		}

		int at = args[0].IndexOf("@");
		string user = args[0].Substring(0, at);
		string ip = args[0].Substring(at + 1, args[0].Length() - at - 1);
		ROOT_AE3_NetworkComponent self = ctx.m_Exec.GetNetwork();
		if (!self || self.GetAddress().IsEmpty())
		{
			ctx.Err("ssh: network is unreachable");
			return;
		}

		ROOT_AE3_NetworkComponent targetNet = ROOT_AE3_NetworkSystem.Get().Resolve(ip);
		ROOT_AE3_ComputerComponent target;
		if (targetNet)
			target = ROOT_AE3_ComputerComponent.Of(targetNet.GetOwner());

		if (!target || target == ctx.m_Exec)
		{
			ctx.Err(string.Format("ssh: connect to host %1: No route to host", ip));
			return;
		}

		string reason;
		if (!ROOT_AE3_NetworkSystem.Get().SshAllowed(self, targetNet, reason) || !target.IsRunning())
		{
			if (reason.IsEmpty())
				reason = "Host is down.";

			ctx.Err(string.Format("ssh: connect to host %1: %2", ip, reason));
			return;
		}

		if (user == "root" && !target.AllowsRootLogin())
		{
			ctx.Err("ssh: root login is disabled on the remote host");
			return;
		}

		if (!ctx.m_Session.CheckAuth(target.HasUser(user) && target.CheckPassword(user, args[1])))
		{
			ctx.Err("ssh: permission denied");
			ROOT_AE3_Shell.WriteLog(target, "/var/log/auth.log", string.Format("ssh: failed login for '%1' from %2", user, self.GetAddress()));
			return;
		}

		ROOT_AE3_TerminalState t = ctx.m_Term;
		t.m_SshTarget = target;
		t.m_sSshUser = user;
		string home = ROOT_AE3_Filesystem.HomeOf(user);
		if (!target.GetFilesystem().Exists(home))
			home = "/";

		t.m_sSshCwd = home;
		ROOT_AE3_Shell.WriteLog(target, "/var/log/auth.log", string.Format("ssh: user '%1' logged in from %2", user, self.GetAddress()));
		ctx.Out(string.Format("Connected to %1 (%2) as %3. Type 'exit' to close the connection.", target.GetHostname(), ip, user));
	}
}
