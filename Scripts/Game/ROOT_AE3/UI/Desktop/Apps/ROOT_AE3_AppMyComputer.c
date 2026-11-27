//! Volumes as cards: the local disk and each USB port with its drive, state, usage meter and Open, Mount /
//! Unmount and Eject buttons; below, the flash drives in the player's inventory with Insert.
class ROOT_AE3_AppMyComputer : ROOT_AE3_App
{
	protected static const int ID_REFRESH = 1;
	protected static const int ID_CARD = 100;	//!< + volume * 10 + action
	protected static const int A_OPEN = 0;
	protected static const int A_MOUNT = 1;
	protected static const int A_UNMOUNT = 2;
	protected static const int A_EJECT = 3;
	protected static const int ID_INSERT = 1000;	//!< + carried drive

	protected ref array<ref array<string>> m_aDrives = {};	//!< Carried drives: "drive", id, name

	protected ref array<ref array<string>> m_aVolumes = {};	//!< id, path, device, state, used text, bytes, capacity MB

	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "My Computer";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "computer";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(640, 540, 0);
	}

	//------------------------------------------------------------------------------------------------
	override bool NeedsFileBrowsing()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Build()
	{
		float w = Width();
		float h = Height();
		float top = ToolbarBg(44);
		ROOT_AE3_UI.Icon(GetContent(), 12, 11, 22, "computer");
		ROOT_AE3_UI.Text(GetContent(), 44, 13, w - 200, 20, ROOT_AE3_Theme.Caption("Devices and drives"), 16, ROOT_AE3_Theme.Text());
		ROOT_AE3_UI.ItemButton(GetContent(), w - 118, 6, 112, 32, "g_refresh", "Refresh", ID_REFRESH, this, 14);
		ROOT_AE3_UI.Rect(GetContent(), 0, top, w, h - top, ROOT_AE3_Theme.Panel());
		if (m_aVolumes.IsEmpty())
		{
			ROOT_AE3_UI.Text(GetContent(), 0, h * 0.45, w, 20, "Loading...", 14, ROOT_AE3_Theme.Muted(), false, false, 1);
			Request0("vol_list");
			return;
		}

		float cw = (w - 36) * 0.5;
		float ch = 112;
		float bottom = top;
		foreach (int i, array<string> v : m_aVolumes)
		{
			int col = i % 2;
			int row = i / 2;
			float x = 12 + col * (cw + 12);
			float y = top + 12 + row * (ch + 12);
			if (y + ch > h)
				break;

			BuildCard(i, v, x, y, cw, ch);
			bottom = y + ch;
		}

		float dy = bottom + 16;
		ROOT_AE3_UI.Text(GetContent(), 12, dy, w - 24, 18, ROOT_AE3_Theme.Caption("Flash drives in your inventory"), 13, ROOT_AE3_Theme.Muted());
		dy += 24;
		if (m_aDrives.IsEmpty())
		{
			ROOT_AE3_UI.Text(GetContent(), 12, dy, w - 24, 18, "None", 13, ROOT_AE3_Theme.Muted());
			return;
		}

		foreach (int k, array<string> d : m_aDrives)
		{
			if (dy + 32 > h)
				break;

			ROOT_AE3_UI.Icon(GetContent(), 14, dy + 4, 20, "drive");
			ROOT_AE3_UI.Text(GetContent(), 42, dy + 6, w - 170, 18, d[2], 14, ROOT_AE3_Theme.Text());
			Btn(w - 112, dy + 2, 100, 26, "Insert", ID_INSERT + k);
			dy += 34;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildCard(int i, array<string> v, float x, float y, float cw, float ch)
	{
		bool root = v[0] == "root";
		bool present = root || v[2] != "empty";
		bool mounted = v[3] == "mounted";
		Card(x, y, cw, ch);
		string icon = "drive";
		string name = "USB port " + v[0].Substring(3, v[0].Length() - 3);
		if (root)
		{
			icon = "computer";
			name = "Local disk (" + v[2] + ")";
		}

		ROOT_AE3_UI.Icon(GetContent(), x + 12, y + 12, 40, icon);
		ROOT_AE3_UI.Text(GetContent(), x + 62, y + 12, cw - 74, 20, name, 15, ROOT_AE3_Theme.Text());
		string sub = v[1];
		if (!root)
		{
			if (present)
				sub = v[2] + " - " + v[3];
			else
				sub = "No drive";
		}

		ROOT_AE3_UI.Text(GetContent(), x + 62, y + 34, cw - 74, 18, sub, 13, ROOT_AE3_Theme.Muted());
		if (present)
		{
			float usedMb = v[5].ToFloat() / 1048576;
			float capMb = v[6].ToFloat();
			string usage = v[4] + " used";
			float level = 0;
			if (capMb > 0)
			{
				level = usedMb / capMb;
				string capText = ROOT_AE3_Util.FormatFloat(capMb, 0) + " MB";
				if (capMb >= 1024)
					capText = ROOT_AE3_Util.FormatFloat(capMb / 1024, 1) + " GB";

				usage = v[4] + " of " + capText;
			}

			Meter(x + 62, y + 58, cw - 74, 8, level);
			ROOT_AE3_UI.Text(GetContent(), x + 62, y + 68, cw - 74, 16, usage, 12, ROOT_AE3_Theme.Muted());
		}

		float bx = x + 62;
		float by = y + ch - 32;
		int base = ID_CARD + i * 10;
		if (present && (root || mounted))
		{
			Btn(bx, by, 70, 24, "Open", base + A_OPEN);
			bx += 76;
		}

		if (!root && present)
		{
			if (mounted)
				Btn(bx, by, 84, 24, "Unmount", base + A_UNMOUNT);
			else
				Btn(bx, by, 84, 24, "Mount", base + A_MOUNT);

			bx += 90;
			Btn(bx, by, 70, 24, "Eject", base + A_EJECT);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (!ok)
		{
			Status(Arg(payload, 0));
			return;
		}

		if (cmd != "vol_list")
		{
			Status(Arg(payload, 0));
			Request0("vol_list");
			return;
		}

		m_aVolumes.Clear();
		m_aDrives.Clear();
		foreach (string p : payload)
		{
			array<string> e = ROOT_AE3_Codec.SplitSub(p);
			if (e.Count() >= 3 && e[0] == "drive")
			{
				m_aDrives.Insert(e);
				continue;
			}

			if (e.Count() < 5)
				continue;

			while (e.Count() < 7)
			{
				e.Insert("0");
			}

			m_aVolumes.Insert(e);
		}

		Redraw();
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (id == ID_REFRESH)
		{
			Request0("vol_list");
			return true;
		}

		if (id >= ID_INSERT && id < ID_INSERT + m_aDrives.Count())
		{
			Request1("vol_insert", m_aDrives[id - ID_INSERT][1]);
			return true;
		}

		int i = (id - ID_CARD) / 10;
		int action = (id - ID_CARD) % 10;
		if (id < ID_CARD || i >= m_aVolumes.Count())
			return false;

		string vol = m_aVolumes[i][0];
		switch (action)
		{
			case A_OPEN:
			{
				if (vol == "root")
					m_Desktop.OpenApp(ROOT_AE3_AppFiles, "/");
				else
					m_Desktop.OpenApp(ROOT_AE3_AppFiles, "/mnt/" + vol);

				return true;
			}

			case A_MOUNT: Request1("vol_mount", vol); return true;
			case A_UNMOUNT: Request1("vol_unmount", vol); return true;
			case A_EJECT: Request1("vol_eject", vol); return true;
		}

		return false;
	}
}
