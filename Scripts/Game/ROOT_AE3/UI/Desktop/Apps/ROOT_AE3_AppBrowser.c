//! Web browser for in-game pages: address bar, tabs, back / forward, history, markdown pages with links.
class ROOT_AE3_AppBrowser : ROOT_AE3_App
{
	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Browser";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "global";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(820, 560, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected EditBoxWidget m_wAddress;
	protected RichTextWidget m_wPage;
	protected TextWidget m_wTitle;
	protected static const int MAX_TABS = 6;
	protected ref array<string> m_aTabs = {"rootnet.home"};
	protected int m_iTab;
	protected ref array<string> m_aBack = {};
	protected ref array<string> m_aForward = {};
	protected string m_sContent;

	override void Build()
	{
		float w = Width();
		float h = Height();
		float x = 6;
		foreach (int i, string t : m_aTabs)
		{
			string label = t;
			if (label.Length() > 16)
				label = label.Substring(0, 16);

			Color bg = ROOT_AE3_Theme.Button();
			if (i == m_iTab)
				bg = ROOT_AE3_Theme.Accent();

			ROOT_AE3_UI.Button(GetContent(), x, 4, 120, 24, label, 200 + i, this, bg, ROOT_AE3_Theme.Text(), 13);
			x += 124;
		}

		Btn(x, 4, 28, 24, "+", 10);
		Btn(x + 32, 4, 28, 24, "x", 11);
		Btn(6, 32, 36, 26, "<", 1);
		Btn(46, 32, 36, 26, ">", 2);
		Btn(86, 32, 56, 26, "Home", 3);
		m_wAddress = Field(146, 32, w - 300, m_aTabs[m_iTab], 4);
		Btn(w - 150, 32, 50, 26, "Go", 5);
		Btn(w - 96, 32, 90, 26, "History", 6);
		m_wTitle = Label(10, 64, w - 20, 24, "", 18);
		ROOT_AE3_UI.Rect(GetContent(), 6, 90, w - 12, h - 96, ROOT_AE3_Theme.Field());
		m_wPage = ROOT_AE3_UI.Rich(GetContent(), 14, 96, w - 28, h - 108, "", 15, ROOT_AE3_Theme.Text());
		ROOT_AE3_UI.Listen(m_wPage, 7, this);
		Go(m_aTabs[m_iTab], false);
	}

	//------------------------------------------------------------------------------------------------
	protected void Go(string url, bool record = true)
	{
		if (record && m_aTabs[m_iTab] != url)
		{
			m_aBack.Insert(m_aTabs[m_iTab]);
			m_aForward.Clear();
		}

		m_aTabs[m_iTab] = url;
		if (m_wAddress)
			ROOT_AE3_UI.SetEdit(m_wAddress, url);

		Request1("web_get", url);
	}

	//------------------------------------------------------------------------------------------------
	//! Markdown subset to rich text: headings, bold, lists, links.
	static string Render(string md)
	{
		array<string> lines = {};
		md.Split("\n", lines, false);
		string html;
		foreach (string l : lines)
		{
			string t = l;
			if (t.StartsWith("### "))
				t = "<b>" + t.Substring(4, t.Length() - 4) + "</b>";
			else if (t.StartsWith("## "))
				t = "<h2>" + t.Substring(3, t.Length() - 3) + "</h2>";
			else if (t.StartsWith("# "))
				t = "<h1>" + t.Substring(2, t.Length() - 2) + "</h1>";
			else if (t.StartsWith("- ") || t.StartsWith("* "))
				t = "  - " + t.Substring(2, t.Length() - 2);

			while (t.Contains("**"))
			{
				int a = t.IndexOf("**");
				int b = t.IndexOfFrom(a + 2, "**");
				if (b < 0)
					break;

				t = t.Substring(0, a) + "<b>" + t.Substring(a + 2, b - a - 2) + "</b>" + t.Substring(b + 2, t.Length() - b - 2);
			}

			int guard;
			while (t.Contains("](") && guard < 20)
			{
				guard++;
				int close = t.IndexOf("](");
				int open = t.LastIndexOf("[");
				int end = t.IndexOfFrom(close, ")");
				if (open < 0 || open > close || end < 0)
					break;

				string text = t.Substring(open + 1, close - open - 1);
				string link = t.Substring(close + 2, end - close - 2);
				t = t.Substring(0, open) + "<a href=\"" + link + "\">" + text + "</a>" + t.Substring(end + 1, t.Length() - end - 1);
			}

			html += t + "<br/>";
		}

		return html;
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (cmd == "web_history")
		{
			string md = "# History\n\n";
			foreach (string l : payload)
			{
				int bar = l.IndexOf("|");
				if (bar < 0)
					continue;

				string url = l.Substring(bar + 1, l.Length() - bar - 1);
				md += "- " + l.Substring(0, bar) + "  [" + url + "](" + url + ")\n";
			}

			m_wTitle.SetText("History");
			m_wPage.SetText(Render(md));
			return;
		}

		if (!ok)
		{
			m_wTitle.SetText("Page not found");
			m_wPage.SetText(Render("# 404\n\nThe address " + m_aTabs[m_iTab] + " is not available on this network.\n\n" + Arg(payload, 0)));
			return;
		}

		m_wTitle.SetText(Arg(payload, 0));
		m_wPage.SetText(Render(Arg(payload, 1)));
		Status(m_aTabs[m_iTab]);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiLink(Widget w, int id, string link)
	{
		Go(link);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiChange(Widget w, int id, bool finished)
	{
		if (finished && id == 4)
		{
			Go(m_wAddress.GetText());
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (id >= 200 && id < 200 + m_aTabs.Count())
		{
			m_iTab = id - 200;
			Redraw();
			return true;
		}

		switch (id)
		{
			case 1:
			{
				if (m_aBack.IsEmpty())
					return true;

				m_aForward.Insert(m_aTabs[m_iTab]);
				string url = m_aBack[m_aBack.Count() - 1];
				m_aBack.Remove(m_aBack.Count() - 1);
				Go(url, false);
				return true;
			}

			case 2:
			{
				if (m_aForward.IsEmpty())
					return true;

				m_aBack.Insert(m_aTabs[m_iTab]);
				string url = m_aForward[m_aForward.Count() - 1];
				m_aForward.Remove(m_aForward.Count() - 1);
				Go(url, false);
				return true;
			}

			case 3: Go("rootnet.home"); return true;
			case 5: Go(m_wAddress.GetText()); return true;
			case 6: Request0("web_history"); return true;
			case 10:
			{
				if (m_aTabs.Count() < MAX_TABS)
				{
					m_aTabs.Insert("rootnet.home");
					m_iTab = m_aTabs.Count() - 1;
					Redraw();
				}

				return true;
			}

			case 11:
			{
				if (m_aTabs.Count() > 1)
				{
					m_aTabs.Remove(m_iTab);
					m_iTab = Math.Max(0, m_iTab - 1);
					Redraw();
				}

				return true;
			}
		}

		return false;
	}
}
