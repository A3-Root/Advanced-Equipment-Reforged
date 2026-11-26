//! Email: Inbox and Sent tabs, search by sender or subject, reading pane, compose, reply, delete,
//! own addresses (create and delete).
class ROOT_AE3_AppMail : ROOT_AE3_App
{
	protected static const int ID_REFRESH = 1;
	protected static const int ID_INBOX = 2;
	protected static const int ID_SENT = 3;
	protected static const int ID_COMPOSE = 4;
	protected static const int ID_ADDRESSES = 5;
	protected static const int ID_SEARCH = 6;
	protected static const int ID_SEND = 7;
	protected static const int ID_CANCEL = 8;
	protected static const int ID_DELETE = 9;
	protected static const int ID_REPLY = 10;
	protected static const int ID_FROM = 11;
	protected static const int ID_ADDR_NEW = 12;
	protected static const int ID_ADDR_CREATE = 13;
	protected static const int ID_ADDR_DEL = 100;
	protected static const int MI_OPEN = 1;
	protected static const int MI_REPLY = 2;
	protected static const int MI_DELETE = 3;
	protected static const int MI_FROM = 50;
	protected static const float TOP = 44;

	protected ROOT_AE3_ListView m_List;
	protected ref array<ref array<string>> m_aMails = {};		//!< file, who, subject, date
	protected ref array<int> m_aShownIdx = {};
	protected ref array<string> m_aOpen = {};					//!< from, to, subject, date, body
	protected ref array<string> m_aAddresses = {};
	protected string m_sOpenFile;
	protected string m_sBox = "inbox";
	protected string m_sMode = "read";		//!< read, compose, addresses
	protected string m_sSearch;
	protected string m_sFrom;
	protected string m_sTo;
	protected string m_sSubject;
	protected string m_sBody;
	protected EditBoxWidget m_wSearch;
	protected EditBoxWidget m_wTo;
	protected EditBoxWidget m_wSubject;
	protected MultilineEditBoxWidget m_wBody;
	protected EditBoxWidget m_wNewAddr;
	protected float m_fListW;
	protected bool m_bAddrLoaded;

	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Email";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "mail";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(900, 560, 0);
	}

	//------------------------------------------------------------------------------------------------
	override void Build()
	{
		float w = Width();
		float h = Height();
		ROOT_AE3_UI.Rect(GetContent(), 0, 0, w, TOP, ROOT_AE3_Theme.Bar());
		ROOT_AE3_UI.Box(GetContent(), 6, 8, 30, 28, ROOT_AE3_Theme.Button(), ROOT_AE3_Theme.Border());
		ROOT_AE3_UI.IconButton(GetContent(), 7, 9, 28, 26, "g_refresh", ID_REFRESH, this, 14, ROOT_AE3_Theme.Text(), ROOT_AE3_Theme.Button(), ROOT_AE3_Theme.Hover());
		Tab(42, "Inbox", ID_INBOX, m_sBox == "inbox");
		Tab(124, "Sent", ID_SENT, m_sBox == "sent");
		MainBtn(212, 8, 100, 28, "Compose", ID_COMPOSE);
		Btn(318, 8, 100, 28, "Addresses", ID_ADDRESSES);
		float sx = 430;
		ROOT_AE3_UI.Box(GetContent(), sx, 8, w - sx - 8, 28, ROOT_AE3_Theme.Field(), ROOT_AE3_Theme.Border());
		ROOT_AE3_UI.Icon(GetContent(), sx + 8, 14, 16, "g_search", ROOT_AE3_Theme.Muted());
		m_wSearch = ROOT_AE3_UI.Edit(GetContent(), sx + 30, 10, w - sx - 46, 24, m_sSearch, ID_SEARCH, this);
		if (m_wSearch)
		{
			m_wSearch.SetColor(ROOT_AE3_Theme.Text());
			ROOT_AE3_UI.Placeholder(m_wSearch, GetContent(), sx + 34, 14, w - sx - 50, "Search from / subject", ROOT_AE3_Theme.Muted());
		}

		ROOT_AE3_UI.Rect(GetContent(), 0, TOP, w, 1, ROOT_AE3_Theme.Border());
		m_fListW = Math.Floor(w * 0.4);
		m_List = List(1, 0, TOP + 1, m_fListW, h - TOP - 1);
		array<string> headers = {"Subject", "From", "Received"};
		if (m_sBox == "sent")
		{
			headers[1] = "To";
			headers[2] = "Sent";
		}

		array<float> widths = {m_fListW * 0.44, m_fListW * 0.31, m_fListW * 0.25};
		m_List.SetColumns(headers, widths);
		m_List.m_sEmpty = "No mail";
		FillList();
		ROOT_AE3_UI.Rect(GetContent(), m_fListW, TOP + 1, 1, h - TOP - 1, ROOT_AE3_Theme.Border());
		if (m_sMode == "compose")
			BuildCompose();
		else if (m_sMode == "addresses")
			BuildAddresses();
		else
			BuildReader();

		if (m_aMails.IsEmpty())
			List0();
	}

	//------------------------------------------------------------------------------------------------
	protected void List0()
	{
		Request1("mail_list", m_sBox);
	}

	//------------------------------------------------------------------------------------------------
	protected void Tab(float x, string label, int id, bool active)
	{
		if (active)
			MainBtn(x, 8, 76, 28, label, id);
		else
			Btn(x, 8, 76, 28, label, id);
	}

	//------------------------------------------------------------------------------------------------
	protected void FillList()
	{
		m_List.Clear();
		m_aShownIdx.Clear();
		string q = m_sSearch;
		q.ToLower();
		foreach (int i, array<string> m : m_aMails)
		{
			if (!q.IsEmpty())
			{
				string who = m[1];
				string subj = m[2];
				who.ToLower();
				subj.ToLower();
				if (!who.Contains(q) && !subj.Contains(q))
					continue;
			}

			string subject = m[2];
			if (subject.IsEmpty())
				subject = "(no subject)";

			array<string> row = {subject, m[1], m[3]};
			m_List.AddRow(row, "mail");
			if (m[0] == m_sOpenFile)
				m_List.m_iSelected = m_aShownIdx.Count();

			m_aShownIdx.Insert(i);
		}

		m_List.Render();
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildReader()
	{
		float x = m_fListW + 18;
		float w = Width() - x - 16;
		float h = Height();
		if (m_aOpen.Count() < 5)
		{
			ROOT_AE3_UI.Icon(GetContent(), x + (w - 56) * 0.5, h * 0.4 - 40, 56, "mail");
			ROOT_AE3_UI.Text(GetContent(), x, h * 0.4 + 24, w, 22, "Select a message.", 15, ROOT_AE3_Theme.Muted(), false, false, 1);
			return;
		}

		float y = TOP + 14;
		string subject = m_aOpen[2];
		if (subject.IsEmpty())
			subject = "(no subject)";

		Label(x, y, w - 190, 28, subject, 20);
		if (m_sBox == "inbox")
			Btn(x + w - 184, y, 88, 28, "Reply", ID_REPLY);

		Btn(x + w - 90, y, 90, 28, "Delete", ID_DELETE);
		y += 36;
		ROOT_AE3_UI.Text(GetContent(), x, y, w, 18, "From: " + m_aOpen[0], 13, ROOT_AE3_Theme.Muted());
		y += 18;
		if (!m_aOpen[1].IsEmpty())
		{
			ROOT_AE3_UI.Text(GetContent(), x, y, w, 18, "To: " + m_aOpen[1], 13, ROOT_AE3_Theme.Muted());
			y += 18;
		}

		if (!m_aOpen[3].IsEmpty())
		{
			ROOT_AE3_UI.Text(GetContent(), x, y, w, 18, m_aOpen[3], 13, ROOT_AE3_Theme.Muted());
			y += 18;
		}

		y += 8;
		ROOT_AE3_UI.Rect(GetContent(), x, y, w, 1, ROOT_AE3_Theme.Border());
		y += 12;
		ROOT_AE3_UI.Text(GetContent(), x, y, w, h - y - 10, m_aOpen[4], 15, ROOT_AE3_Theme.Text(), false, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildCompose()
	{
		float x = m_fListW + 18;
		float w = Width() - x - 16;
		float h = Height();
		float y = TOP + 12;
		Label(x, y, w, 26, "New message", 18);
		y += 36;
		if (m_sFrom.IsEmpty() && !m_aAddresses.IsEmpty())
			m_sFrom = m_aAddresses[0];

		ROOT_AE3_UI.Text(GetContent(), x, y + 5, 70, 20, "From", 14, ROOT_AE3_Theme.Muted());
		string from = m_sFrom;
		if (from.IsEmpty())
			from = "(create an address first)";

		ROOT_AE3_UI.Box(GetContent(), x + 70, y, w - 70, 28, ROOT_AE3_Theme.Button(), ROOT_AE3_Theme.Border());
		ROOT_AE3_UI.ItemButton(GetContent(), x + 71, y + 1, w - 72, 26, "g_chevron_down", from, ID_FROM, this, 14, ROOT_AE3_Theme.Text(), ROOT_AE3_Theme.Button(), ROOT_AE3_Theme.Hover());
		y += 36;
		ROOT_AE3_UI.Text(GetContent(), x, y + 5, 70, 20, "To", 14, ROOT_AE3_Theme.Muted());
		m_wTo = Field(x + 70, y, w - 70, m_sTo, 20);
		ROOT_AE3_UI.Placeholder(m_wTo, GetContent(), x + 78, y + 4, w - 86, "name@domain", ROOT_AE3_Theme.Muted());
		y += 34;
		ROOT_AE3_UI.Text(GetContent(), x, y + 5, 70, 20, "Subject", 14, ROOT_AE3_Theme.Muted());
		m_wSubject = Field(x + 70, y, w - 70, m_sSubject, 21);
		y += 36;
		ROOT_AE3_UI.Box(GetContent(), x, y, w, h - y - 52, ROOT_AE3_Theme.Field(), ROOT_AE3_Theme.Border());
		m_wBody = ROOT_AE3_UI.MultiEdit(GetContent(), x + 6, y + 4, w - 12, h - y - 60, m_sBody, 22, this);
		if (m_wBody)
			m_wBody.SetColor(ROOT_AE3_Theme.Text());

		MainBtn(x, h - 40, 100, 30, "Send", ID_SEND);
		Btn(x + 108, h - 40, 100, 30, "Discard", ID_CANCEL);
		if (!m_bAddrLoaded)
		{
			m_bAddrLoaded = true;
			Request0("addr_list");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildAddresses()
	{
		float x = m_fListW + 18;
		float w = Width() - x - 16;
		float y = TOP + 12;
		Label(x, y, w, 26, "My addresses", 18);
		y += 36;
		m_wNewAddr = Field(x, y, w - 110, "", ID_ADDR_NEW);
		ROOT_AE3_UI.Placeholder(m_wNewAddr, GetContent(), x + 8, y + 4, w - 126, "name@domain.local", ROOT_AE3_Theme.Muted());
		MainBtn(x + w - 100, y - 1, 100, 28, "Create", ID_ADDR_CREATE);
		y += 40;
		if (m_aAddresses.IsEmpty())
			ROOT_AE3_UI.Text(GetContent(), x, y, w, 20, "No addresses", 14, ROOT_AE3_Theme.Muted());

		foreach (int i, string a : m_aAddresses)
		{
			ROOT_AE3_UI.Box(GetContent(), x, y, w, 34, ROOT_AE3_Theme.Field(), ROOT_AE3_Theme.Border());
			ROOT_AE3_UI.Icon(GetContent(), x + 10, y + 9, 16, "mail");
			ROOT_AE3_UI.Text(GetContent(), x + 34, y + 8, w - 140, 20, a, 14, ROOT_AE3_Theme.Text());
			Btn(x + w - 90, y + 3, 84, 28, "Delete", ID_ADDR_DEL + i);
			y += 40;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Keeps compose fields across rebuilds.
	protected void KeepCompose()
	{
		if (m_sMode != "compose")
			return;

		if (m_wTo)
			m_sTo = m_wTo.GetText();

		if (m_wSubject)
			m_sSubject = m_wSubject.GetText();

		if (m_wBody)
			m_sBody = m_wBody.GetText();
	}

	//------------------------------------------------------------------------------------------------
	override void Redraw()
	{
		KeepCompose();
		super.Redraw();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetMode(string mode)
	{
		KeepCompose();
		m_sMode = mode;
		Redraw();
	}

	//------------------------------------------------------------------------------------------------
	protected void Compose(string to, string subject, string body)
	{
		m_sTo = to;
		m_sSubject = subject;
		m_sBody = body;
		m_sMode = "compose";
		super.Redraw();
	}

	//------------------------------------------------------------------------------------------------
	protected void Open(int shown)
	{
		if (shown < 0 || shown >= m_aShownIdx.Count())
			return;

		array<string> m = m_aMails[m_aShownIdx[shown]];
		m_sOpenFile = m[0];
		if (m_sMode != "read")
		{
			m_sMode = "read";
			m_aOpen.Clear();
			Redraw();
		}

		Request1("mail_read", m_sOpenFile);
	}

	//------------------------------------------------------------------------------------------------
	protected void Reply()
	{
		if (m_aOpen.Count() < 5)
			return;

		string subject = m_aOpen[2];
		if (!subject.StartsWith("Re: "))
			subject = "Re: " + subject;

		if (!m_aOpen[1].IsEmpty() && m_aAddresses.Contains(m_aOpen[1]))
			m_sFrom = m_aOpen[1];

		string quoted = m_aOpen[4];
		quoted.Replace("\n", "\n> ");
		Compose(m_aOpen[0], subject, "\n\n--- " + m_aOpen[0] + " wrote:\n> " + quoted);
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteOpen()
	{
		if (m_sOpenFile.IsEmpty() || !m_Desktop)
			return;

		m_Desktop.ShowDialog(this, "delete", "Delete", "Delete this email?", false, "", "Delete");
	}

	//------------------------------------------------------------------------------------------------
	override void OnDialog(string tag, bool ok, string value)
	{
		if (tag == "delete" && ok && !m_sOpenFile.IsEmpty())
			Request1("mail_delete", m_sOpenFile);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuItem(int id)
	{
		if (id >= MI_FROM && id < MI_FROM + m_aAddresses.Count())
		{
			KeepCompose();
			m_sFrom = m_aAddresses[id - MI_FROM];
			super.Redraw();
			return;
		}

		switch (id)
		{
			case MI_OPEN: break;
			case MI_REPLY: Reply(); break;
			case MI_DELETE: DeleteOpen(); break;
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

		switch (cmd)
		{
			case "mail_list":
			{
				m_aMails.Clear();
				foreach (string p : payload)
				{
					array<string> e = ROOT_AE3_Codec.SplitSub(p);
					if (e.Count() >= 4)
						m_aMails.Insert(e);
				}

				if (m_List)
					FillList();

				Status(string.Format("%1 messages", m_aMails.Count()));
				return;
			}

			case "mail_read":
			{
				m_aOpen.Clear();
				m_aOpen.Copy(payload);
				Redraw();
				return;
			}

			case "mail_delete":
			{
				m_sOpenFile = "";
				m_aOpen.Clear();
				m_aMails.Clear();
				Status(Arg(payload, 0));
				Redraw();
				return;
			}

			case "mail_send":
			{
				m_sTo = "";
				m_sSubject = "";
				m_sBody = "";
				m_sMode = "read";
				m_sBox = "sent";
				m_aMails.Clear();
				m_aOpen.Clear();
				m_sOpenFile = "";
				Status("Mail sent.");
				super.Redraw();
				return;
			}

			case "addr_list":
			{
				m_aAddresses.Clear();
				foreach (string a : payload)
				{
					if (!a.IsEmpty())
						m_aAddresses.Insert(a);
				}

				if (!m_aAddresses.Contains(m_sFrom))
					m_sFrom = "";

				if (m_sMode != "read")
					Redraw();

				return;
			}

			case "addr_create":
			case "addr_delete":
			{
				Status(Arg(payload, 0));
				Request0("addr_list");
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnListSelect(int listId, int row)
	{
		Open(row);
	}

	//------------------------------------------------------------------------------------------------
	override void OnListContext(int listId, int row)
	{
		if (row < 0 || !m_Desktop)
			return;

		array<string> labels = {"Open"};
		array<int> ids = {MI_OPEN};
		array<string> icons = {"g_open"};
		if (m_sBox == "inbox")
		{
			labels.Insert("Reply");
			ids.Insert(MI_REPLY);
			icons.Insert("g_back");
		}

		labels.Insert("-");
		ids.Insert(0);
		icons.Insert("");
		labels.Insert("Delete");
		ids.Insert(MI_DELETE);
		icons.Insert("g_delete");

		m_Desktop.ShowMenu(this, labels, ids, icons);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnKey(KeyCode key, bool ctrl)
	{
		if (key == KeyCode.KC_DELETE)
		{
			DeleteOpen();
			return true;
		}

		if (key == KeyCode.KC_F5)
		{
			List0();
			return true;
		}

		if ((key == KeyCode.KC_UP || key == KeyCode.KC_DOWN) && m_List)
		{
			int d = 1;
			if (key == KeyCode.KC_UP)
				d = -1;

			m_List.Move(d);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiChange(Widget w, int id, bool finished)
	{
		if (id == ID_SEARCH && m_wSearch)
		{
			m_sSearch = m_wSearch.GetText();
			FillList();
			return true;
		}

		if (id == ID_ADDR_NEW && finished && m_wNewAddr)
		{
			Request1("addr_create", m_wNewAddr.GetText());
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (id >= ID_ADDR_DEL && id < ID_ADDR_DEL + m_aAddresses.Count())
		{
			Request1("addr_delete", m_aAddresses[id - ID_ADDR_DEL]);
			return true;
		}

		switch (id)
		{
			case ID_REFRESH:
			{
				List0();
				Request0("addr_list");
				return true;
			}

			case ID_INBOX:
			case ID_SENT:
			{
				m_sBox = "inbox";
				if (id == ID_SENT)
					m_sBox = "sent";

				m_aMails.Clear();
				m_aOpen.Clear();
				m_sOpenFile = "";
				if (m_sMode == "compose")
					KeepCompose();

				m_sMode = "read";
				super.Redraw();
				return true;
			}

			case ID_COMPOSE: Compose("", "", ""); return true;
			case ID_ADDRESSES:
			{
				SetMode("addresses");
				Request0("addr_list");
				return true;
			}

			case ID_REPLY: Reply(); return true;
			case ID_DELETE: DeleteOpen(); return true;
			case ID_CANCEL:
			{
				m_sTo = "";
				m_sSubject = "";
				m_sBody = "";
				m_sMode = "read";
				super.Redraw();
				return true;
			}

			case ID_FROM:
			{
				if (!m_Desktop || m_aAddresses.IsEmpty())
				{
					Status("Create an address first (Addresses).");
					return true;
				}

				array<string> labels = {};
				array<int> ids = {};
				array<string> icons = {};
				foreach (int i, string a : m_aAddresses)
				{
					labels.Insert(a);
					ids.Insert(MI_FROM + i);
					icons.Insert("mail");
				}

				m_Desktop.ShowMenu(this, labels, ids, icons);
				return true;
			}

			case ID_ADDR_CREATE:
			{
				if (m_wNewAddr)
					Request1("addr_create", m_wNewAddr.GetText());

				return true;
			}

			case ID_SEND:
			{
				KeepCompose();
				array<string> args = {m_sFrom, m_sTo, m_sSubject, m_sBody};
				Request("mail_send", args);
				return true;
			}
		}

		return false;
	}
}
