//! armaOS program 'ping': Pings the given address.
class ROOT_AE3_Cmd_Ping : ROOT_AE3_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "ping";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/sbin/ping";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Pings the given address.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage ping: 'ping [address]' pings the given address.";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AE3_ShellContext ctx, array<string> args)
	{
		if (args.Count() != 1)
		{
			Usage(ctx);
			return;
		}

		string ip = args[0];
		int a, b, c, d;
		if (!ROOT_AE3_Util.ParseIp(ip, a, b, c, d))
		{
			ctx.Err(string.Format("ping: %1: invalid address", ip));
			return;
		}

		ROOT_AE3_NetworkComponent self = ctx.m_Exec.GetNetwork();
		if (!self || self.GetAddress().IsEmpty())
		{
			ctx.Err("ping: network is unreachable");
			return;
		}

		ROOT_AE3_NetworkComponent target = ROOT_AE3_NetworkSystem.Get().Resolve(ip);
		int hops;
		ctx.Out(string.Format("PING %1 from %2", ip, self.GetAddress()));
		if (!target || !ROOT_AE3_NetworkSystem.Get().CanReach(self, target, hops))
		{
			ctx.Err(string.Format("From %1: Destination Host Unreachable", self.GetAddress()));
			return;
		}

		ROOT_AE3_PingJob job = new ROOT_AE3_PingJob();
		job.m_Ctx = ctx;
		job.m_sIp = ip;
		job.m_iLeft = 4;
		job.m_iHops = hops;
		ctx.Hold();
		job.Schedule(1000);
	}
}
