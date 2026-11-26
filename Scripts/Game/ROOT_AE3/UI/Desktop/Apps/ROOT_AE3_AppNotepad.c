//! Text editor: New, Open... and Save as... use the file dialog; Save writes the open file.
class ROOT_AE3_AppNotepad : ROOT_AE3_App
{
	protected static const int ID_NEW = 1;
	protected static const int ID_OPEN = 2;
	protected static const int ID_SAVE = 3;
	protected static const int ID_SAVE_AS = 4;
	protected static const int ID_TEXT = 5;

	protected MultilineEditBoxWidget m_wText;
	protected string m_sPath;
	protected string m_sText;
	protected bool m_bLoaded;

	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Notepad";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "notepad";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(720, 520, 0);
	}

	//------------------------------------------------------------------------------------------------
	override bool NeedsFileBrowsing()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Build()
	{
		if (!m_bLoaded && !m_sArg.IsEmpty())
			m_sPath = m_sArg;

		float w = Width();
		float h = Height();
		ROOT_AE3_UI.Rect(GetContent(), 0, 0, w, 40, ROOT_AE3_Theme.Bar());
		Tool(6, 76, "g_newfile", "New", ID_NEW);
		Tool(86, 90, "g_open", "Open...", ID_OPEN);
		Tool(180, 76, "g_check", "Save", ID_SAVE);
		Tool(260, 104, "g_copy", "Save as...", ID_SAVE_AS);
		ROOT_AE3_UI.Rect(GetContent(), 0, 40, w, 1, ROOT_AE3_Theme.Border());
		ROOT_AE3_UI.Rect(GetContent(), 0, 41, w, h - 41, ROOT_AE3_Theme.Field());
		m_wText = ROOT_AE3_UI.MultiEdit(GetContent(), 10, 48, w - 20, h - 56, m_sText, ID_TEXT, this);
		if (m_wText)
			m_wText.SetColor(ROOT_AE3_Theme.Text());

		UpdateTitle();
		if (!m_bLoaded && !m_sPath.IsEmpty())
		{
			m_bLoaded = true;
			Request1("fs_read", m_sPath);
		}

		m_bLoaded = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Keeps the typed text across rebuilds (resize).
	override void Redraw()
	{
		if (m_wText)
			m_sText = m_wText.GetText();

		super.Redraw();
	}

	//------------------------------------------------------------------------------------------------
	protected void Tool(float x, float w, string icon, string label, int id)
	{
		ROOT_AE3_UI.ItemButton(GetContent(), x, 6, w, 28, icon, label, id, this, 14, ROOT_AE3_Theme.Text(), ROOT_AE3_UI.Rgb(0, 0, 0, 0), ROOT_AE3_Theme.Hover());
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateTitle()
	{
		if (!m_Window)
			return;

		string name = "Untitled";
		if (!m_sPath.IsEmpty())
			name = ROOT_AE3_Filesystem.BaseName(m_sPath);

		m_Window.SetTitle(name + " - Notepad");
	}

	//------------------------------------------------------------------------------------------------
	protected string Folder()
	{
		if (m_sPath.IsEmpty())
			return ROOT_AE3_Filesystem.HomeOf(ROOT_AE3_Client.Get().m_sUser);

		return ROOT_AE3_Filesystem.Parent(m_sPath);
	}

	//------------------------------------------------------------------------------------------------
	protected void SaveAs()
	{
		if (!m_Desktop)
			return;

		string name = "new.txt";
		if (!m_sPath.IsEmpty())
			name = ROOT_AE3_Filesystem.BaseName(m_sPath);

		m_Desktop.ShowFilePicker(this, "save", "Save as", Folder(), true, name, "Save");
	}

	//------------------------------------------------------------------------------------------------
	override void OnDialog(string tag, bool ok, string value)
	{
		if (!ok || value.IsEmpty())
			return;

		if (tag == "open")
		{
			m_sPath = value;
			Request1("fs_read", m_sPath);
			return;
		}

		if (tag == "save")
		{
			m_sPath = value;
			UpdateTitle();
			Request2("fs_save", m_sPath, m_wText.GetText());
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

		if (cmd == "fs_read")
		{
			m_sText = Arg(payload, 0);
			ROOT_AE3_UI.SetEdit(m_wText, m_sText);
			UpdateTitle();
			Status("Opened " + m_sPath);
			return;
		}

		Status(Arg(payload, 0));
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiChange(Widget w, int id, bool finished)
	{
		if (id == ID_TEXT && m_wText)
			m_sText = m_wText.GetText();

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		switch (id)
		{
			case ID_NEW:
			{
				m_sPath = "";
				m_sText = "";
				ROOT_AE3_UI.SetEdit(m_wText, "");
				UpdateTitle();
				Status("New document");
				return true;
			}

			case ID_OPEN:
			{
				if (m_Desktop)
					m_Desktop.ShowFilePicker(this, "open", "Open", Folder(), false, "", "Open");

				return true;
			}

			case ID_SAVE:
			{
				if (m_sPath.IsEmpty())
					SaveAs();
				else
					Request2("fs_save", m_sPath, m_wText.GetText());

				return true;
			}

			case ID_SAVE_AS: SaveAs(); return true;
		}

		return false;
	}
}
