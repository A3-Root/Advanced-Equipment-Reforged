//! armaOS program 'standby': Puts the computer in standby mode.
class ROOT_AE3_Cmd_Standby : ROOT_AE3_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "standby";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/sbin/standby";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Puts the computer in standby mode.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage standby: 'standby' activates the computers standby mode.";
	}

	//------------------------------------------------------------------------------------------------
	override bool IsSshCompatible()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AE3_ShellContext ctx, array<string> args)
	{
		ROOT_AE3_PowerComponent p = ctx.m_Exec.GetPower();
		string reason;
		if (!p || !p.Standby(reason))
		{
			if (reason.IsEmpty())
				reason = "not supported";

			ctx.Err("standby: " + reason);
		}
	}
}
