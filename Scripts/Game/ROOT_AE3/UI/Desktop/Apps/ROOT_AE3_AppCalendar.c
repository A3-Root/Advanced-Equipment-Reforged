//! Calendar: month grid (previous / next month and year, go to date, today), days with events marked,
//! day panel with the events of the selected day, delete, and a form to add an event.
class ROOT_AE3_AppCalendar : ROOT_AE3_App
{
	protected static const int ID_PREV_YEAR = 1;
	protected static const int ID_PREV_MONTH = 2;
	protected static const int ID_NEXT_MONTH = 3;
	protected static const int ID_NEXT_YEAR = 4;
	protected static const int ID_GO = 5;
	protected static const int ID_TODAY = 6;
	protected static const int ID_REFRESH = 7;
	protected static const int ID_ADD = 8;
	protected static const int ID_GOTO = 9;
	protected static const int ID_DAY = 100;
	protected static const int ID_DEL = 200;
	protected static const float GRID_W = 420;
	protected static ref array<string> MONTHS = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
	protected static ref array<string> DAYS = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};

	protected ref array<ref array<string>> m_aEvents = {};		//!< date, time, title, text, location
	protected ref array<int> m_aDayEvents = {};
	protected int m_iYear;
	protected int m_iMonth;		//!< 1-12
	protected string m_sSel;
	protected bool m_bLoaded;
	protected EditBoxWidget m_wGoto;
	protected EditBoxWidget m_wTitle;
	protected EditBoxWidget m_wTime;
	protected EditBoxWidget m_wLoc;
	protected MultilineEditBoxWidget m_wBody;
	protected string m_sMsg;

	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Calendar";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "calendar";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(880, 560, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Leap(int y)
	{
		return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
	}

	//------------------------------------------------------------------------------------------------
	protected static int DaysIn(int y, int m)
	{
		array<int> days = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
		if (m == 2 && Leap(y))
			return 29;

		return days[m - 1];
	}

	//------------------------------------------------------------------------------------------------
	//! 0 Sunday ... 6 Saturday.
	protected static int Weekday(int y, int m, int d)
	{
		array<int> t = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
		if (m < 3)
			y -= 1;

		return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Iso(int y, int m, int d)
	{
		return string.Format("%1-%2-%3", y, ROOT_AE3_Util.PadLeft(m.ToString(), 2, "0"), ROOT_AE3_Util.PadLeft(d.ToString(), 2, "0"));
	}

	//------------------------------------------------------------------------------------------------
	//! Parses YYYY-MM-DD; false when invalid.
	protected static bool ParseIso(string iso, out int y, out int m, out int d)
	{
		array<string> p = {};
		string clean = iso.Trim();
		clean.Split("-", p, true);
		if (p.Count() != 3)
			return false;

		y = p[0].ToInt();
		m = p[1].ToInt();
		d = p[2].ToInt();
		return y > 0 && m >= 1 && m <= 12 && d >= 1 && d <= DaysIn(y, m);
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasEvents(string iso)
	{
		foreach (array<string> e : m_aEvents)
		{
			if (e[0] == iso)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void Build()
	{
		if (m_iYear == 0)
		{
			int d;
			if (!ParseIso(ROOT_AE3_Util.Today(), m_iYear, m_iMonth, d))
			{
				m_iYear = 2026;
				m_iMonth = 1;
			}

			m_sSel = ROOT_AE3_Util.Today();
		}

		float h = Height();
		ROOT_AE3_UI.Rect(GetContent(), 0, 0, GRID_W, h, ROOT_AE3_Theme.Sidebar());
		Btn(10, 10, 36, 28, "<<", ID_PREV_YEAR);
		Btn(50, 10, 36, 28, "<", ID_PREV_MONTH);
		ROOT_AE3_UI.Text(GetContent(), 90, 13, GRID_W - 180, 24, MONTHS[m_iMonth - 1] + " " + m_iYear, 17, ROOT_AE3_Theme.Text(), false, false, 1);
		Btn(GRID_W - 86, 10, 36, 28, ">", ID_NEXT_MONTH);
		Btn(GRID_W - 46, 10, 36, 28, ">>", ID_NEXT_YEAR);
		m_wGoto = Field(10, 46, GRID_W - 210, "", ID_GOTO);
		ROOT_AE3_UI.Placeholder(m_wGoto, GetContent(), 18, 50, GRID_W - 226, "YYYY-MM-DD", ROOT_AE3_Theme.Muted());
		MainBtn(GRID_W - 194, 46, 56, 27, "Go", ID_GO);
		Btn(GRID_W - 132, 46, 66, 27, "Today", ID_TODAY);
		ROOT_AE3_UI.Box(GetContent(), GRID_W - 60, 46, 50, 27, ROOT_AE3_Theme.Button(), ROOT_AE3_Theme.Border());
		ROOT_AE3_UI.IconButton(GetContent(), GRID_W - 59, 47, 48, 25, "g_refresh", ID_REFRESH, this, 14, ROOT_AE3_Theme.Text(), ROOT_AE3_Theme.Button(), ROOT_AE3_Theme.Hover());

		float gx = 10;
		float gy = 86;
		float cw = (GRID_W - 20) / 7;
		float ch = Math.Min(60, (h - gy - 30) / 6);
		foreach (int i, string dn : DAYS)
		{
			ROOT_AE3_UI.Text(GetContent(), gx + i * cw, gy, cw, 18, dn, 12, ROOT_AE3_Theme.Muted(), false, false, 1);
		}

		gy += 22;
		string today = ROOT_AE3_Util.Today();
		int first = Weekday(m_iYear, m_iMonth, 1);
		int count = DaysIn(m_iYear, m_iMonth);
		for (int day = 1; day <= count; day++)
		{
			int slot = first + day - 1;
			int col = slot % 7;
			int row = slot / 7;
			float x = gx + col * cw;
			float y = gy + row * ch;
			string iso = Iso(m_iYear, m_iMonth, day);
			Color bg = ROOT_AE3_Theme.Field();
			Color fg = ROOT_AE3_Theme.Text();
			if (iso == today)
			{
				bg = ROOT_AE3_Theme.Accent();
				fg = ROOT_AE3_UI.Rgb(1, 1, 1);
			}

			if (iso == m_sSel)
				ROOT_AE3_UI.Rect(GetContent(), x + 1, y + 1, cw - 2, ch - 2, ROOT_AE3_Theme.Accent());

			float inset = 2;
			if (iso == m_sSel)
				inset = 4;

			ROOT_AE3_UI.Hit(GetContent(), x + inset, y + inset, cw - inset * 2, ch - inset * 2, ID_DAY + day, this, bg, ROOT_AE3_Theme.Hover());
			ROOT_AE3_UI.Text(GetContent(), x, y + ch * 0.5 - 12, cw, 20, day.ToString(), 15, fg, false, false, 1);
			if (HasEvents(iso))
				ROOT_AE3_UI.Rect(GetContent(), x + cw * 0.5 - 3, y + ch - 12, 6, 6, ROOT_AE3_UI.Rgb(1, 0.55, 0.35));
		}

		ROOT_AE3_UI.Rect(GetContent(), GRID_W, 0, 1, h, ROOT_AE3_Theme.Border());
		BuildDay();
		if (!m_bLoaded)
		{
			m_bLoaded = true;
			Request0("cal_list");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildDay()
	{
		float x = GRID_W + 18;
		float w = Width() - x - 16;
		float h = Height();
		float y = 14;
		if (m_sSel.IsEmpty())
		{
			ROOT_AE3_UI.Text(GetContent(), x, h * 0.45, w, 22, "Select a day to view or add events.", 15, ROOT_AE3_Theme.Muted(), false, false, 1);
			return;
		}

		int sy, sm, sd;
		string heading = m_sSel;
		if (ParseIso(m_sSel, sy, sm, sd))
		{
			array<string> names = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
			heading = string.Format("%1, %2 %3 %4", names[Weekday(sy, sm, sd)], sd, MONTHS[sm - 1], sy);
		}

		Label(x, y, w, 26, heading, 18);
		y += 36;
		m_aDayEvents.Clear();
		foreach (int i, array<string> e : m_aEvents)
		{
			if (e[0] == m_sSel)
				m_aDayEvents.Insert(i);
		}

		float formY = h - 196;
		if (m_aDayEvents.IsEmpty())
		{
			ROOT_AE3_UI.Text(GetContent(), x, y, w, 20, "No events.", 14, ROOT_AE3_Theme.Muted());
			y += 26;
		}

		foreach (int k, int idx : m_aDayEvents)
		{
			array<string> ev = m_aEvents[idx];
			float cardH = 34;
			if (Arg(ev, 4) != "")
				cardH += 18;

			if (ev[3] != "")
				cardH += 36;

			if (y + cardH > formY - 8)
			{
				ROOT_AE3_UI.Text(GetContent(), x, y, w, 18, string.Format("+ %1 more", m_aDayEvents.Count() - k), 13, ROOT_AE3_Theme.Muted());
				break;
			}

			ROOT_AE3_UI.Box(GetContent(), x, y, w, cardH, ROOT_AE3_Theme.Field(), ROOT_AE3_Theme.Border());
			ROOT_AE3_UI.Rect(GetContent(), x, y, 4, cardH, ROOT_AE3_Theme.Accent());
			string title = ev[2];
			if (!ev[1].IsEmpty())
				title = ev[1] + "  -  " + title;

			ROOT_AE3_UI.Text(GetContent(), x + 14, y + 8, w - 60, 20, title, 15, ROOT_AE3_Theme.Text());
			ROOT_AE3_UI.IconButton(GetContent(), x + w - 34, y + 4, 28, 26, "g_close", ID_DEL + k, this, 12, ROOT_AE3_Theme.Muted(), ROOT_AE3_UI.Rgb(0, 0, 0, 0), ROOT_AE3_Theme.Danger());
			float ty = y + 30;
			if (Arg(ev, 4) != "")
			{
				ROOT_AE3_UI.Text(GetContent(), x + 14, ty, w - 28, 16, "@ " + ev[4], 12, ROOT_AE3_Theme.Muted());
				ty += 18;
			}

			if (ev[3] != "")
				ROOT_AE3_UI.Text(GetContent(), x + 14, ty, w - 28, 34, ev[3], 13, ROOT_AE3_Theme.Text(), false, true);

			y += cardH + 8;
		}

		// New event form.
		y = formY;
		ROOT_AE3_UI.Rect(GetContent(), x, y - 8, w, 1, ROOT_AE3_Theme.Border());
		m_wTitle = Field(x, y, w, "", 20);
		ROOT_AE3_UI.Placeholder(m_wTitle, GetContent(), x + 8, y + 4, w - 16, "New event title", ROOT_AE3_Theme.Muted());
		y += 32;
		m_wTime = Field(x, y, 140, "", 21);
		ROOT_AE3_UI.Placeholder(m_wTime, GetContent(), x + 8, y + 4, 124, "Time HH:MM", ROOT_AE3_Theme.Muted());
		m_wLoc = Field(x + 148, y, w - 148, "", 22);
		ROOT_AE3_UI.Placeholder(m_wLoc, GetContent(), x + 156, y + 4, w - 164, "Location (optional)", ROOT_AE3_Theme.Muted());
		y += 32;
		ROOT_AE3_UI.Box(GetContent(), x, y, w, 70, ROOT_AE3_Theme.Field(), ROOT_AE3_Theme.Border());
		m_wBody = ROOT_AE3_UI.MultiEdit(GetContent(), x + 6, y + 4, w - 12, 62, "", 23, this);
		if (m_wBody)
			m_wBody.SetColor(ROOT_AE3_Theme.Text());

		y += 78;
		MainBtn(x, y, 110, 28, "Add event", ID_ADD);
		if (!m_sMsg.IsEmpty())
			ROOT_AE3_UI.Text(GetContent(), x + 120, y + 5, w - 120, 20, m_sMsg, 13, ROOT_AE3_Theme.Muted());
	}

	//------------------------------------------------------------------------------------------------
	protected void Shift(int months)
	{
		int total = m_iYear * 12 + (m_iMonth - 1) + months;
		m_iYear = total / 12;
		m_iMonth = total % 12 + 1;
		m_sMsg = "";
		Redraw();
	}

	//------------------------------------------------------------------------------------------------
	protected void GoTo(string iso)
	{
		int y, m, d;
		if (!ParseIso(iso, y, m, d))
		{
			Status("Enter a date as YYYY-MM-DD.");
			return;
		}

		m_iYear = y;
		m_iMonth = m;
		m_sSel = Iso(y, m, d);
		m_sMsg = "";
		Redraw();
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (!ok)
		{
			m_sMsg = "Failed: " + Arg(payload, 0);
			Redraw();
			return;
		}

		if (cmd != "cal_list")
		{
			if (cmd == "cal_add")
				m_sMsg = "Added.";

			Request0("cal_list");
			return;
		}

		m_aEvents.Clear();
		foreach (string p : payload)
		{
			array<string> e = ROOT_AE3_Codec.SplitSub(p);
			if (e.Count() < 4)
				continue;

			if (e.Count() < 5)
				e.Insert("");

			m_aEvents.Insert(e);
		}

		Status(string.Format("%1 events", m_aEvents.Count()));
		Redraw();
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiChange(Widget w, int id, bool finished)
	{
		if (finished && id == ID_GOTO && m_wGoto)
		{
			GoTo(m_wGoto.GetText());
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnKey(KeyCode key, bool ctrl)
	{
		switch (key)
		{
			case KeyCode.KC_LEFT: Shift(-1); return true;
			case KeyCode.KC_RIGHT: Shift(1); return true;
			case KeyCode.KC_F5: Request0("cal_list"); return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (id > ID_DAY && id <= ID_DAY + 31)
		{
			m_sSel = Iso(m_iYear, m_iMonth, id - ID_DAY);
			m_sMsg = "";
			Redraw();
			return true;
		}

		if (id >= ID_DEL && id < ID_DEL + m_aDayEvents.Count())
		{
			array<string> ev = m_aEvents[m_aDayEvents[id - ID_DEL]];
			Request2("cal_remove", ev[0], ev[2]);
			return true;
		}

		switch (id)
		{
			case ID_PREV_YEAR: Shift(-12); return true;
			case ID_PREV_MONTH: Shift(-1); return true;
			case ID_NEXT_MONTH: Shift(1); return true;
			case ID_NEXT_YEAR: Shift(12); return true;
			case ID_GO:
			{
				if (m_wGoto)
					GoTo(m_wGoto.GetText());

				return true;
			}

			case ID_TODAY: GoTo(ROOT_AE3_Util.Today()); return true;
			case ID_REFRESH: Request0("cal_list"); return true;
			case ID_ADD:
			{
				if (!m_wTitle || m_wTitle.GetText().Trim().IsEmpty())
				{
					m_sMsg = "Title required.";
					Redraw();
					return true;
				}

				array<string> args = {m_sSel, m_wTime.GetText().Trim(), m_wTitle.GetText().Trim(), m_wBody.GetText(), m_wLoc.GetText().Trim()};
				Request("cal_add", args);
				return true;
			}
		}

		return false;
	}
}
