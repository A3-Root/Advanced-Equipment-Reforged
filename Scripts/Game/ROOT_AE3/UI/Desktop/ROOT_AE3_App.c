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

	// Building helpers ------------------------------------------------------------------------------

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
		return ROOT_AE3_UI.Button(GetContent(), x, y, w, h, text, id, this, ROOT_AE3_Theme.Button(), ROOT_AE3_Theme.Text());
	}

	//------------------------------------------------------------------------------------------------
	EditBoxWidget Field(float x, float y, float w, string text, int id, bool password = false)
	{
		ROOT_AE3_UI.Rect(GetContent(), x, y, w, 26, ROOT_AE3_Theme.Field());
		EditBoxWidget e = ROOT_AE3_UI.Edit(GetContent(), x + 4, y + 2, w - 8, 22, text, id, this, password);
		if (e)
			e.SetColor(ROOT_AE3_Theme.Text());

		return e;
	}

	//------------------------------------------------------------------------------------------------
	static string Arg(array<string> p, int i)
	{
		if (!p || i < 0 || i >= p.Count())
			return "";

		return p[i];
	}
}
