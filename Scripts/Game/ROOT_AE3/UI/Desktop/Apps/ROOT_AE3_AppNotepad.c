//! Text editor: open, edit and save files.
class ROOT_AE3_AppNotepad : ROOT_AE3_App
{
	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Text Editor";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "details";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(700, 500, 0);
	}

	//------------------------------------------------------------------------------------------------
	override bool NeedsFileBrowsing()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected EditBoxWidget m_wPath;
	protected MultilineEditBoxWidget m_wText;
	protected string m_sPath;

	override void Build()
	{
		if (!m_sArg.IsEmpty())
			m_sPath = m_sArg;

		float w = Width();
		float h = Height();
		m_wPath = Field(6, 6, w - 260, m_sPath, 1);
		Btn(w - 248, 6, 76, 26, "Open", 2);
		Btn(w - 166, 6, 76, 26, "Save", 3);
		Btn(w - 84, 6, 78, 26, "New", 4);
		ROOT_AE3_UI.Rect(GetContent(), 6, 38, w - 12, h - 44, ROOT_AE3_Theme.Field());
		m_wText = ROOT_AE3_UI.MultiEdit(GetContent(), 10, 42, w - 20, h - 52, "", 5, this);
		if (m_wText)
			m_wText.SetColor(ROOT_AE3_Theme.Text());

		if (!m_sPath.IsEmpty())
			Request1("fs_read", m_sPath);
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
			m_wText.SetText(Arg(payload, 0));
			Status("Opened " + m_sPath);
			return;
		}

		Status(Arg(payload, 0));
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		switch (id)
		{
			case 2:
			{
				m_sPath = m_wPath.GetText();
				Request1("fs_read", m_sPath);
				return true;
			}

			case 3:
			{
				m_sPath = m_wPath.GetText();
				if (m_sPath.IsEmpty())
				{
					Status("Enter a path first.");
					return true;
				}

				Request2("fs_save", m_sPath, m_wText.GetText());
				return true;
			}

			case 4:
			{
				m_sPath = "";
				m_wPath.SetText("~/Documents/new.txt");
				m_wText.SetText("");
				return true;
			}
		}

		return false;
	}
}
