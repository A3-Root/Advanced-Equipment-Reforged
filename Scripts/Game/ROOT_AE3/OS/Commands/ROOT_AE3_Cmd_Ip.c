//! armaOS program 'ip': Returns the current ip configuration.
class ROOT_AE3_Cmd_Ip : ROOT_AE3_Command
{
	protected string m_sNameOverride;

	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return m_sNameOverride;
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/sbin/" + m_sNameOverride;
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Returns the current ip configuration.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage ip: 'ip' or 'ifconfig' returns the current ip configuration, 'ip r' additionally shows the gateway.";
	}

	//------------------------------------------------------------------------------------------------
	void ROOT_AE3_Cmd_Ip(string name = "ip")
	{
		m_sNameOverride = name;
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AE3_ShellContext ctx, array<string> args)
	{
		ROOT_AE3_NetworkComponent n = ctx.m_Exec.GetNetwork();
		if (!n)
		{
			ctx.Out("No network interface.");
			return;
		}

		string state = "DOWN";
		if (!n.GetAddress().IsEmpty())
			state = "UP";

		string ip = n.GetAddress();
		if (ip.IsEmpty())
			ip = "-";

		ctx.Out(string.Format("wlan0: state %1", state));
		ctx.Out(string.Format("    inet %1/24", ip));
		if (n.GetParent())
			ctx.Out(string.Format("    ssid %1", n.GetParent().GetSsid()));

		if (!args.IsEmpty() && args[0] == "r")
		{
			string gw = n.GetGateway();
			if (gw.IsEmpty())
				ctx.Out("No default route.");
			else
				ctx.Out(string.Format("default via %1 dev wlan0", gw));
		}
	}
}
