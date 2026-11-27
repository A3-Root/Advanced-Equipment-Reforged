//! armaOS terminal: header bar and scrollable console. Text is rendered client-side from the lines sent by the
//! server; the prompt and the typed text follow the last line with a cursor (the edit box that takes the keys is
//! not drawn). Enter sends the line, Tab completes, Up / Down recall lines.
class ROOT_AER_TerminalView : ROOT_AER_UiListener
{
	protected ref ROOT_AER_DesignReply m_DesignReply;
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
	protected string m_sShownInput;
	protected float m_fSincePrompt;
	protected float m_fBlink;
	protected bool m_bCursor = true;

	//------------------------------------------------------------------------------------------------
	void ROOT_AER_TerminalView(Widget parent)
	{
		m_fFontScale = ROOT_AER_SettingsManager.Get().m_fTerminalDefaultSize;
		m_wRoot = ROOT_AER_UI.Frame(parent, 0, 0, 100, 100);
		ROOT_AER_UI.Fill(m_wRoot);
		Build();
	}

	//------------------------------------------------------------------------------------------------
	void Destroy()
	{
		ReleaseHandlers();
		if (GetGame())
			GetGame().GetCallqueue().Remove(Focus);

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
		ROOT_AER_UI.Clear(m_wRoot);
		m_aHeaderTexts.Clear();
		ROOT_AER_UI.ScreenSize(m_fW, m_fH);
		float x = m_fW * 0.08;
		float y = m_fH * 0.06;
		float w = m_fW * 0.84;
		float h = m_fH * 0.88;
		ROOT_AER_Client c = ROOT_AER_Client.Get();
		m_wHeaderBg = ROOT_AER_UI.Rect(m_wRoot, x, y, w, 36, Color.FromInt(Color.BLACK));
		m_wConsoleBg = ROOT_AER_UI.Rect(m_wRoot, x, y + 36, w, h - 36, Color.FromInt(Color.BLACK), ID_CONSOLE, this);
		m_wTitle = ROOT_AER_UI.Text(m_wRoot, x + 12, y + 8, w * 0.45, 24, ROOT_AER_SettingsManager.Get().m_sTerminalDialogTitle + "  -  " + c.m_sHostname, 18);
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

			ROOT_AER_UI.Button(m_wRoot, bx, y + 5, bw, 26, l, ids[i], this, ROOT_AER_UI.Rgb(1, 1, 1, 0.12));
		}

		float fs = FontSize();
		float inputY = y + h - fs - 18;
		m_wConsole = ROOT_AER_UI.Text(m_wRoot, x + 12, y + 44, w - 24, inputY - y - 50, "", fs, null, true, false);
		m_wPrompt = ROOT_AER_UI.Text(m_wRoot, x + 12, inputY + 2, w * 0.4, fs + 8, "", fs, null, true);
		m_wPrompt.SetVisible(false);
		m_wInput = ROOT_AER_UI.Edit(m_wRoot, x + 12, inputY, w - 200, fs + 10, "", ID_INPUT, this, false, true);
		if (m_wInput)
			m_wInput.SetOpacity(0);
		ROOT_AER_UI.Button(m_wRoot, x + w - 182, inputY, 50, fs + 10, "Tab", ID_TAB, this, ROOT_AER_UI.Rgb(1, 1, 1, 0.12));
		ROOT_AER_UI.Button(m_wRoot, x + w - 126, inputY, 50, fs + 10, "Up", ID_UP, this, ROOT_AER_UI.Rgb(1, 1, 1, 0.12));
		ROOT_AER_UI.Button(m_wRoot, x + w - 70, inputY, 58, fs + 10, "Down", ID_DOWN, this, ROOT_AER_UI.Rgb(1, 1, 1, 0.12));
		ApplyDesign();
		Refresh();
		RefreshPrompt();
	}

	//------------------------------------------------------------------------------------------------
	void ApplyDesign()
	{
		int d = ROOT_AER_Client.Get().m_iDesign;
		m_wHeaderBg.SetColor(ROOT_AER_TerminalDesigns.Get(d, 0));
		m_wConsoleBg.SetColor(ROOT_AER_TerminalDesigns.Get(d, 1));
		foreach (TextWidget t : m_aHeaderTexts)
		{
			t.SetColor(ROOT_AER_TerminalDesigns.Get(d, 2));
		}

		Color text = ROOT_AER_TerminalDesigns.Get(d, 3);
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

		ROOT_AER_Client c = ROOT_AER_Client.Get();
		array<string> lines = {};
		int start = Math.Max(0, c.m_aLines.Count() - 150);
		for (int i = start; i < c.m_aLines.Count(); i++)
		{
			lines.Insert(c.m_aLines[i]);
		}

		lines.Insert(InputLine());
		int cols = Columns();
		array<string> wrapped = {};
		for (int i = 0; i < lines.Count(); i++)
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

		// Text starting with '#' is looked up in the string table.
		if (text.StartsWith("#"))
			text = " " + text;

		m_wConsole.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	//! Prompt, typed text (stars while hidden) and cursor; empty while a command runs.
	protected string InputLine()
	{
		ROOT_AER_Client c = ROOT_AER_Client.Get();
		if (c.m_bBusy)
			return "";

		string typed;
		if (m_wInput)
			typed = m_wInput.GetText();

		m_sShownInput = typed;
		if (c.m_bHidden)
			typed = ROOT_AER_Util.Repeat("*", typed.Length());

		string cursor = " ";
		if (m_bCursor)
			cursor = "_";

		return c.m_sPrompt + typed + cursor;
	}

	//------------------------------------------------------------------------------------------------
	void RefreshPrompt()
	{
		ROOT_AER_Client c = ROOT_AER_Client.Get();
		m_fSincePrompt = 0;
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

		Refresh();
	}

	//------------------------------------------------------------------------------------------------
	void SetInput(string line)
	{
		ROOT_AER_UI.SetEdit(m_wInput, line);
		Focus();
	}

	//------------------------------------------------------------------------------------------------
	protected void Submit()
	{
		ROOT_AER_Client c = ROOT_AER_Client.Get();
		string line = m_wInput.GetText();
		ROOT_AER_UI.SetEdit(m_wInput, "");
		m_iScroll = 0;
		m_iHistoryPos = -1;
		if (!c.m_bHidden && !line.Trim().IsEmpty() && c.m_sPrompt.EndsWith("/>"))
			c.m_aHistory.Insert(line);

		c.Send(ROOT_AER_EMsg.TERM_LINE, line);
		m_sLastSynced = "";
		// Write mode ends after the change event: activated again next frame.
		GetGame().GetCallqueue().CallLater(Focus, 0, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void History(int dir)
	{
		array<string> h = ROOT_AER_Client.Get().m_aHistory;
		if (h.IsEmpty())
			return;

		if (m_iHistoryPos < 0)
			m_iHistoryPos = h.Count();

		m_iHistoryPos = Math.ClampInt(m_iHistoryPos + dir, 0, h.Count());
		if (m_iHistoryPos >= h.Count())
			ROOT_AER_UI.SetEdit(m_wInput, "");
		else
			ROOT_AER_UI.SetEdit(m_wInput, h[m_iHistoryPos]);

		Focus();
	}

	//------------------------------------------------------------------------------------------------
	void Update(float dt)
	{
		m_fSincePrompt += dt;
		// Typing goes to the input line whenever no button is held.
		if (m_wInput && !m_wInput.IsInWriteMode() && GetGame().GetInputManager().GetActionValue("MouseLeft") <= 0)
			Focus();

		m_fBlink += dt;
		if (m_fBlink >= 0.5)
		{
			m_fBlink = 0;
			m_bCursor = !m_bCursor;
			Refresh();
		}
		else if (m_wInput && m_wInput.GetText() != m_sShownInput)
		{
			m_bCursor = true;
			m_fBlink = 0;
			Refresh();
		}

		bool up = ROOT_AER_Keys.Pressed(KeyCode.KC_UP);
		bool down = ROOT_AER_Keys.Pressed(KeyCode.KC_DOWN);
		if (ROOT_AER_Keys.Pressed(KeyCode.KC_TAB))
			ROOT_AER_Client.Get().Send(ROOT_AER_EMsg.TERM_TAB, m_wInput.GetText());
		else if (up)
			History(-1);
		else if (down)
			History(1);

		ROOT_AER_Client c = ROOT_AER_Client.Get();
		ROOT_AER_Settings s = ROOT_AER_SettingsManager.Get();
		m_fSyncTimer += dt;
		if (m_fSyncTimer < Math.Max(0.05, s.m_fUiKeystrokeSyncInterval) || c.m_bHidden)
			return;

		m_fSyncTimer = 0;
		string text = m_wInput.GetText();
		if (text != m_sLastSynced)
		{
			m_sLastSynced = text;
			if (ROOT_AER_SettingsManager.Get().m_bUiOnTexture)
				ROOT_AER_Client.Get().Send(ROOT_AER_EMsg.TERM_KEY, "INPUT:" + text);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiChange(Widget w, int id, bool finished)
	{
		if (id == ID_INPUT && finished)
		{
			if (GetGame().GetInputManager().GetActionValue("MouseLeft") > 0)
				return false;

			// Write mode ends without Enter when the box is refocused after a prompt: no empty line then.
			bool enter = Debug.KeyState(KeyCode.KC_RETURN) != 0 || Debug.KeyState(KeyCode.KC_NUMPADENTER) != 0;
			if (m_wInput && m_wInput.GetText().IsEmpty() && (m_fSincePrompt < 0.6 || !enter))
				return true;

			Submit();
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiChar(Widget w, int id, int charCode)
	{
		// Tab completion is read in Update.
		return id == ID_INPUT && charCode == 9;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiWheel(Widget w, int id, int wheel)
	{
		int step = Math.Max(1, ROOT_AER_SettingsManager.Get().m_iTerminalScrollSpeed);
		m_iScroll += wheel * step;
		Refresh();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		ROOT_AER_Client c = ROOT_AER_Client.Get();
		switch (id)
		{
			case ID_CLOSE: c.m_Menu.Close(); return true;
			case ID_TAB: c.Send(ROOT_AER_EMsg.TERM_TAB, m_wInput.GetText()); return true;
			case ID_UP: History(-1); return true;
			case ID_DOWN: History(1); return true;
			case ID_CTRLC: c.Send(ROOT_AER_EMsg.TERM_KEY, "CTRL_C"); return true;
			case ID_DESKTOP: c.Send(ROOT_AER_EMsg.TERM_KEY, "MODE:1"); return true;
			case ID_CONSOLE: Focus(); return true;
			case ID_DESIGN:
			{
				m_DesignReply = new ROOT_AER_DesignReply();
				m_DesignReply.m_iPrevious = c.m_iDesign;
				m_DesignReply.m_View = this;
				c.m_iDesign = (c.m_iDesign + 1) % ROOT_AER_TerminalDesigns.COUNT;
				ApplyDesign();
				array<string> args = {"design", c.m_iDesign.ToString()};
				c.Request("sys_set", args, m_DesignReply);
				SCR_HintManagerComponent.ShowCustomHint(ROOT_AER_TerminalDesigns.Name(c.m_iDesign), "Terminal design", 1.5);
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
				ROOT_AER_UI.SetEdit(m_wInput, kept);
				Focus();
				return true;
			}
		}

		return false;
	}
}
