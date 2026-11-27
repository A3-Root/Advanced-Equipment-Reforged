//! Command line options: short flags (-l, -iv), long flags (--big) and positional arguments.
class ROOT_AER_Opts
{
	ref array<string> m_aFlags = {};
	ref array<string> m_aArgs = {};
	string m_sError;

	//------------------------------------------------------------------------------------------------
	//! shortFlags: allowed letters, for example "liv". longFlags: allowed names without dashes.
	static ROOT_AER_Opts Parse(notnull array<string> args, string shortFlags, array<string> longFlags = null)
	{
		ROOT_AER_Opts o = new ROOT_AER_Opts();
		bool onlyArgs;
		foreach (string a : args)
		{
			if (onlyArgs || a == "-" || !a.StartsWith("-") || ROOT_AER_Util.IsInt(a.Substring(1, a.Length() - 1)))
			{
				o.m_aArgs.Insert(a);
				continue;
			}

			if (a == "--")
			{
				onlyArgs = true;
				continue;
			}

			if (a.StartsWith("--"))
			{
				string name = a.Substring(2, a.Length() - 2);
				if (!longFlags || !longFlags.Contains(name))
				{
					o.m_sError = string.Format("invalid option '%1'", a);
					return o;
				}

				o.m_aFlags.Insert(name);
				continue;
			}

			for (int i = 1; i < a.Length(); i++)
			{
				string f = a.Get(i);
				if (!shortFlags.Contains(f))
				{
					o.m_sError = string.Format("invalid option -- '%1'", f);
					return o;
				}

				o.m_aFlags.Insert(f);
			}
		}

		return o;
	}

	//------------------------------------------------------------------------------------------------
	bool Has(string flag)
	{
		return m_aFlags.Contains(flag);
	}

	//------------------------------------------------------------------------------------------------
	int Count()
	{
		return m_aArgs.Count();
	}

	//------------------------------------------------------------------------------------------------
	string Arg(int i, string def = "")
	{
		if (i < 0 || i >= m_aArgs.Count())
			return def;

		return m_aArgs[i];
	}
}
