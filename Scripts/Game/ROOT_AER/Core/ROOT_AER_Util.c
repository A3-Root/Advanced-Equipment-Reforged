//! Shared string, number and time helpers.
class ROOT_AER_Util
{
	//------------------------------------------------------------------------------------------------
	//! Entries separated by commas when the text has one, else by spaces.
	static array<string> List(string text)
	{
		if (!text.Contains(","))
			return Words(text);

		array<string> parts = {};
		text.Split(",", parts, true);
		array<string> res = {};
		foreach (string p : parts)
		{
			p = p.Trim();
			if (!p.IsEmpty())
				res.Insert(p);
		}

		return res;
	}

	//------------------------------------------------------------------------------------------------
	//! Last len characters, starting at a line break when one is inside.
	static string Last(string s, int len)
	{
		if (s.Length() <= len)
			return s;

		string tail = s.Substring(s.Length() - len, len);
		int nl = tail.IndexOf("\n");
		if (nl >= 0)
			tail = tail.Substring(nl + 1, tail.Length() - nl - 1);

		return tail;
	}

	//------------------------------------------------------------------------------------------------
	//! First len characters.
	static string Clip(string s, int len)
	{
		if (s.Length() <= len)
			return s;

		return s.Substring(0, len);
	}

	//------------------------------------------------------------------------------------------------
	//! Damage state destroyed.
	static bool IsDestroyed(IEntity ent)
	{
		if (!ent)
			return true;

		DamageManagerComponent dmg = DamageManagerComponent.Cast(ent.FindComponent(DamageManagerComponent));
		return dmg && dmg.GetState() == EDamageState.DESTROYED;
	}

	//------------------------------------------------------------------------------------------------
	//! In an inventory or a character's hands.
	static bool IsCarried(IEntity ent)
	{
		if (!ent)
			return false;

		InventoryItemComponent item = InventoryItemComponent.Cast(ent.FindComponent(InventoryItemComponent));
		return item && item.GetParentSlot();
	}

	//------------------------------------------------------------------------------------------------
	//! In the inventory of the character (any depth: uniform, vest, backpack; also while seated in a vehicle).
	static bool CarriedBy(IEntity ent, IEntity character)
	{
		if (!character || !IsCarried(ent))
			return false;

		IEntity p = ent.GetParent();
		while (p)
		{
			if (p == character)
				return true;

			p = p.GetParent();
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! World position: the carrier's position for an item in an inventory.
	static vector WorldPos(IEntity ent)
	{
		if (!ent)
			return vector.Zero;

		if (IsCarried(ent))
		{
			IEntity root = ent.GetRootParent();
			if (root)
				return root.GetOrigin();
		}

		return ent.GetOrigin();
	}

	//------------------------------------------------------------------------------------------------
	//! Character whose inventory holds the item, null when not carried by a character.
	static IEntity Carrier(IEntity ent)
	{
		if (!IsCarried(ent))
			return null;

		IEntity p = ent.GetParent();
		while (p)
		{
			if (ChimeraCharacter.Cast(p))
				return p;

			p = p.GetParent();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	static string PadRight(string s, int len)
	{
		while (s.Length() < len)
			s += " ";

		return s;
	}

	//------------------------------------------------------------------------------------------------
	static string PadLeft(string s, int len, string ch = " ")
	{
		while (s.Length() < len)
			s = ch + s;

		return s;
	}

	//------------------------------------------------------------------------------------------------
	static string Repeat(string s, int count)
	{
		string r;
		for (int i = 0; i < count; i++)
		{
			r += s;
		}

		return r;
	}

	//------------------------------------------------------------------------------------------------
	//! Byte count as B / KB / MB / GB with one decimal.
	static string FormatSize(int bytes)
	{
		if (bytes < 1024)
			return bytes.ToString() + " B";

		float v = bytes / 1024.0;
		if (v < 1024)
			return FormatFloat(v, 1) + " KB";

		v = v / 1024.0;
		if (v < 1024)
			return FormatFloat(v, 1) + " MB";

		return FormatFloat(v / 1024.0, 1) + " GB";
	}

	//------------------------------------------------------------------------------------------------
	static string FormatFloat(float v, int decimals)
	{
		float m = Math.Pow(10, decimals);
		float r = Math.Round(v * m) / m;
		string s = r.ToString(-1, decimals);
		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! Mission date and time as YYYY-MM-DD HH:MM:SS.
	static string Now()
	{
		int y, mo, d, h, mi, s;
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		TimeAndWeatherManagerEntity tw;
		if (world)
			tw = world.GetTimeAndWeatherManager();

		if (tw)
		{
			tw.GetDate(y, mo, d);
			tw.GetHoursMinutesSeconds(h, mi, s);
		}
		else
		{
			System.GetYearMonthDay(y, mo, d);
			System.GetHourMinuteSecond(h, mi, s);
		}

		return string.Format("%1-%2-%3 %4:%5:%6", y, PadLeft(mo.ToString(), 2, "0"), PadLeft(d.ToString(), 2, "0"),
			PadLeft(h.ToString(), 2, "0"), PadLeft(mi.ToString(), 2, "0"), PadLeft(s.ToString(), 2, "0"));
	}

	//------------------------------------------------------------------------------------------------
	//! Mission date as YYYY-MM-DD.
	static string Today()
	{
		string now = Now();
		return now.Substring(0, 10);
	}

	//------------------------------------------------------------------------------------------------
	//! Splits on whitespace runs.
	static array<string> Words(string s)
	{
		array<string> res = {};
		s.Replace("\t", " ");
		s.Split(" ", res, true);
		return res;
	}

	//------------------------------------------------------------------------------------------------
	static string JoinFrom(notnull array<string> parts, int start, string sep = " ")
	{
		string r;
		for (int i = start; i < parts.Count(); i++)
		{
			if (i > start)
				r += sep;

			r += parts[i];
		}

		return r;
	}

	//------------------------------------------------------------------------------------------------
	//! Keeps printable ASCII, newline and tab.
	static string Ascii(string s)
	{
		string r;
		int n = s.Length();
		for (int i = 0; i < n; i++)
		{
			int c = s.ToAscii(i);
			if ((c >= 32 && c < 127) || c == 10 || c == 9)
				r += s.Get(i);
		}

		return r;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsInt(string s)
	{
		if (s.IsEmpty())
			return false;

		for (int i = 0; i < s.Length(); i++)
		{
			if (!s.IsDigitAt(i))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static string Lower(string s)
	{
		s.ToLower();
		return s;
	}

	//------------------------------------------------------------------------------------------------
	static string BoolStr(bool b)
	{
		if (b)
			return "1";

		return "0";
	}

	//------------------------------------------------------------------------------------------------
	static float BoolNum(bool b)
	{
		if (b)
			return 1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Parses "a.b.c.d". Returns false on bad format.
	static bool ParseIp(string ip, out int a, out int b, out int c, out int d)
	{
		array<string> p = {};
		ip.Split(".", p, false);
		if (p.Count() != 4)
			return false;

		foreach (string s : p)
		{
			if (!IsInt(s) || s.ToInt() > 255)
				return false;
		}

		a = p[0].ToInt();
		b = p[1].ToInt();
		c = p[2].ToInt();
		d = p[3].ToInt();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! "a.b.c" of "a.b.c.d".
	static string Subnet(string ip)
	{
		int i = ip.LastIndexOf(".");
		if (i < 0)
			return ip;

		return ip.Substring(0, i);
	}
}
