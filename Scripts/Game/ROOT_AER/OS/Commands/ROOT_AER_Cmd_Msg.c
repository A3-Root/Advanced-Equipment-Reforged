//! armaOS program 'msg': Send an instant message to another computer.
class ROOT_AER_Cmd_Msg : ROOT_AER_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "msg";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/sbin/msg";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Send an instant message to another computer.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage msg: 'msg TARGET-IP MESSAGE...' sends a message over the network. Received messages are stored in /var/mail/inbox (read with cat).";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AER_ShellContext ctx, array<string> args)
	{
		if (args.Count() < 2)
		{
			Usage(ctx);
			return;
		}

		ROOT_AER_NetworkComponent self = ctx.m_Exec.GetNetwork();
		ROOT_AER_NetworkComponent targetNet = ROOT_AER_NetworkSystem.Get().Resolve(args[0]);
		int hops;
		if (!self || !targetNet || !ROOT_AER_NetworkSystem.Get().CanReach(self, targetNet, hops))
		{
			ctx.Err(string.Format("msg: %1: host unreachable", args[0]));
			return;
		}

		ROOT_AER_ComputerComponent target = ROOT_AER_ComputerComponent.Of(targetNet.GetOwner());
		if (!target || !target.IsRunning())
		{
			ctx.Err(string.Format("msg: %1: host is down", args[0]));
			return;
		}

		string text = ROOT_AER_Util.JoinFrom(args, 1);
		ROOT_AER_Intel.DeliverInstantMessage(target, self.GetAddress(), ctx.m_sUser, text);
		ctx.Out(string.Format("Message sent to %1.", args[0]));
	}
}
