//! Volumes: the local disk and the USB ports with mount, unmount and eject.
class ROOT_AE3_AppMyComputer : ROOT_AE3_App
{
	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "My Computer";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "system";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(560, 380, 0);
	}

	//------------------------------------------------------------------------------------------------
	override bool NeedsFileBrowsing()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected ROOT_AE3_ListView m_List;
	protected ref array<string> m_aIds = {};

	override void Build()
	{
		float w = Width();
		float h = Height();
		m_List = List(1, 6, 6, w - 12, h - 44);
		array<string> headers = {"Volume", "Device", "State", "Used"};
		array<float> widths = {(w - 12) * 0.25, (w - 12) * 0.3, (w - 12) * 0.25, (w - 12) * 0.2};
		m_List.SetColumns(headers, widths);
		Btn(6, h - 32, 90, 26, "Open", 1);
		Btn(102, h - 32, 90, 26, "Mount", 2);
		Btn(198, h - 32, 90, 26, "Unmount", 3);
		Btn(294, h - 32, 90, 26, "Eject", 4);
		Btn(390, h - 32, 90, 26, "Refresh", 5);
		Request0("vol_list");
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (!ok)
		{
			Status(Arg(payload, 0));
			return;
		}

		if (cmd != "vol_list")
		{
			Status(Arg(payload, 0));
			Request0("vol_list");
			return;
		}

		m_List.Clear();
		m_aIds.Clear();
		foreach (string p : payload)
		{
			array<string> e = ROOT_AE3_Codec.SplitSub(p);
			if (e.Count() < 5)
				continue;

			m_aIds.Insert(e[0]);
			array<string> row = {e[1], e[2], e[3], e[4]};
			m_List.AddRow(row);
		}

		m_List.Render();
	}

	//------------------------------------------------------------------------------------------------
	protected string Sel()
	{
		if (m_List.m_iSelected < 0 || m_List.m_iSelected >= m_aIds.Count())
			return "";

		return m_aIds[m_List.m_iSelected];
	}

	//------------------------------------------------------------------------------------------------
	override void OnListActivate(int listId, int row)
	{
		string id = Sel();
		if (id == "root")
			m_Desktop.OpenApp(ROOT_AE3_AppFiles, "/");
		else if (!id.IsEmpty())
			m_Desktop.OpenApp(ROOT_AE3_AppFiles, "/mnt/" + id);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		string sel = Sel();
		switch (id)
		{
			case 1: OnListActivate(1, m_List.m_iSelected); return true;
			case 2: Request1("vol_mount", sel); return true;
			case 3: Request1("vol_unmount", sel); return true;
			case 4: Request1("vol_eject", sel); return true;
			case 5: Request0("vol_list"); return true;
		}

		return false;
	}
}
