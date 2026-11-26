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
		Btn(6, h - 34, 90, 28, "View", 8);
		MainBtn(102, h - 34, 130, 28, "Download...", 9);
		Btn(238, h - 34, 130, 28, "Upload...", 10);
		ROOT_AE3_UI.Text(GetContent(), 378, h - 29, w - 384, 20, "Download: selected remote file to a local folder. Upload: a local file to the listed remote folder.", 12, ROOT_AE3_Theme.Muted(), false, true);
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
			case 9:
			{
				if (SelPath().IsEmpty())
				{
					Status("Select a remote file first.");
					return true;
				}

				if (m_Desktop)
					m_Desktop.ShowFilePicker(this, "pull", "Download to folder", "", false, "", "Download here", true);

				return true;
			}

			case 10:
			{
				if (m_Desktop)
					m_Desktop.ShowFilePicker(this, "push", "Upload file", "", false, "", "Upload");

				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnDialog(string tag, bool ok, string value)
	{
		if (!ok || value.IsEmpty())
			return;

		if (tag == "pull")
			Request2("ssh_pull", SelPath(), value);
		else if (tag == "push")
			Request2("ssh_push", value, m_wPath.GetText());
	}
}
