//! Ping an address or hostname: target bar with recent targets, results log (newest first, kept on resize).
class ROOT_AER_AppPing : ROOT_AER_App
{
	protected static const int ID_TARGET = 1;
	protected static const int ID_PING = 2;
	protected static const int ID_CLEAR = 3;
	protected static const int ID_RECENT = 100;
	protected static const int MAX_LOG = 12;

	protected static ref array<string> s_aRecent = {};

	protected EditBoxWidget m_wTarget;
	protected string m_sTarget = "192.168.1.1";
	protected ref array<string> m_aLog = {};		//!< Result blocks, newest first
	protected ref array<bool> m_aOk = {};

	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Ping";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "ping";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(620, 460, 0);
	}

	//------------------------------------------------------------------------------------------------
	override void Build()
	{
		float w = Width();
		float h = Height();
		float top = ToolbarBg(44);
		ROOT_AER_UI.Icon(GetContent(), 12, 13, 18, "ping");
		m_wTarget = Field(40, 9, w - 40 - 196, m_sTarget, ID_TARGET);
		MainBtn(w - 150, 9, 70, 26, "Ping", ID_PING);
		Btn(w - 74, 9, 68, 26, "Clear", ID_CLEAR);
		float y = top + 8;
		if (!s_aRecent.IsEmpty())
		{
			ROOT_AER_UI.Text(GetContent(), 12, y + 5, 60, 18, "Recent", 13, ROOT_AER_Theme.Muted());
			float x = 70;
			foreach (int i, string r : s_aRecent)
			{
				float bw = 20 + r.Length() * 7;
				if (x + bw > w - 8)
					break;

				ROOT_AER_UI.Button(GetContent(), x, y, bw, 26, r, ID_RECENT + i, this, ROOT_AER_Theme.Button(), ROOT_AER_Theme.Text(), 13);
				x += bw + 6;
			}

			y += 36;
		}

		ROOT_AER_UI.Box(GetContent(), 8, y, w - 16, h - y - 8, ROOT_AER_Theme.Field());
		if (m_aLog.IsEmpty())
		{
			ROOT_AER_UI.Text(GetContent(), 8, y + (h - y) * 0.4, w - 16, 20, "Enter an IP address or hostname and press Ping.", 14, ROOT_AER_Theme.Muted(), false, false, 1);
			return;
		}

		float ly = y + 10;
		foreach (int k, string block : m_aLog)
		{
			array<string> lines = {};
			block.Split("\n", lines, false);
			float bh = 8 + lines.Count() * 18;
			if (ly + bh > h - 12)
				break;

			Color mark = ROOT_AER_Theme.Good();
			if (!m_aOk[k])
				mark = ROOT_AER_Theme.Danger();

			ROOT_AER_UI.Rect(GetContent(), 16, ly + 2, 3, bh - 8, mark);
			foreach (int li, string line : lines)
			{
				Color col = ROOT_AER_Theme.Text();
				if (li == 0)
					col = ROOT_AER_Theme.Accent();

				ROOT_AER_UI.Text(GetContent(), 28, ly + li * 18, w - 52, 18, line, 13, col, true);
			}

			ly += bh + 6;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Ping()
	{
		if (m_wTarget)
			m_sTarget = m_wTarget.GetText().Trim();

		if (m_sTarget.IsEmpty())
		{
			Status("Enter an address.");
			return;
		}

		s_aRecent.RemoveItem(m_sTarget);
		s_aRecent.InsertAt(m_sTarget, 0);
		if (s_aRecent.Count() > 6)
			s_aRecent.Remove(6);

		Status("Pinging " + m_sTarget + "...");
		Request1("ping_host", m_sTarget);
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		string text = Arg(payload, 0);
		bool reached = ok && text.Contains("received");
		string now = ROOT_AER_Util.Now();
		if (now.Length() >= 19)
			now = now.Substring(11, 8);

		m_aLog.InsertAt(now + "  " + text, 0);
		m_aOk.InsertAt(reached, 0);
		if (m_aLog.Count() > MAX_LOG)
		{
			m_aLog.Remove(MAX_LOG);
			m_aOk.Remove(MAX_LOG);
		}

		if (reached)
			Status(m_sTarget + " replied.");
		else
			Status(m_sTarget + " did not reply.");

		Redraw();
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (id >= ID_RECENT && id < ID_RECENT + s_aRecent.Count())
		{
			m_sTarget = s_aRecent[id - ID_RECENT];
			ROOT_AER_UI.SetEdit(m_wTarget, m_sTarget);
			Ping();
			return true;
		}

		switch (id)
		{
			case ID_PING: Ping(); return true;
			case ID_CLEAR:
			{
				m_aLog.Clear();
				m_aOk.Clear();
				Redraw();
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiChange(Widget w, int id, bool finished)
	{
		if (id == ID_TARGET && m_wTarget)
			m_sTarget = m_wTarget.GetText();

		if (finished && id == ID_TARGET)
		{
			Ping();
			return true;
		}

		return false;
	}
}
