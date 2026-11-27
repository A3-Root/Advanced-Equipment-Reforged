//! SSH file transfer. Not connected: connect card (user@host, password, recent hosts). Connected: remote file
//! browser with back / forward / up / refresh, address bar, icons, preview pane, download and upload.
//! Keys: Enter open, Backspace back, F5 refresh. The listing survives window resizing.
class ROOT_AE3_AppSsh : ROOT_AE3_App
{
	protected static const int ID_TARGET = 1;
	protected static const int ID_PASS = 2;
	protected static const int ID_CONNECT = 3;
	protected static const int ID_DISCONNECT = 4;
	protected static const int ID_ADDR = 5;
	protected static const int ID_BACK = 6;
	protected static const int ID_FWD = 7;
	protected static const int ID_UP = 8;
	protected static const int ID_REFRESH = 9;
	protected static const int ID_VIEW = 10;
	protected static const int ID_PULL = 11;
	protected static const int ID_PUSH = 12;
	protected static const int ID_RECENT = 100;
	protected static const int M_OPEN = 1;
	protected static const int M_PULL = 2;
	protected static const int M_PUSH = 3;
	protected static const int M_REFRESH = 4;
	protected static const int M_UP = 5;
	protected static const float BAR1 = 44;
	protected static const float BAR2 = 40;

	protected static ref array<string> s_aRecent = {};	//!< Last connected targets (this client)

	protected EditBoxWidget m_wTarget;
	protected EditBoxWidget m_wPass;
	protected EditBoxWidget m_wAddr;
	protected ROOT_AE3_ListView m_List;
	protected TextWidget m_wPreview;
	protected TextWidget m_wPreviewTitle;
	protected bool m_bConnected;
	protected string m_sTarget;
	protected string m_sPath = "/";
	protected string m_sPreview;
	protected string m_sPreviewName;
	protected ref array<string> m_aBack = {};
	protected ref array<string> m_aFwd = {};
	protected ref array<string> m_aPaths = {};
	protected ref array<string> m_aTypes = {};
	protected ref array<string> m_aSizes = {};

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
		return Vector(860, 560, 0);
	}

	//------------------------------------------------------------------------------------------------
	override bool NeedsFileBrowsing()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Build()
	{
		if (m_bConnected)
			BuildBrowser();
		else
			BuildConnect();
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildConnect()
	{
		m_List = null;
		float w = Width();
		float h = Height();
		ROOT_AE3_UI.Rect(GetContent(), 0, 0, w, h, ROOT_AE3_Theme.Panel());
		float cw = Math.Min(460, w - 40);
		float ch = 300;
		float x = (w - cw) * 0.5;
		float y = Math.Max(16, (h - ch) * 0.4);
		ROOT_AE3_UI.Box(GetContent(), x, y, cw, ch, ROOT_AE3_Theme.Sidebar());
		ROOT_AE3_UI.Icon(GetContent(), x + 20, y + 20, 32, "remote");
		ROOT_AE3_UI.Text(GetContent(), x + 64, y + 18, cw - 84, 22, ROOT_AE3_Theme.Caption("Connect to a remote computer"), 17, ROOT_AE3_Theme.Text());
		ROOT_AE3_UI.Text(GetContent(), x + 64, y + 42, cw - 84, 18, "SSH login with a user of that computer", 13, ROOT_AE3_Theme.Muted());
		ROOT_AE3_UI.Text(GetContent(), x + 20, y + 78, cw - 40, 18, "User and host (user@address or user@hostname)", 13, ROOT_AE3_Theme.Muted());
		m_wTarget = Field(x + 20, y + 98, cw - 40, m_sTarget, ID_TARGET);
		ROOT_AE3_UI.Placeholder(m_wTarget, GetContent(), x + 26, y + 101, cw - 52, "root@192.168.1.10", ROOT_AE3_Theme.Muted());
		ROOT_AE3_UI.Text(GetContent(), x + 20, y + 134, cw - 40, 18, "Password", 13, ROOT_AE3_Theme.Muted());
		m_wPass = Field(x + 20, y + 154, cw - 40, "", ID_PASS, true);
		MainBtn(x + 20, y + 194, cw - 40, 32, "Connect", ID_CONNECT);
		if (s_aRecent.IsEmpty())
		{
			ROOT_AE3_UI.Text(GetContent(), x + 20, y + 242, cw - 40, 40, "Remote access must be on at the host (Network app or sshd on).", 12, ROOT_AE3_Theme.Muted(), false, true);
			return;
		}

		ROOT_AE3_UI.Text(GetContent(), x + 20, y + 238, cw - 40, 18, "Recent", 13, ROOT_AE3_Theme.Muted());
		float bx = x + 20;
		foreach (int i, string r : s_aRecent)
		{
			float bw = Math.Min(200, 24 + r.Length() * 7);
			if (bx + bw > x + cw - 20)
				break;

			ROOT_AE3_UI.ItemButton(GetContent(), bx, y + 258, bw, 28, "computer", r, ID_RECENT + i, this, 13, null, ROOT_AE3_Theme.Button());
			bx += bw + 6;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildBrowser()
	{
		float w = Width();
		float h = Height();
		ROOT_AE3_UI.Rect(GetContent(), 0, 0, w, BAR1 + BAR2, ROOT_AE3_Theme.Panel());
		ROOT_AE3_UI.IconButton(GetContent(), 6, 6, 34, 32, "g_back", ID_BACK, this, 16);
		ROOT_AE3_UI.IconButton(GetContent(), 42, 6, 34, 32, "g_forward", ID_FWD, this, 16);
		ROOT_AE3_UI.IconButton(GetContent(), 78, 6, 34, 32, "g_up", ID_UP, this, 16);
		ROOT_AE3_UI.IconButton(GetContent(), 114, 6, 34, 32, "g_refresh", ID_REFRESH, this, 16);
		float rightW = 150;
		float aw = w - 158 - rightW - 12;
		ROOT_AE3_UI.Box(GetContent(), 154, 8, aw, 28, ROOT_AE3_Theme.Field());
		ROOT_AE3_UI.Icon(GetContent(), 162, 14, 16, "computer");
		m_wAddr = ROOT_AE3_UI.Edit(GetContent(), 184, 10, aw - 36, 24, m_sPath, ID_ADDR, this, false, true);
		if (m_wAddr)
			m_wAddr.SetColor(ROOT_AE3_Theme.Text());

		Btn(w - rightW - 6, 8, rightW, 28, "Disconnect", ID_DISCONNECT);

		array<string> labels = {"Open", "Download to...", "Upload here..."};
		array<string> icons = {"g_open", "g_copy", "g_paste"};
		array<int> ids = {ID_VIEW, ID_PULL, ID_PUSH};
		float x = 6;
		foreach (int i, string l : labels)
		{
			float bw = 44 + l.Length() * 7;
			ROOT_AE3_UI.ItemButton(GetContent(), x, BAR1 + 2, bw, 34, icons[i], l, ids[i], this, 14);
			x += bw + 4;
		}

		ROOT_AE3_UI.Text(GetContent(), x + 10, BAR1 + 11, w - x - 20, 18, "Connected as " + m_sTarget, 13, ROOT_AE3_Theme.Muted(), false, false, 2);
		ROOT_AE3_UI.Rect(GetContent(), 0, BAR1 + BAR2, w, 1, ROOT_AE3_Theme.Border());

		float top = BAR1 + BAR2 + 1;
		float lw = Math.Floor(w * 0.6);
		m_List = List(1, 0, top, lw, h - top);
		array<string> headers = {"Name", "Type", "Size"};
		array<float> widths = {lw * 0.6, lw * 0.2, lw * 0.2};
		m_List.SetColumns(headers, widths);
		m_List.m_sEmpty = "This folder is empty.";
		ROOT_AE3_UI.Rect(GetContent(), lw, top, 1, h - top, ROOT_AE3_Theme.Border());
		ROOT_AE3_UI.Rect(GetContent(), lw + 1, top, w - lw - 1, h - top, ROOT_AE3_Theme.Sidebar());
		m_wPreviewTitle = ROOT_AE3_UI.Text(GetContent(), lw + 14, top + 10, w - lw - 28, 18, "", 14, ROOT_AE3_Theme.Text());
		m_wPreview = ROOT_AE3_UI.Text(GetContent(), lw + 14, top + 36, w - lw - 28, h - top - 46, "", 13, ROOT_AE3_Theme.Text(), true, true);
		ShowPreview();
		ShowEntries();
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowPreview()
	{
		if (!m_wPreview)
			return;

		if (m_sPreviewName.IsEmpty())
		{
			m_wPreviewTitle.SetText("Preview");
			m_wPreview.SetText("Select a file and press Open to read it here.");
			m_wPreview.SetColor(ROOT_AE3_Theme.Muted());
			return;
		}

		m_wPreviewTitle.SetText(m_sPreviewName);
		m_wPreview.SetText(m_sPreview);
		m_wPreview.SetColor(ROOT_AE3_Theme.Text());
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowEntries()
	{
		if (!m_List)
			return;

		m_List.Clear();
		foreach (int i, string p : m_aPaths)
		{
			string name = ROOT_AE3_Filesystem.BaseName(p);
			string type = "File";
			string size = m_aSizes[i];
			if (m_aTypes[i] == "dir")
			{
				type = "Folder";
				size = "";
			}

			array<string> row = {name, type, size};
			m_List.AddRow(row, ROOT_AE3_Icons.ForFile(m_aTypes[i], false, false, false, name));
		}

		m_List.Render();
		Status(string.Format("%1 - %2 items", m_sTarget, m_aPaths.Count()));
	}

	//------------------------------------------------------------------------------------------------
	protected void Connect(string target)
	{
		string t = target.Trim();
		if (t.IsEmpty())
		{
			Status("Enter user@host.");
			return;
		}

		string pass;
		if (m_wPass)
			pass = m_wPass.GetText();

		m_sTarget = t;
		Status("Connecting to " + t + "...");
		Request2("ssh_connect", t, pass);
	}

	//------------------------------------------------------------------------------------------------
	//! push: the current folder goes to the back history.
	protected void Navigate(string path, bool push = true)
	{
		if (push && path != m_sPath)
		{
			m_aBack.Insert(m_sPath);
			m_aFwd.Clear();
		}

		Request1("ssh_ls", path);
	}

	//------------------------------------------------------------------------------------------------
	protected void GoBack()
	{
		if (m_aBack.IsEmpty())
			return;

		m_aFwd.Insert(m_sPath);
		string p = m_aBack[m_aBack.Count() - 1];
		m_aBack.Remove(m_aBack.Count() - 1);
		Navigate(p, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void GoForward()
	{
		if (m_aFwd.IsEmpty())
			return;

		m_aBack.Insert(m_sPath);
		string p = m_aFwd[m_aFwd.Count() - 1];
		m_aFwd.Remove(m_aFwd.Count() - 1);
		Navigate(p, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void GoUp()
	{
		if (m_sPath == "/" || m_sPath.IsEmpty())
			return;

		int cut = m_sPath.LastIndexOf("/");
		string parent = "/";
		if (cut > 0)
			parent = m_sPath.Substring(0, cut);

		Navigate(parent);
	}

	//------------------------------------------------------------------------------------------------
	protected int Selected()
	{
		if (!m_List)
			return -1;

		int i = m_List.m_iSelected;
		if (i < 0 || i >= m_aPaths.Count())
			return -1;

		return i;
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenSelected()
	{
		int i = Selected();
		if (i < 0)
		{
			Status("Select a file or folder first.");
			return;
		}

		if (m_aTypes[i] == "dir")
		{
			Navigate(m_aPaths[i]);
			return;
		}

		m_sPreviewName = ROOT_AE3_Filesystem.BaseName(m_aPaths[i]);
		Request1("ssh_read", m_aPaths[i]);
	}

	//------------------------------------------------------------------------------------------------
	protected void Pull()
	{
		int i = Selected();
		if (i < 0 || m_aTypes[i] == "dir")
		{
			Status("Select a remote file first.");
			return;
		}

		if (m_Desktop)
			m_Desktop.ShowFilePicker(this, "pull", "Download to folder", "", false, "", "Download here", true);
	}

	//------------------------------------------------------------------------------------------------
	protected void Push()
	{
		if (m_Desktop)
			m_Desktop.ShowFilePicker(this, "push", "Upload file to " + m_sPath, "", false, "", "Upload");
	}

	//------------------------------------------------------------------------------------------------
	protected void Disconnect()
	{
		Request0("ssh_close");
		m_bConnected = false;
		m_aPaths.Clear();
		m_aTypes.Clear();
		m_aSizes.Clear();
		m_aBack.Clear();
		m_aFwd.Clear();
		m_sPath = "/";
		m_sPreviewName = "";
		Redraw();
		Status("Disconnected.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnClose()
	{
		if (m_bConnected)
			Request0("ssh_close");

		super.OnClose();
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (!ok)
		{
			Status(Arg(payload, 0));
			if (cmd == "ssh_ls" && m_bConnected && !m_aBack.IsEmpty() && m_aBack[m_aBack.Count() - 1] == m_sPath)
				m_aBack.Remove(m_aBack.Count() - 1);

			return;
		}

		switch (cmd)
		{
			case "ssh_connect":
			{
				m_bConnected = true;
				s_aRecent.RemoveItem(m_sTarget);
				s_aRecent.InsertAt(m_sTarget, 0);
				if (s_aRecent.Count() > 5)
					s_aRecent.Remove(5);

				m_sPath = "/";
				Redraw();
				Status(Arg(payload, 0));
				Request1("ssh_ls", "/");
				return;
			}

			case "ssh_ls":
			{
				m_sPath = Arg(payload, 0);
				ROOT_AE3_UI.SetEdit(m_wAddr, m_sPath);
				m_aPaths.Clear();
				m_aTypes.Clear();
				m_aSizes.Clear();
				for (int i = 1; i < payload.Count(); i++)
				{
					array<string> e = ROOT_AE3_Codec.SplitSub(payload[i]);
					if (e.Count() < 3)
						continue;

					// Folders first, then by name.
					int at = m_aPaths.Count();
					if (e[1] == "dir")
					{
						at = 0;
						while (at < m_aTypes.Count() && m_aTypes[at] == "dir")
						{
							at++;
						}
					}

					m_aPaths.InsertAt(e[0], at);
					m_aTypes.InsertAt(e[1], at);
					m_aSizes.InsertAt(e[2], at);
				}

				ShowEntries();
				return;
			}

			case "ssh_read":
			{
				m_sPreview = Arg(payload, 0);
				ShowPreview();
				return;
			}
		}

		Status(Arg(payload, 0));
	}

	//------------------------------------------------------------------------------------------------
	override void OnListActivate(int listId, int row)
	{
		OpenSelected();
	}

	//------------------------------------------------------------------------------------------------
	override void OnListContext(int listId, int row)
	{
		if (!m_Desktop)
			return;

		if (row < 0)
		{
			array<string> bgLabels = {"Refresh", "Up one level", "-", "Upload here..."};
			array<int> bgIds = {M_REFRESH, M_UP, 0, M_PUSH};
			array<string> bgIcons = {"g_refresh", "g_up", "", "g_paste"};
			m_Desktop.ShowMenu(this, bgLabels, bgIds, bgIcons);
			return;
		}

		array<string> labels = {"Open", "Download to...", "-", "Upload here...", "Refresh"};
		array<int> ids = {M_OPEN, M_PULL, 0, M_PUSH, M_REFRESH};
		array<string> icons = {"g_open", "g_copy", "", "g_paste", "g_refresh"};
		m_Desktop.ShowMenu(this, labels, ids, icons);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuItem(int id)
	{
		switch (id)
		{
			case M_OPEN: OpenSelected(); break;
			case M_PULL: Pull(); break;
			case M_PUSH: Push(); break;
			case M_REFRESH: Navigate(m_sPath, false); break;
			case M_UP: GoUp(); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool OnKey(KeyCode key, bool ctrl)
	{
		if (!m_bConnected || ctrl)
			return false;

		switch (key)
		{
			case KeyCode.KC_RETURN:
			case KeyCode.KC_NUMPADENTER: OpenSelected(); return true;
			case KeyCode.KC_BACK: GoBack(); return true;
			case KeyCode.KC_F5: Navigate(m_sPath, false); return true;
			case KeyCode.KC_UP: m_List.Move(-1); return true;
			case KeyCode.KC_DOWN: m_List.Move(1); return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiChange(Widget w, int id, bool finished)
	{
		if (id == ID_TARGET && m_wTarget)
			m_sTarget = m_wTarget.GetText();

		if (!finished)
			return false;

		if (id == ID_ADDR && m_wAddr)
		{
			string p = m_wAddr.GetText().Trim();
			GetGame().GetWorkspace().SetFocusedWidget(null);
			if (!p.IsEmpty())
				Navigate(p);

			return true;
		}

		if (id == ID_PASS || id == ID_TARGET)
		{
			Connect(m_sTarget);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (id >= ID_RECENT && id < ID_RECENT + s_aRecent.Count())
		{
			m_sTarget = s_aRecent[id - ID_RECENT];
			ROOT_AE3_UI.SetEdit(m_wTarget, m_sTarget);
			if (m_wPass)
			{
				GetGame().GetWorkspace().SetFocusedWidget(m_wPass);
				m_wPass.ActivateWriteMode();
			}

			return true;
		}

		switch (id)
		{
			case ID_CONNECT: Connect(m_sTarget); return true;
			case ID_DISCONNECT: Disconnect(); return true;
			case ID_BACK: GoBack(); return true;
			case ID_FWD: GoForward(); return true;
			case ID_UP: GoUp(); return true;
			case ID_REFRESH: Navigate(m_sPath, false); return true;
			case ID_VIEW: OpenSelected(); return true;
			case ID_PULL: Pull(); return true;
			case ID_PUSH: Push(); return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnDialog(string tag, bool ok, string value)
	{
		if (!ok || value.IsEmpty())
			return;

		int i = Selected();
		if (tag == "pull" && i >= 0)
			Request2("ssh_pull", m_aPaths[i], value);
		else if (tag == "push")
			Request2("ssh_push", value, m_sPath);
	}
}
