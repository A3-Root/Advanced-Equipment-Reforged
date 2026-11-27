//! System information in cards: System (host, user, date), Power (state, battery meter, draw),
//! Network (address, gateway), Storage (disk, USB ports). Refreshes every 3 s.
class ROOT_AER_AppSysInfo : ROOT_AER_App
{
	protected float m_fTimer;
	protected ref map<string, string> m_mValues = new map<string, string>();

	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "System Info";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "info";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(620, 420, 0);
	}

	//------------------------------------------------------------------------------------------------
	override void Build()
	{
		float w = Width();
		float h = Height();
		ROOT_AER_UI.Rect(GetContent(), 0, 0, w, h, ROOT_AER_Theme.Panel());
		if (m_mValues.IsEmpty())
		{
			ROOT_AER_UI.Text(GetContent(), 0, h * 0.4, w, 20, "Loading...", 14, ROOT_AER_Theme.Muted(), false, false, 1);
			Request0("sysinfo");
			return;
		}

		ROOT_AER_UI.Icon(GetContent(), 16, 14, 40, "computer");
		ROOT_AER_UI.Text(GetContent(), 66, 14, w - 80, 24, V("Host"), 20, ROOT_AER_Theme.Text());
		ROOT_AER_UI.Text(GetContent(), 66, 38, w - 80, 18, "Signed in as " + V("User") + "   " + V("Date"), 13, ROOT_AER_Theme.Muted());
		float cw = (w - 36) * 0.5;
		float ch = (h - 86) * 0.5;
		float x1 = 12;
		float x2 = 24 + cw;
		float y1 = 70;
		float y2 = 82 + ch;

		Card(x1, y1, cw, ch - 6);
		float y = Heading(x1 + 12, y1 + 8, cw - 24, "Power");
		y = KeyValue(x1 + 12, y, cw - 24, "State", V("Power"), 90);
		string bat = V("Battery");
		if (!bat.IsEmpty())
		{
			y = KeyValue(x1 + 12, y, cw - 24, "Battery", bat, 90);
			Meter(x1 + 12, y, cw - 24, 10, bat.ToInt() / 100.0);
			y += 18;
		}

		KeyValue(x1 + 12, y, cw - 24, "Draw", V("Draw"), 90);

		Card(x2, y1, cw, ch - 6);
		y = Heading(x2 + 12, y1 + 8, cw - 24, "Network");
		string addr = V("Address");
		y = KeyValue(x2 + 12, y, cw - 24, "Address", addr, 90);
		y = KeyValue(x2 + 12, y, cw - 24, "Gateway", V("Gateway"), 90);
		Color state = ROOT_AER_Theme.Good();
		string label = "Connected";
		if (addr.IsEmpty() || addr == "not connected")
		{
			state = ROOT_AER_Theme.Danger();
			label = "Not connected";
		}

		ROOT_AER_UI.Rect(GetContent(), x2 + 12, y + 6, 8, 8, state);
		ROOT_AER_UI.Text(GetContent(), x2 + 28, y, cw - 40, 20, label, 14, ROOT_AER_Theme.Text());

		Card(x1, y2, w - 24, ch - 6);
		y = Heading(x1 + 12, y2 + 8, w - 48, "Storage");
		y = KeyValue(x1 + 12, y, w - 48, "Disk used", V("Disk used"), 110);
		KeyValue(x1 + 12, y, w - 48, "USB ports", V("USB ports"), 110);
	}

	//------------------------------------------------------------------------------------------------
	protected string V(string key)
	{
		string v;
		m_mValues.Find(key, v);
		return v;
	}

	//------------------------------------------------------------------------------------------------
	override void Update(float dt)
	{
		m_fTimer += dt;
		if (m_fTimer > 3)
		{
			m_fTimer = 0;
			Request0("sysinfo");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Reply: "Key: value" lines.
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (!ok)
			return;

		array<string> lines = {};
		Arg(payload, 0).Split("\n", lines, true);
		m_mValues.Clear();
		foreach (string line : lines)
		{
			int c = line.IndexOf(":");
			if (c <= 0)
				continue;

			string key = line.Substring(0, c);
			string value = line.Substring(c + 1, line.Length() - c - 1);
			value.TrimInPlace();
			m_mValues.Set(key, value);
		}

		Redraw();
	}
}
