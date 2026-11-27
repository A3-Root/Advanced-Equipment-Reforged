//! armaOS program 'help': Prints all installed programs.
class ROOT_AER_Cmd_Help : ROOT_AER_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "help";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/help";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Prints all installed programs.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage help: 'help' returns a list of available programs. No options needed.";
	}

	//------------------------------------------------------------------------------------------------
	//! Program installed on the executing computer under a name, null when missing.
	static ROOT_AER_Command FindInstalled(ROOT_AER_ShellContext ctx, string name)
	{
		foreach (string dir : ROOT_AER_Shell.GetPath())
		{
			string err;
			ROOT_AER_FsNode f = ctx.Fs().Resolve(dir + "/" + name, "root", err);
			if (f && !f.m_bDir)
				return ROOT_AER_CommandRegistry.Get().Find(ROOT_AER_CommandRegistry.ProgramOf(f.m_sContent));
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AER_ShellContext ctx, array<string> args)
	{
		array<string> names = {};
		map<string, string> desc = new map<string, string>();
		ROOT_AER_Filesystem fs = ctx.Fs();
		foreach (string dir : ROOT_AER_Shell.GetPath())
		{
			string err;
			ROOT_AER_FsNode d = fs.Resolve(dir, "root", err);
			if (!d || !d.m_bDir)
				continue;

			foreach (string n : d.GetSortedNames())
			{
				ROOT_AER_FsNode f = d.GetChild(n);
				if (f.m_bDir || names.Contains(n))
					continue;

				string program = ROOT_AER_CommandRegistry.ProgramOf(f.m_sContent);
				ROOT_AER_Command c = ROOT_AER_CommandRegistry.Get().Find(program);
				if (!c)
					continue;

				names.Insert(n);
				desc.Set(n, c.GetDescription());
			}
		}

		names.Sort();
		int width = 8;
		foreach (string n : names)
		{
			width = Math.Max(width, n.Length() + 2);
		}

		foreach (string n : names)
		{
			ctx.Out(ROOT_AER_Util.PadRight(n, width) + desc.Get(n));
		}
	}
}
