//! Packs string lists into one string for RPC and storage.
//! Level 1 separator ASCII 30, level 2 separator ASCII 31. Both are stripped from user text.
class ROOT_AE3_Codec
{
	protected static string s_sSep1;
	protected static string s_sSep2;

	//------------------------------------------------------------------------------------------------
	static string Sep1()
	{
		if (s_sSep1.IsEmpty())
			s_sSep1 = (30).AsciiToString();

		return s_sSep1;
	}

	//------------------------------------------------------------------------------------------------
	static string Sep2()
	{
		if (s_sSep2.IsEmpty())
			s_sSep2 = (31).AsciiToString();

		return s_sSep2;
	}

	//------------------------------------------------------------------------------------------------
	//! Removes both separators from text.
	static string Clean(string text)
	{
		text.Replace(Sep1(), "");
		text.Replace(Sep2(), "");
		return text;
	}

	//------------------------------------------------------------------------------------------------
	static string Join(notnull array<string> fields)
	{
		return Join1(fields, Sep1());
	}

	//------------------------------------------------------------------------------------------------
	static string JoinSub(notnull array<string> fields)
	{
		return Join1(fields, Sep2());
	}

	//------------------------------------------------------------------------------------------------
	protected static string Join1(notnull array<string> fields, string sep)
	{
		string result;
		foreach (int i, string f : fields)
		{
			if (i > 0)
				result += sep;

			result += f;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	static array<string> Split(string data)
	{
		array<string> res = {};
		if (data.IsEmpty())
			return res;

		data.Split(Sep1(), res, false);
		return res;
	}

	//------------------------------------------------------------------------------------------------
	static array<string> SplitSub(string data)
	{
		array<string> res = {};
		if (data.IsEmpty())
			return res;

		data.Split(Sep2(), res, false);
		return res;
	}
}
