//! Desktop application shown in a window. Subclasses build into GetContent() and talk to the server
//! with Request(cmd, args); replies arrive in OnGuiReply.
class ROOT_AE3_App : ROOT_AE3_GuiReplyHandler
{
	ROOT_AE3_Window m_Window;
	ROOT_AE3_DesktopView m_Desktop;
	string m_sArg;		//!< Start argument (for example a path)
	protected ref array<ref ROOT_AE3_ListView> m_aLists = {};

	//------------------------------------------------------------------------------------------------
	string GetTitle()
	{
		return "App";
	}

	//------------------------------------------------------------------------------------------------
	//! Icon name in icons_wrapperUI-64.imageset.
	string GetIcon()
	{
		return "misc";
	}

	//------------------------------------------------------------------------------------------------
	//! Initial window size.
	vector GetSize()
	{
		return Vector(640, 420, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! Runs instead of opening a window when it returns true.
	bool Launch()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Shown in the start menu.
	bool IsListed()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Icon on the desktop.
	bool IsOnDesktop()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Needs the file browsing setting.
	bool NeedsFileBrowsing()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	FrameWidget GetContent()
	{
		return m_Window.GetContent();
	}

	//------------------------------------------------------------------------------------------------
	float Width()
	{
		return m_Window.ContentWidth();
	}

	//------------------------------------------------------------------------------------------------
	float Height()
	{
		return m_Window.ContentHeight();
	}

	//------------------------------------------------------------------------------------------------
	//! Builds the content; called again after resizing.
	void Build()
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Rebuilds the content from scratch.
	void Redraw()
	{
		m_aLists.Clear();
		ReleaseHandlers();
		ROOT_AE3_UI.Clear(GetContent());
		Build();
	}

	//------------------------------------------------------------------------------------------------
	void Update(float dt)
	{
	}

	//------------------------------------------------------------------------------------------------
	void OnClose()
	{
		m_aLists.Clear();
		ROOT_AE3_Client.Get().Forget(this);
	}

	//------------------------------------------------------------------------------------------------
	void Request(string cmd, notnull array<string> args)
	{
		ROOT_AE3_Client.Get().Request(cmd, args, this);
	}

	//------------------------------------------------------------------------------------------------
	void Request0(string cmd)
	{
		array<string> args = {};
		Request(cmd, args);
	}

	//------------------------------------------------------------------------------------------------
	void Request1(string cmd, string a)
	{
		array<string> args = {a};
		Request(cmd, args);
	}

	//------------------------------------------------------------------------------------------------
	void Request2(string cmd, string a, string b)
	{
		array<string> args = {a, b};
		Request(cmd, args);
	}

	//------------------------------------------------------------------------------------------------
	//! Error or info text in the window status bar.
	void Status(string text)
	{
		if (m_Window)
			m_Window.SetStatus(text);
	}

	//------------------------------------------------------------------------------------------------
	//! Default handling of failed replies.
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (!ok && !payload.IsEmpty())
			Status(payload[0]);
	}

	//------------------------------------------------------------------------------------------------
	void OnListSelect(int listId, int row)
	{
	}

	//------------------------------------------------------------------------------------------------
	void OnListActivate(int listId, int row)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Right click on a row; row -1 for the empty area.
	void OnListContext(int listId, int row)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Column header clicked.
	void OnListSort(int listId, int column)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Key pressed while this window is focused and no edit box is in write mode.
	bool OnKey(KeyCode key, bool ctrl)
	{
		return false;
	}

	// Building helpers ------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! True when this app's window has the focus.
	bool IsFocused()
	{
		return m_Desktop && m_Window && m_Desktop.GetFocusWindow() == m_Window;
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AE3_ListView List(int listId, float x, float y, float w, float h)
	{
		ROOT_AE3_ListView l = new ROOT_AE3_ListView(this, listId, GetContent(), x, y, w, h);
		m_aLists.Insert(l);
		return l;
	}

	//------------------------------------------------------------------------------------------------
	TextWidget Label(float x, float y, float w, float h, string text, int size = 15, bool mono = false, bool wrap = false)
	{
		return ROOT_AE3_UI.Text(GetContent(), x, y, w, h, text, size, ROOT_AE3_Theme.Text(), mono, wrap);
	}

	//------------------------------------------------------------------------------------------------
	ImageWidget Btn(float x, float y, float w, float h, string text, int id)
	{
		if (ROOT_AE3_Theme.IsTactical())
			return ROOT_AE3_UI.Button(GetContent(), x, y, w, h, text, id, this, ROOT_AE3_Theme.Button(), ROOT_AE3_Theme.Text(), 14);

		ROOT_AE3_UI.Rect(GetContent(), x, y, w, h, ROOT_AE3_Theme.Border());
		return ROOT_AE3_UI.Button(GetContent(), x + 1, y + 1, w - 2, h - 2, text, id, this, ROOT_AE3_Theme.Button(), ROOT_AE3_Theme.Text(), 14);
	}

	//------------------------------------------------------------------------------------------------
	//! Accent colored button for the main action.
	ImageWidget MainBtn(float x, float y, float w, float h, string text, int id)
	{
		return ROOT_AE3_UI.Button(GetContent(), x, y, w, h, text, id, this, ROOT_AE3_Theme.Accent(), ROOT_AE3_Theme.OnAccent(), 14);
	}

	//------------------------------------------------------------------------------------------------
	EditBoxWidget Field(float x, float y, float w, string text, int id, bool password = false)
	{
		ROOT_AE3_UI.Box(GetContent(), x, y, w, 26, ROOT_AE3_Theme.Field());
		EditBoxWidget e = ROOT_AE3_UI.Edit(GetContent(), x + 4, y + 2, w - 8, 22, text, id, this, password);
		if (e)
			e.SetColor(ROOT_AE3_Theme.Text());

		return e;
	}

	//------------------------------------------------------------------------------------------------
	//! Tool bar background across the window (height h) with its bottom line; returns the y below it.
	float ToolbarBg(float h = 44)
	{
		ROOT_AE3_UI.Rect(GetContent(), 0, 0, Width(), h, ROOT_AE3_Theme.Panel());
		ROOT_AE3_UI.Rect(GetContent(), 0, h, Width(), 1, ROOT_AE3_Theme.Border());
		return h + 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Panel with a border in the navigation pane colour.
	void Card(float x, float y, float w, float h)
	{
		ROOT_AE3_UI.Box(GetContent(), x, y, w, h, ROOT_AE3_Theme.Sidebar());
	}

	//------------------------------------------------------------------------------------------------
	//! Section heading with a rule below; returns the y below it.
	float Heading(float x, float y, float w, string title)
	{
		ROOT_AE3_UI.Text(GetContent(), x, y, w, 20, ROOT_AE3_Theme.Caption(title), 15, ROOT_AE3_Theme.Accent());
		ROOT_AE3_UI.Rect(GetContent(), x, y + 24, w, 1, ROOT_AE3_Theme.Border());
		return y + 32;
	}

	//------------------------------------------------------------------------------------------------
	//! Muted key and value text in one row; returns the y below it.
	float KeyValue(float x, float y, float w, string key, string value, float keyW = 120)
	{
		ROOT_AE3_UI.Text(GetContent(), x, y, keyW, 20, key, 14, ROOT_AE3_Theme.Muted());
		ROOT_AE3_UI.Text(GetContent(), x + keyW, y, w - keyW, 20, value, 14, ROOT_AE3_Theme.Text());
		return y + 24;
	}

	//------------------------------------------------------------------------------------------------
	//! Horizontal level bar 0-1.
	void Meter(float x, float y, float w, float h, float level, Color fill = null)
	{
		ref Color col = fill;
		if (!col)
			col = ROOT_AE3_Theme.Accent();

		ROOT_AE3_UI.Box(GetContent(), x, y, w, h, ROOT_AE3_Theme.Field());
		float k = Math.Clamp(level, 0, 1);
		if (k > 0)
			ROOT_AE3_UI.Rect(GetContent(), x + 1, y + 1, (w - 2) * k, h - 2, col);
	}

	//------------------------------------------------------------------------------------------------
	static string Arg(array<string> p, int i)
	{
		if (!p || i < 0 || i >= p.Count())
			return "";

		return p[i];
	}
}
