//! armaOS terminal: header bar, scrollable console, prompt and input line. Text is rendered client-side
//! from the lines sent by the server; Enter sends the line, Tab completes, history buttons recall lines.
class ROOT_AE3_TerminalView : ROOT_AE3_UiListener
{
	protected ref ROOT_AE3_DesignReply m_DesignReply;
	protected static const int ID_CLOSE = 1;
	protected static const int ID_DESIGN = 2;
	protected static const int ID_DESKTOP = 3;
	protected static const int ID_TAB = 4;
	protected static const int ID_UP = 5;
	protected static const int ID_DOWN = 6;
	protected static const int ID_INPUT = 7;
	protected static const int ID_CONSOLE = 8;
	protected static const int ID_SMALLER = 9;
	protected static const int ID_BIGGER = 10;
	protected static const int ID_CTRLC = 11;

	protected FrameWidget m_wRoot;
	protected ImageWidget m_wHeaderBg;
	protected ImageWidget m_wConsoleBg;
	protected TextWidget m_wTitle;
	protected TextWidget m_wConsole;
	protected TextWidget m_wPrompt;
	protected EditBoxWidget m_wInput;
	protected ref array<TextWidget> m_aHeaderTexts = {};
	protected int m_iScroll;
	protected int m_iHistoryPos = -1;
	protected float m_fFontScale = 0.75;
	protected float m_fW;
	protected float m_fH;
	protected string m_sLastSynced;
	protected float m_fSyncTimer;
	protected bool m_bShown;

	//------------------------------------------------------------------------------------------------
	void ROOT_AE3_TerminalView(Widget parent)
	{
		m_fFontScale = ROOT_AE3_SettingsManager.Get().m_fTerminalDefaultSize;
		m_wRoot = ROOT_AE3_UI.Frame(parent, 0, 0, 100, 100);
		ROOT_AE3_UI.Fill(m_wRoot);
		Build();
	}

	//------------------------------------------------------------------------------------------------
	void Destroy()
	{
		ReleaseHandlers();
		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();
	}

	//------------------------------------------------------------------------------------------------
	protected int FontSize()
	{
		return Math.Round(24 * m_fFontScale);
	}

	//------------------------------------------------------------------------------------------------
	protected void Build()
	{
		ReleaseHandlers();
		ROOT_AE3_UI.Clear(m_wRoot);
		m_aHeaderTexts.Clear();
		ROOT_AE3_UI.ScreenSize(m_fW, m_fH);
		float x = m_fW * 0.08;
		float y = m_fH * 0.06;
		float w = m_fW * 0.84;
		float h = m_fH * 0.88;
		ROOT_AE3_Client c = ROOT_AE3_Client.Get();
		m_wHeaderBg = ROOT_AE3_UI.Rect(m_wRoot, x, y, w, 36, Color.FromInt(Color.BLACK));
		m_wConsoleBg = ROOT_AE3_UI.Rect(m_wRoot, x, y + 36, w, h - 36, Color.FromInt(Color.BLACK), ID_CONSOLE, this);
		m_wTitle = ROOT_AE3_UI.Text(m_wRoot, x + 12, y + 8, w * 0.45, 24, ROOT_AE3_SettingsManager.Get().m_sTerminalDialogTitle + "  -  " + c.m_sHostname, 18);
		m_aHeaderTexts.Insert(m_wTitle);
		float bx = x + w - 8;
		array<string> labels = {"X", "Ctrl+C", "Desktop", "Design", "A+", "A-"};
		array<int> ids = {ID_CLOSE, ID_CTRLC, ID_DESKTOP, ID_DESIGN, ID_BIGGER, ID_SMALLER};
		foreach (int i, string l : labels)
		{
			float bw = 30 + l.Length() * 8;
			bx -= bw + 6;
			if (ids[i] == ID_DESKTOP && c.m_iInterface == 0)
				continue;

			ROOT_AE3_UI.Button(m_wRoot, bx, y + 5, bw, 26, l, ids[i], this, ROOT_AE3_UI.Rgb(1, 1, 1, 0.12));
		}

		float fs = FontSize();
		float inputY = y + h - fs - 18;
		m_wConsole = ROOT_AE3_UI.Text(m_wRoot, x + 12, y + 44, w - 24, inputY - y - 50, "", fs, null, true, false);
		m_wPrompt = ROOT_AE3_UI.Text(m_wRoot, x + 12, inputY + 2, w * 0.4, fs + 8, "", fs, null, true);
		m_wInput = ROOT_AE3_UI.Edit(m_wRoot, x + 12, inputY, w - 200, fs + 10, "", ID_INPUT, this, false, true);
		ROOT_AE3_UI.Button(m_wRoot, x + w - 182, inputY, 50, fs + 10, "Tab", ID_TAB, this, ROOT_AE3_UI.Rgb(1, 1, 1, 0.12));
		ROOT_AE3_UI.Button(m_wRoot, x + w - 126, inputY, 50, fs + 10, "Up", ID_UP, this, ROOT_AE3_UI.Rgb(1, 1, 1, 0.12));
		ROOT_AE3_UI.Button(m_wRoot, x + w - 70, inputY, 58, fs + 10, "Down", ID_DOWN, this, ROOT_AE3_UI.Rgb(1, 1, 1, 0.12));
		ApplyDesign();
		Refresh();
		RefreshPrompt();
	}

	//------------------------------------------------------------------------------------------------
	void ApplyDesign()
	{
		int d = ROOT_AE3_Client.Get().m_iDesign;
		m_wHeaderBg.SetColor(ROOT_AE3_TerminalDesigns.Get(d, 0));
		m_wConsoleBg.SetColor(ROOT_AE3_TerminalDesigns.Get(d, 1));
		foreach (TextWidget t : m_aHeaderTexts)
		{
			t.SetColor(ROOT_AE3_TerminalDesigns.Get(d, 2));
		}

		Color text = ROOT_AE3_TerminalDesigns.Get(d, 3);
		m_wConsole.SetColor(text);
		m_wPrompt.SetColor(text);
		if (m_wInput)
			m_wInput.SetColor(text);
	}

	//------------------------------------------------------------------------------------------------
	bool IsShown()
	{
		return m_bShown;
	}

	//------------------------------------------------------------------------------------------------
	void Show(bool show)
	{
		m_bShown = show;
		m_wRoot.SetVisible(show);
		if (show)
			Focus();
	}

	//------------------------------------------------------------------------------------------------
	void Focus()
	{
		if (!m_wInput)
			return;

		GetGame().GetWorkspace().SetFocusedWidget(m_wInput);
		m_wInput.ActivateWriteMode();
	}

	//------------------------------------------------------------------------------------------------
	protected int VisibleLines()
	{
		float lineH = FontSize() * 1.25;
		float h = FrameSlot.GetSizeY(m_wConsole);
		return Math.Max(3, Math.Floor(h / lineH));
	}

	//------------------------------------------------------------------------------------------------
	protected int Columns()
	{
		float charW = FontSize() * 0.6;
		return Math.Max(20, Math.Floor(FrameSlot.GetSizeX(m_wConsole) / charW));
	}

	//------------------------------------------------------------------------------------------------
	void Refresh()
	{
		if (!m_wConsole)
			return;

		array<string> lines = ROOT_AE3_Client.Get().m_aLines;
		int cols = Columns();
		array<string> wrapped = {};
		int start = Math.Max(0, lines.Count() - 150);
		for (int i = start; i < lines.Count(); i++)
		{
			string l = lines[i];
			while (l.Length() > cols)
			{
				wrapped.Insert(l.Substring(0, cols));
				l = l.Substring(cols, l.Length() - cols);
			}

			wrapped.Insert(l);
		}

		int vis = VisibleLines();
		int maxScroll = Math.Max(0, wrapped.Count() - vis);
		m_iScroll = Math.ClampInt(m_iScroll, 0, maxScroll);
		int end = wrapped.Count() - m_iScroll;
		int first = Math.Max(0, end - vis);
		string text;
		for (int i = first; i < end; i++)
		{
			if (i > first)
				text += "\n";

			text += wrapped[i];
		}

		m_wConsole.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	void RefreshPrompt()
	{
		ROOT_AE3_Client c = ROOT_AE3_Client.Get();
		string prompt = c.m_sPrompt;
		if (c.m_bBusy)
			prompt = "";

		m_wPrompt.SetText(prompt);
		float pw = (prompt.Length() + 1) * FontSize() * 0.6;
		Widget editRoot = m_wInput.GetParent();
		float x = FrameSlot.GetPosX(m_wPrompt);
		FrameSlot.SetPosX(editRoot, x + pw);
		FrameSlot.SetSizeX(editRoot, Math.Max(80, FrameSlot.GetSizeX(m_wConsole) - pw - 180));
		if (c.m_bHidden)
			m_wInput.SetObfuscationChar("*");
		else
			m_wInput.SetObfuscationChar("");
	}

	//------------------------------------------------------------------------------------------------
	void SetInput(string line)
	{
		m_wInput.SetText(line);
		Focus();
	}

	//------------------------------------------------------------------------------------------------
	protected void Submit()
	{
		ROOT_AE3_Client c = ROOT_AE3_Client.Get();
		string line = m_wInput.GetText();
		m_wInput.SetText("");
		m_iScroll = 0;
		m_iHistoryPos = -1;
		if (!c.m_bHidden && !line.Trim().IsEmpty() && c.m_sPrompt.EndsWith("/>"))
			c.m_aHistory.Insert(line);

		c.Send(ROOT_AE3_EMsg.TERM_LINE, line);
		m_sLastSynced = "";
		Focus();
	}

	//------------------------------------------------------------------------------------------------
	protected void History(int dir)
	{
		array<string> h = ROOT_AE3_Client.Get().m_aHistory;
		if (h.IsEmpty())
			return;

		if (m_iHistoryPos < 0)
			m_iHistoryPos = h.Count();

		m_iHistoryPos = Math.ClampInt(m_iHistoryPos + dir, 0, h.Count());
		if (m_iHistoryPos >= h.Count())
			m_wInput.SetText("");
		else
			m_wInput.SetText(h[m_iHistoryPos]);

		Focus();
	}

	//------------------------------------------------------------------------------------------------
	void Update(float dt)
	{
		InputManager im = GetGame().GetInputManager();
		if (im.GetActionTriggered("MenuUp"))
			History(-1);
		else if (im.GetActionTriggered("MenuDown"))
			History(1);

		ROOT_AE3_Client c = ROOT_AE3_Client.Get();
		ROOT_AE3_Settings s = ROOT_AE3_SettingsManager.Get();
		m_fSyncTimer += dt;
		if (m_fSyncTimer < Math.Max(0.05, s.m_fUiKeystrokeSyncInterval) || c.m_bHidden)
			return;

		m_fSyncTimer = 0;
		string text = m_wInput.GetText();
		if (text != m_sLastSynced)
		{
			m_sLastSynced = text;
			if (ROOT_AE3_SettingsManager.Get().m_bUiOnTexture)
				ROOT_AE3_Client.Get().Send(ROOT_AE3_EMsg.TERM_KEY, "INPUT:" + text);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiChange(Widget w, int id, bool finished)
	{
		if (id == ID_INPUT && finished)
		{
			if (GetGame().GetInputManager().GetActionValue("MouseLeft") > 0)
				return false;

			Submit();
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiChar(Widget w, int id, int charCode)
	{
		if (id == ID_INPUT && charCode == 9)
		{
			ROOT_AE3_Client.Get().Send(ROOT_AE3_EMsg.TERM_TAB, m_wInput.GetText());
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiWheel(Widget w, int id, int wheel)
	{
		int step = Math.Max(1, ROOT_AE3_SettingsManager.Get().m_iTerminalScrollSpeed);
		m_iScroll += wheel * step;
		Refresh();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		ROOT_AE3_Client c = ROOT_AE3_Client.Get();
		switch (id)
		{
			case ID_CLOSE: c.m_Menu.Close(); return true;
			case ID_TAB: c.Send(ROOT_AE3_EMsg.TERM_TAB, m_wInput.GetText()); return true;
			case ID_UP: History(-1); return true;
			case ID_DOWN: History(1); return true;
			case ID_CTRLC: c.Send(ROOT_AE3_EMsg.TERM_KEY, "CTRL_C"); return true;
			case ID_DESKTOP: c.Send(ROOT_AE3_EMsg.TERM_KEY, "MODE:1"); return true;
			case ID_CONSOLE: Focus(); return true;
			case ID_DESIGN:
			{
				m_DesignReply = new ROOT_AE3_DesignReply();
				m_DesignReply.m_iPrevious = c.m_iDesign;
				m_DesignReply.m_View = this;
				c.m_iDesign = (c.m_iDesign + 1) % ROOT_AE3_TerminalDesigns.COUNT;
				ApplyDesign();
				array<string> args = {"design", c.m_iDesign.ToString()};
				c.Request("sys_set", args, m_DesignReply);
				SCR_HintManagerComponent.ShowCustomHint(ROOT_AE3_TerminalDesigns.Name(c.m_iDesign), "Terminal design", 1.5);
				return true;
			}

			case ID_BIGGER:
			case ID_SMALLER:
			{
				if (id == ID_BIGGER)
					m_fFontScale = Math.Min(1.5, m_fFontScale + 0.1);
				else
					m_fFontScale = Math.Max(0.5, m_fFontScale - 0.1);

				string kept = m_wInput.GetText();
				Build();
				m_wInput.SetText(kept);
				Focus();
				return true;
			}
		}

		return false;
	}
}
