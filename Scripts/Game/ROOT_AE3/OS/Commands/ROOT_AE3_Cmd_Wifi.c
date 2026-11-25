//! armaOS program 'wifi': Lists, joins and leaves wireless networks.
class ROOT_AE3_Cmd_Wifi : ROOT_AE3_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "wifi";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/sbin/wifi";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Lists, joins and leaves wireless networks.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage wifi: 'wifi scan' lists networks in range, 'wifi connect <ssid|gateway> [password]' joins one, 'wifi disconnect' leaves the current network.";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AE3_ShellContext ctx, array<string> args)
	{
		ROOT_AE3_NetworkComponent n = ctx.m_Exec.GetNetwork();
		if (!n || n.IsRouter())
		{
			ctx.Err("wifi: no wireless interface");
			return;
		}

		string sub = "scan";
		if (!args.IsEmpty())
			sub = args[0];

		ROOT_AE3_NetworkSystem sys = ROOT_AE3_NetworkSystem.Get();
		vector pos = ctx.m_Exec.GetOwner().GetOrigin();
		if (sub == "scan")
		{
			array<ROOT_AE3_NetworkComponent> found = {};
			sys.Scan(pos, found);
			if (found.IsEmpty())
			{
				ctx.Out("No networks in range.");
				return;
			}

			ctx.Out(ROOT_AE3_Util.PadRight("SSID", 20) + ROOT_AE3_Util.PadRight("GATEWAY", 16) + ROOT_AE3_Util.PadRight("DIST", 8) + "SECURITY");
			foreach (ROOT_AE3_NetworkComponent r : found)
			{
				string sec = "open";
				if (!r.GetPassword().IsEmpty())
					sec = "password";

				string dist = Math.Round(vector.Distance(pos, r.GetOwner().GetOrigin())).ToString() + " m";
				ctx.Out(ROOT_AE3_Util.PadRight(r.GetSsid(), 20) + ROOT_AE3_Util.PadRight(r.GetGateway(), 16) + ROOT_AE3_Util.PadRight(dist, 8) + sec);
			}

			return;
		}

		if (sub == "disconnect")
		{
			sys.Disconnect(n);
			ctx.Out("Disconnected.");
			return;
		}

		if (sub != "connect" || args.Count() < 2)
		{
			ctx.Err("Usage: wifi scan | wifi connect <ssid|gateway> [password] | wifi disconnect");
			return;
		}

		array<ROOT_AE3_NetworkComponent> inRange = {};
		sys.Scan(pos, inRange);
		ROOT_AE3_NetworkComponent target;
		foreach (ROOT_AE3_NetworkComponent r : inRange)
		{
			if (r.GetSsid() == args[1] || r.GetGateway() == args[1])
			{
				target = r;
				break;
			}
		}

		if (!target)
		{
			ctx.Err(string.Format("wifi: network '%1' not in range", args[1]));
			return;
		}

		string password;
		if (args.Count() > 2)
			password = args[2];

		if (!target.GetPassword().IsEmpty() && !ctx.m_Session.CheckAuth(target.GetPassword() == password))
		{
			ctx.Err("wifi: wrong password");
			return;
		}

		string reason;
		if (!sys.Connect(n, target, password, reason))
		{
			ctx.Err("wifi: " + reason);
			return;
		}

		ctx.Out(string.Format("Connected to %1 as %2", target.GetSsid(), n.GetAddress()));
	}
}
