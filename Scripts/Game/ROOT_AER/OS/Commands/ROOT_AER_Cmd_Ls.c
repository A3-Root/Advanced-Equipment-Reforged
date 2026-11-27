//! armaOS program 'ls': Display the content of a directory.
class ROOT_AER_Cmd_Ls : ROOT_AER_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "ls";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/ls";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Display the content of a directory.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage ls: 'ls [path]' returns a list of filesystem objects found in this path. 'ls -l' shows permissions, owner and size, 'ls -a' shows hidden entries.";
	}

	//------------------------------------------------------------------------------------------------
	static string Entry(ROOT_AER_FsNode node, string name, bool longList, int ownerLen)
	{
		string shown = name;
		if (node.m_bDir)
			shown += "/";
		else if (node.IsLink())
			shown += " -> " + node.m_sLink;

		if (!longList)
			return shown;

		string size = "-";
		if (!node.m_bDir)
			size = ROOT_AER_Util.FormatSize(node.GetSize());

		return string.Format("%1  %2  %3  %4", node.PermString(), ROOT_AER_Util.PadRight(node.m_sOwner, ownerLen), ROOT_AER_Util.PadLeft(size, 9), shown);
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AER_ShellContext ctx, array<string> args)
	{
		array<string> longs = {"long"};
		ROOT_AER_Opts o = ROOT_AER_Opts.Parse(args, "la", longs);
		if (!o.m_sError.IsEmpty())
		{
			ctx.Err("ls: " + o.m_sError);
			return;
		}

		bool longList = o.Has("l") || o.Has("long");
		bool all = o.Has("a");
		array<string> paths = {};
		paths.Copy(o.m_aArgs);
		if (paths.IsEmpty())
			paths.Insert(".");

		string user = ctx.FsUser();
		foreach (string p : paths)
		{
			string err;
			string abs = ctx.Abs(p);
			ROOT_AER_FsNode node = ctx.Fs().Resolve(abs, user, err);
			if (!node)
			{
				ctx.Err(string.Format("ls: cannot access '%1': %2", p, err));
				continue;
			}

			if (paths.Count() > 1)
				ctx.Out(p + ":");

			if (!node.m_bDir)
			{
				ctx.Out(Entry(node, node.m_sName, longList, 5));
				continue;
			}

			if (!node.HasPermission(user, 0))
			{
				ctx.Err(string.Format("ls: cannot open directory '%1': %2", p, ROOT_AER_Filesystem.ERR_PERMISSION));
				continue;
			}

			array<string> names = node.GetSortedNames();
			int ownerLen = 5;
			foreach (string n : names)
			{
				ownerLen = Math.Max(ownerLen, node.GetChild(n).m_sOwner.Length());
			}

			foreach (string n : names)
			{
				if (!all && n.StartsWith("."))
					continue;

				ctx.Out(Entry(node.GetChild(n), n, longList, ownerLen));
			}
		}
	}
}
