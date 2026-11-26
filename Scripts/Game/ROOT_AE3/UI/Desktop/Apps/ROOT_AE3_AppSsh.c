//! SSH file transfer: connect to a remote computer, browse, pull and push files.
class ROOT_AE3_AppSsh : ROOT_AE3_App
{
	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Remote Access";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "remote";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(780, 520, 0);
	}

	//------------------------------------------------------------------------------------------------
	override bool NeedsFileBrowsing()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected EditBoxWidget m_wTarget;
	protected EditBoxWidget m_wPass;
	protected EditBoxWidget m_wPath;
	protected EditBoxWidget m_wLocal;
	protected ROOT_AE3_ListView m_List;
	protected ref array<string> m_aPaths = {};
	protected ref array<bool> m_aDirs = {};
	protected bool m_bConnected;
	protected TextWidget m_wView;

	override void Build()
	{
		float w = Width();
		float h = Height();
		Label(8, 10, 90, 22, "user@ip");
		m_wTarget = Field(90, 6, 220, "", 1);
		Label(318, 10, 70, 22, "Password");
		m_wPass = Field(388, 6, 150, "", 2, true);
		Btn(544, 6, 90, 26, "Connect", 3);
		Btn(638, 6, w - 644, 26, "Close", 4);
		m_wPath = Field(6, 38, w - 100, "/", 5);
		Btn(w - 88, 38, 82, 26, "List", 6);
		m_List = List(1, 6, 70, w * 0.55, h - 110);
		array<string> headers = {"Remote", "Size"};
		array<float> widths = {w * 0.55 * 0.7, w * 0.55 * 0.3};
		m_List.SetColumns(headers, widths);
		ROOT_AE3_UI.Rect(GetContent(), w * 0.55 + 12, 70, w * 0.45 - 18, h - 110, ROOT_AE3_Theme.Field());
		m_wView = Label(w * 0.55 + 20, 76, w * 0.45 - 34, h - 122, "", 13, true, true);
		Label(8, h - 32, 70, 22, "Local");
		m_wLocal = Field(70, h - 34, w - 330, "~/Downloads", 7);
		Btn(w - 254, h - 34, 80, 26, "Read", 8);
		Btn(w - 170, h - 34, 80, 26, "Pull", 9);
		Btn(w - 86, h - 34, 80, 26, "Push", 10);
	}

	//------------------------------------------------------------------------------------------------
	protected string SelPath()
	{
		int i = m_List.m_iSelected;
		if (i < 0 || i >= m_aPaths.Count())
			return "";

		return m_aPaths[i];
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (!ok)
		{
			Status(Arg(payload, 0));
			return;
		}

		switch (cmd)
		{
			case "ssh_connect":
			{
				m_bConnected = true;
				Status(Arg(payload, 0));
				Request1("ssh_ls", m_wPath.GetText());
				return;
			}

			case "ssh_ls":
			{
				ROOT_AE3_UI.SetEdit(m_wPath, Arg(payload, 0));
				m_List.Clear();
				m_aPaths.Clear();
				m_aDirs.Clear();
				for (int i = 1; i < payload.Count(); i++)
				{
					array<string> e = ROOT_AE3_Codec.SplitSub(payload[i]);
					if (e.Count() < 3)
						continue;

					m_aPaths.Insert(e[0]);
					m_aDirs.Insert(e[1] == "dir");
					string name = ROOT_AE3_Filesystem.BaseName(e[0]);
					if (e[1] == "dir")
						name += "/";

					array<string> row = {name, e[2]};
					m_List.AddRow(row);
				}

				m_List.Render();
				return;
			}

			case "ssh_read":
			{
				m_wView.SetText(Arg(payload, 0));
				return;
			}
		}

		Status(Arg(payload, 0));
	}

	//------------------------------------------------------------------------------------------------
	override void OnListActivate(int listId, int row)
	{
		if (row >= 0 && row < m_aPaths.Count() && m_aDirs[row])
			Request1("ssh_ls", m_aPaths[row]);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		switch (id)
		{
			case 3: Request2("ssh_connect", m_wTarget.GetText(), m_wPass.GetText()); return true;
			case 4: Request0("ssh_close"); m_bConnected = false; m_List.Clear(); m_List.Render(); return true;
			case 6: Request1("ssh_ls", m_wPath.GetText()); return true;
			case 8: Request1("ssh_read", SelPath()); return true;
			case 9: Request2("ssh_pull", SelPath(), m_wLocal.GetText()); return true;
			case 10: Request2("ssh_push", m_wLocal.GetText(), m_wPath.GetText()); return true;
		}

		return false;
	}
}
