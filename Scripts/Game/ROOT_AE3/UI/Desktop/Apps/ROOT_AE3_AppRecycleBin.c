//! Recycle bin: restore, delete permanently, empty.
class ROOT_AE3_AppRecycleBin : ROOT_AE3_App
{
	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Recycle Bin";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "trash";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(600, 400, 0);
	}

	//------------------------------------------------------------------------------------------------
	override bool NeedsFileBrowsing()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected ROOT_AE3_ListView m_List;
	protected ref array<string> m_aEntries = {};

	override void Build()
	{
		float w = Width();
		float h = Height();
		m_List = List(1, 6, 6, w - 12, h - 44);
		array<string> headers = {"Name", "Original location"};
		array<float> widths = {(w - 12) * 0.35, (w - 12) * 0.65};
		m_List.SetColumns(headers, widths);
		Btn(6, h - 32, 100, 26, "Restore", 1);
		Btn(112, h - 32, 140, 26, "Delete forever", 2);
		Btn(258, h - 32, 120, 26, "Empty bin", 3);
		Request0("trash_list");
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (!ok)
		{
			Status(Arg(payload, 0));
			return;
		}

		if (cmd != "trash_list")
		{
			Request0("trash_list");
			return;
		}

		m_List.Clear();
		m_aEntries.Clear();
		for (int i = 0; i + 1 < payload.Count(); i += 2)
		{
			m_aEntries.Insert(payload[i]);
			array<string> row = {payload[i], payload[i + 1]};
			m_List.AddRow(row, "file");
		}

		m_List.Render();
		Status(string.Format("%1 items", m_aEntries.Count()));
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		string sel;
		if (m_List.m_iSelected >= 0 && m_List.m_iSelected < m_aEntries.Count())
			sel = m_aEntries[m_List.m_iSelected];

		switch (id)
		{
			case 1: Request1("fs_restore", sel); return true;
			case 2: Request1("fs_purge", sel); return true;
			case 3: Request0("fs_empty_trash"); return true;
		}

		return false;
	}
}
