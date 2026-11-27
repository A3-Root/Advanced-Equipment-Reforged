//! Backtracking regular expression matcher.
//! Supports literals, '.', '^', '$', '*', '+', '?', character classes '[abc]' '[a-z]' '[^x]' and escapes '\d' '\w' '\s' '\.'.
class ROOT_AER_Regex
{
	//------------------------------------------------------------------------------------------------
	//! True when pattern matches anywhere in text.
	protected static int s_iSteps;
	protected static float s_fFrame = -1;
	protected static int s_iOwner;	//!< Player whose budget the next searches use
	protected static ref map<int, int> s_mUsed = new map<int, int>();
	protected static bool s_bLimitHit;
	static const int MAX_STEPS = 20000;	//!< Match steps per player and second; further searches fail
	static const int MAX_TEXT = 1024;

	//! shared = false: own step budget, not counted against player searches (allow lists).
	static bool Search(string pattern, string text, bool ignoreCase = false, bool shared = true)
	{
		float now = GetGame().GetWorld().GetWorldTime() / 1000.0;
		if (now - s_fFrame >= 1 || now < s_fFrame)
		{
			s_fFrame = now;
			s_mUsed.Clear();
		}

		if (shared)
			s_iSteps = s_mUsed.Get(s_iOwner);

		if (!shared)
		{
			int saved = s_iSteps;
			s_iSteps = MAX_STEPS - 20000;
			bool own = SearchBody(pattern, text, ignoreCase);
			s_iSteps = saved;
			return own;
		}

		if (s_iSteps > MAX_STEPS)
		{
			s_bLimitHit = true;
			return false;
		}

		bool found = SearchBody(pattern, text, ignoreCase);
		s_mUsed.Set(s_iOwner, s_iSteps);
		if (s_iSteps > MAX_STEPS)
			s_bLimitHit = true;

		return found;
	}

	//------------------------------------------------------------------------------------------------
	//! Searches until the next SetOwner count against this player's budget.
	static void SetOwner(int playerId)
	{
		s_iOwner = playerId;
		s_bLimitHit = false;
	}

	//------------------------------------------------------------------------------------------------
	//! A search since SetOwner stopped at the step limit.
	static bool LimitHit()
	{
		return s_bLimitHit;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool SearchBody(string pattern, string text, bool ignoreCase)
	{

		if (text.Length() > MAX_TEXT)
			text = text.Substring(0, MAX_TEXT);

		if (pattern.Length() > 256)
			return false;

		if (ignoreCase)
		{
			pattern.ToLower();
			text.ToLower();
		}

		if (pattern.StartsWith("^"))
			return MatchHere(pattern, 1, text, 0);

		for (int i = 0; i <= text.Length(); i++)
		{
			if (MatchHere(pattern, 0, text, i))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! True when pattern matches the whole text.
	static bool Full(string pattern, string text, bool ignoreCase = false, bool shared = true)
	{
		if (!pattern.StartsWith("^"))
			pattern = "^" + pattern;

		if (!pattern.EndsWith("$"))
			pattern = pattern + "$";

		return Search(pattern, text, ignoreCase, shared);
	}

	//------------------------------------------------------------------------------------------------
	//! Length of the atom at p (1 for a plain char, 2 for an escape, n for a class).
	protected static int AtomLen(string re, int p)
	{
		string c = re.Get(p);
		if (c == "\\" && p + 1 < re.Length())
			return 2;

		if (c == "[")
		{
			int q = p + 1;
			if (q < re.Length() && re.Get(q) == "^")
				q++;

			if (q < re.Length() && re.Get(q) == "]")
				q++;

			while (q < re.Length() && re.Get(q) != "]")
			{
				q++;
			}

			return q - p + 1;
		}

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool ClassMatch(string cls, string ch)
	{
		bool neg = false;
		int i = 0;
		if (cls.Get(0) == "^")
		{
			neg = true;
			i = 1;
		}

		bool hit = false;
		int n = cls.Length();
		int code = ch.ToAscii();
		while (i < n)
		{
			string a = cls.Get(i);
			if (a == "\\" && i + 1 < n)
			{
				if (EscapeMatch(cls.Get(i + 1), ch))
					hit = true;

				i += 2;
				continue;
			}

			if (i + 2 < n && cls.Get(i + 1) == "-")
			{
				if (code >= a.ToAscii() && code <= cls.ToAscii(i + 2))
					hit = true;

				i += 3;
				continue;
			}

			if (a == ch)
				hit = true;

			i++;
		}

		return hit != neg;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool EscapeMatch(string e, string ch)
	{
		int c = ch.ToAscii();
		switch (e)
		{
			case "d": return c >= 48 && c <= 57;
			case "w": return (c >= 48 && c <= 57) || (c >= 65 && c <= 90) || (c >= 97 && c <= 122) || c == 95;
			case "s": return c == 32 || c == 9 || c == 10 || c == 13;
		}

		return e == ch;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool AtomMatch(string re, int p, int len, string ch)
	{
		string c = re.Get(p);
		if (len == 2 && c == "\\")
			return EscapeMatch(re.Get(p + 1), ch);

		if (c == "[")
			return ClassMatch(re.Substring(p + 1, len - 2), ch);

		if (c == ".")
			return true;

		return c == ch;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool MatchHere(string re, int p, string text, int t)
	{
		s_iSteps++;
		if (s_iSteps > MAX_STEPS)
			return false;

		int rl = re.Length();
		if (p >= rl)
			return true;

		if (re.Get(p) == "$" && p == rl - 1)
			return t == text.Length();

		int len = AtomLen(re, p);
		string q;
		if (p + len < rl)
			q = re.Get(p + len);

		if (q == "*" || q == "+" || q == "?")
		{
			int maxRep = text.Length() - t;
			if (q == "?")
				maxRep = Math.Min(1, maxRep);

			int count = 0;
			while (count < maxRep && AtomMatch(re, p, len, text.Get(t + count)))
			{
				count++;
			}

			int minRep = 0;
			if (q == "+")
				minRep = 1;

			for (int k = count; k >= minRep; k--)
			{
				if (MatchHere(re, p + len + 1, text, t + k))
					return true;
			}

			return false;
		}

		if (t < text.Length() && AtomMatch(re, p, len, text.Get(t)))
			return MatchHere(re, p + len, text, t + 1);

		return false;
	}
}
