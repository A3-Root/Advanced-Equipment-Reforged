//! armaOS program 'snake': Retro Snake Game
class ROOT_AER_Cmd_Snake : ROOT_AER_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "snake";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/games/snake";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Retro Snake Game";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Retro Snake Game - use option --big for doubled block size. Arrow keys or WASD steer, Esc quits.";
	}

	//------------------------------------------------------------------------------------------------
	override bool IsSshCompatible()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AER_ShellContext ctx, array<string> args)
	{
		array<string> longs = {"big"};
		ROOT_AER_Opts o = ROOT_AER_Opts.Parse(args, "", longs);
		if (!o.m_sError.IsEmpty())
		{
			ctx.Err("snake: " + o.m_sError);
			return;
		}

		ROOT_AER_Net.SendToPlayer(ctx.m_Session.m_iPlayerId, ROOT_AER_EMsg.SNAKE, ROOT_AER_Util.BoolStr(o.Has("big")));
	}
}
