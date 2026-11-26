//! Messenger: navigation (Message Center, Handles, Reading as), conversation list with search and new
//! conversation, message pane with bubbles. Each conversation belongs to one own handle (sender identity).
class ROOT_AE3_AppMessenger : ROOT_AE3_App
{
	protected static const int ID_NAV_CHATS = 1;
	protected static const int ID_NAV_HANDLES = 2;
	protected static const int ID_READING = 3;
	protected static const int ID_SEARCH = 4;
	protected static const int ID_NEW = 5;
	protected static const int ID_MSG = 6;
	protected static const int ID_SEND = 7;
	protected static const int ID_HANDLE_NEW = 8;
	protected static const int ID_HANDLE_CREATE = 9;
	protected static const int ID_MSGS = 10;
	protected static const int ID_THREAD = 200;
	protected static const int ID_HANDLE_DEL = 400;
	protected static const int MI_READ_ALL = 1;
	protected static const int MI_READ = 100;
	protected static const int MI_PEER = 300;
	protected static const int MI_OTHER = 2;
	protected static const float RAIL_W = 160;
	protected static const float LIST_W = 290;
	protected static const float ROW_H = 54;

	protected ref array<ref array<string>> m_aThreads = {};	//!< self, peer, last, time
	protected ref array<int> m_aShown = {};
	protected ref array<string> m_aHandles = {};
	protected ref array<string> m_aKnown = {};
	protected ref array<string> m_aLines = {};
	protected string m_sView = "chats";
	protected string m_sReading = "*";
	protected string m_sSelf;
	protected string m_sPeer;
	protected string m_sSearch;
	protected string m_sDraft;
	protected bool m_bHandlesLoaded;
	protected int m_iThreadScroll;
	protected int m_iMsgScroll;
	protected float m_fTimer;
	protected EditBoxWidget m_wSearch;
	protected EditBoxWidget m_wMsg;
	protected EditBoxWidget m_wNewHandle;
	protected FrameWidget m_wThreads;
	protected FrameWidget m_wMsgs;
	protected ref ROOT_AE3_ForwardListener m_ThreadListener;
	protected ref ROOT_AE3_ForwardListener m_MsgListener;
	protected float m_fListH;
	protected float m_fMsgW;
	protected float m_fMsgH;

	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Messenger";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "messenger";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(920, 540, 0);
	}

	//------------------------------------------------------------------------------------------------
	override void Build()
	{
		float w = Width();
		float h = Height();

		// Navigation rail.
		ROOT_AE3_UI.Rect(GetContent(), 0, 0, RAIL_W, h, ROOT_AE3_Theme.Sidebar());
		ROOT_AE3_UI.Icon(GetContent(), 12, 12, 24, "messenger");
		Label(42, 14, RAIL_W - 50, 22, "Messenger", 16);
		Nav(52, "messenger", "Message Center", ID_NAV_CHATS, m_sView == "chats");
		Nav(88, "user", "Handles", ID_NAV_HANDLES, m_sView == "handles");
		ROOT_AE3_UI.Text(GetContent(), 12, h - 64, RAIL_W - 20, 16, "Reading as", 12, ROOT_AE3_Theme.Muted());
		string reading = m_sReading;
		if (reading == "*")
			reading = "All handles";

		ROOT_AE3_UI.Box(GetContent(), 8, h - 44, RAIL_W - 16, 32, ROOT_AE3_Theme.Button(), ROOT_AE3_Theme.Border());
		ROOT_AE3_UI.ItemButton(GetContent(), 9, h - 43, RAIL_W - 18, 30, "g_chevron_down", reading, ID_READING, this, 13, ROOT_AE3_Theme.Text(), ROOT_AE3_Theme.Button(), ROOT_AE3_Theme.Hover());
		ROOT_AE3_UI.Rect(GetContent(), RAIL_W, 0, 1, h, ROOT_AE3_Theme.Border());

		if (m_sView == "handles")
			BuildHandles();
		else
			BuildChats();

		if (!m_bHandlesLoaded)
		{
			m_bHandlesLoaded = true;
			Request0("handle_list");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Nav(float y, string icon, string label, int id, bool active)
	{
		Color bg = ROOT_AE3_UI.Rgb(0, 0, 0, 0);
		if (active)
			bg = ROOT_AE3_Theme.Select();

		ROOT_AE3_UI.ItemButton(GetContent(), 6, y, RAIL_W - 12, 32, icon, label, id, this, 14, ROOT_AE3_Theme.Text(), bg, ROOT_AE3_Theme.Hover());
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildChats()
	{
		float h = Height();
		float x = RAIL_W + 1;
		ROOT_AE3_UI.Box(GetContent(), x + 8, 8, LIST_W - 56, 30, ROOT_AE3_Theme.Field(), ROOT_AE3_Theme.Border());
		ROOT_AE3_UI.Icon(GetContent(), x + 16, 15, 16, "g_search", ROOT_AE3_Theme.Muted());
		m_wSearch = ROOT_AE3_UI.Edit(GetContent(), x + 38, 11, LIST_W - 94, 24, m_sSearch, ID_SEARCH, this);
		if (m_wSearch)
		{
			m_wSearch.SetColor(ROOT_AE3_Theme.Text());
			ROOT_AE3_UI.Placeholder(m_wSearch, GetContent(), x + 42, 15, LIST_W - 100, "Search handle / message", ROOT_AE3_Theme.Muted());
		}

		ROOT_AE3_UI.Box(GetContent(), x + LIST_W - 42, 8, 34, 30, ROOT_AE3_Theme.Accent());
		ROOT_AE3_UI.IconButton(GetContent(), x + LIST_W - 42, 8, 34, 30, "g_plus", ID_NEW, this, 14, ROOT_AE3_UI.Rgb(1, 1, 1), ROOT_AE3_Theme.Accent(), ROOT_AE3_Theme.Hover());
		m_fListH = h - 48;
		m_wThreads = ROOT_AE3_UI.Frame(GetContent(), x, 46, LIST_W, m_fListH);
		RenderThreads();
		ROOT_AE3_UI.Rect(GetContent(), x + LIST_W, 0, 1, h, ROOT_AE3_Theme.Border());
		BuildConversation();
		if (m_aThreads.IsEmpty())
			Request0("chat_threads");
	}

	//------------------------------------------------------------------------------------------------
	protected bool Visible(array<string> t)
	{
		if (m_sReading != "*" && t[0] != m_sReading)
			return false;

		if (m_sSearch.IsEmpty())
			return true;

		string q = m_sSearch;
		q.ToLower();
		string a = t[1] + " " + t[0] + " " + t[2];
		a.ToLower();
		return a.Contains(q);
	}

	//------------------------------------------------------------------------------------------------
	protected void RenderThreads()
	{
		if (!m_wThreads)
			return;

		m_ThreadListener = new ROOT_AE3_ForwardListener(this);
		ROOT_AE3_UI.Clear(m_wThreads);
		ROOT_AE3_UI.Rect(m_wThreads, 0, 0, LIST_W, m_fListH, ROOT_AE3_UI.Rgb(0, 0, 0, 0), ID_THREAD - 1, m_ThreadListener);
		m_aShown.Clear();
		foreach (int i, array<string> t : m_aThreads)
		{
			if (Visible(t))
				m_aShown.Insert(i);
		}

		int lines = Math.Floor(m_fListH / ROW_H);
		m_iThreadScroll = Math.ClampInt(m_iThreadScroll, 0, Math.Max(0, m_aShown.Count() - lines));
		float y = 0;
		for (int k = m_iThreadScroll; k < Math.Min(m_aShown.Count(), m_iThreadScroll + lines); k++)
		{
			array<string> t = m_aThreads[m_aShown[k]];
			Color bg = ROOT_AE3_UI.Rgb(0, 0, 0, 0);
			if (t[0] == m_sSelf && t[1] == m_sPeer)
				bg = ROOT_AE3_Theme.Select();

			ROOT_AE3_UI.Hit(m_wThreads, 4, y + 2, LIST_W - 8, ROW_H - 4, ID_THREAD + k, m_ThreadListener, bg, ROOT_AE3_Theme.Hover());
			ROOT_AE3_UI.Icon(m_wThreads, 12, y + 11, 32, "user");
			ROOT_AE3_UI.Text(m_wThreads, 52, y + 8, LIST_W - 150, 20, t[1], 15, ROOT_AE3_Theme.Text());
			Chip(m_wThreads, LIST_W - 96, y + 9, 86, t[0]);
			ROOT_AE3_UI.Text(m_wThreads, 52, y + 29, LIST_W - 62, 18, t[2], 12, ROOT_AE3_Theme.Muted());
			y += ROW_H;
		}

		if (m_aShown.IsEmpty())
			ROOT_AE3_UI.Text(m_wThreads, 0, 30, LIST_W, 20, "No conversations", 14, ROOT_AE3_Theme.Muted(), false, false, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Colored label of an own handle; the color is stable per handle.
	protected void Chip(Widget parent, float x, float y, float w, string handle)
	{
		int hash;
		for (int i = 0; i < handle.Length(); i++)
		{
			hash = (hash * 31 + handle.ToAscii(i)) % 360;
		}

		array<ref Color> colors = {ROOT_AE3_UI.Rgb(0.2, 0.5, 0.8), ROOT_AE3_UI.Rgb(0.6, 0.3, 0.7), ROOT_AE3_UI.Rgb(0.2, 0.6, 0.4), ROOT_AE3_UI.Rgb(0.75, 0.45, 0.15), ROOT_AE3_UI.Rgb(0.7, 0.25, 0.3), ROOT_AE3_UI.Rgb(0.3, 0.55, 0.6)};
		Color c = colors[hash % colors.Count()];
		ROOT_AE3_UI.Rect(parent, x, y, w, 18, c);
		string label = handle;
		if (label.StartsWith("@"))
			label = label.Substring(1, label.Length() - 1);

		ROOT_AE3_UI.Text(parent, x + 4, y + 1, w - 8, 16, label, 11, ROOT_AE3_UI.Rgb(1, 1, 1), false, false, 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildConversation()
	{
		float x = RAIL_W + LIST_W + 2;
		float w = Width() - x;
		float h = Height();
		if (m_sPeer.IsEmpty() || (m_sReading != "*" && m_sSelf != m_sReading))
		{
			ROOT_AE3_UI.Icon(GetContent(), x + (w - 56) * 0.5, h * 0.4 - 40, 56, "messenger");
			ROOT_AE3_UI.Text(GetContent(), x, h * 0.4 + 24, w, 22, "Select or start a conversation.", 15, ROOT_AE3_Theme.Muted(), false, false, 1);
			m_wMsgs = null;
			m_wMsg = null;
			return;
		}

		ROOT_AE3_UI.Rect(GetContent(), x, 0, w, 46, ROOT_AE3_Theme.Bar());
		ROOT_AE3_UI.Icon(GetContent(), x + 12, 9, 28, "user");
		Label(x + 48, 13, w - 160, 22, m_sPeer, 16);
		Chip(GetContent(), x + w - 104, 14, 92, m_sSelf);
		ROOT_AE3_UI.Rect(GetContent(), x, 46, w, 1, ROOT_AE3_Theme.Border());
		m_fMsgW = w;
		m_fMsgH = h - 47 - 52;
		m_wMsgs = ROOT_AE3_UI.Frame(GetContent(), x, 47, m_fMsgW, m_fMsgH);
		RenderMessages();
		ROOT_AE3_UI.Rect(GetContent(), x, h - 52, w, 52, ROOT_AE3_Theme.Bar());
		ROOT_AE3_UI.Box(GetContent(), x + 10, h - 42, w - 120, 32, ROOT_AE3_Theme.Field(), ROOT_AE3_Theme.Border());
		m_wMsg = ROOT_AE3_UI.Edit(GetContent(), x + 16, h - 39, w - 132, 26, m_sDraft, ID_MSG, this);
		if (m_wMsg)
		{
			m_wMsg.SetColor(ROOT_AE3_Theme.Text());
			ROOT_AE3_UI.Placeholder(m_wMsg, GetContent(), x + 20, h - 35, w - 140, "Message", ROOT_AE3_Theme.Muted());
		}

		MainBtn(x + w - 102, h - 42, 92, 32, "Send", ID_SEND);
	}

	//------------------------------------------------------------------------------------------------
	protected void RenderMessages()
	{
		if (!m_wMsgs)
			return;

		m_MsgListener = new ROOT_AE3_ForwardListener(this);
		ROOT_AE3_UI.Clear(m_wMsgs);
		ROOT_AE3_UI.Rect(m_wMsgs, 0, 0, m_fMsgW, m_fMsgH, ROOT_AE3_Theme.Field(), ID_MSGS, m_MsgListener);
		float maxW = m_fMsgW * 0.7;
		float charW = 7.6;
		int perLine = Math.Max(8, Math.Floor((maxW - 24) / charW));
		m_iMsgScroll = Math.ClampInt(m_iMsgScroll, 0, Math.Max(0, m_aLines.Count() - 1));
		float y = m_fMsgH - 8;
		int last = m_aLines.Count() - 1 - m_iMsgScroll;
		for (int i = last; i >= 0 && y > 0; i--)
		{
			string line = m_aLines[i];
			string time;
			string who;
			string text = line;
			int close = line.IndexOf("] ");
			if (line.StartsWith("[") && close > 0)
			{
				time = line.Substring(1, close - 1);
				text = line.Substring(close + 2, line.Length() - close - 2);
			}

			int colon = text.IndexOf(": ");
			if (colon > 0)
			{
				who = text.Substring(0, colon);
				text = text.Substring(colon + 2, text.Length() - colon - 2);
			}

			bool outgoing = who == m_sSelf;
			float len = text.Length();
			int rows = Math.Max(1, Math.Ceil(len / perLine));
			float bh = rows * 18 + 26;
			float bw = Math.Min(maxW, Math.Max(90, Math.Min(text.Length(), perLine) * charW + 24));
			y -= bh;
			float bx = 12;
			Color bg = ROOT_AE3_Theme.Button();
			Color fg = ROOT_AE3_Theme.Text();
			if (outgoing)
			{
				bx = m_fMsgW - bw - 12;
				bg = ROOT_AE3_Theme.Accent();
				fg = ROOT_AE3_UI.Rgb(1, 1, 1);
			}

			ROOT_AE3_UI.Rect(m_wMsgs, bx, y, bw, bh, bg);
			ROOT_AE3_UI.Text(m_wMsgs, bx + 10, y + 5, bw - 20, rows * 18 + 2, text, 14, fg, false, true);
			string stamp = time;
			if (stamp.Length() >= 16)
				stamp = stamp.Substring(11, 5);

			ROOT_AE3_UI.Text(m_wMsgs, bx + 10, y + bh - 17, bw - 20, 14, stamp, 10, fg, false, false, 2);
			y -= 8;
		}

		if (m_aLines.IsEmpty())
			ROOT_AE3_UI.Text(m_wMsgs, 0, m_fMsgH * 0.45, m_fMsgW, 20, "No messages yet. Say hello.", 14, ROOT_AE3_Theme.Muted(), false, false, 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildHandles()
	{
		float x = RAIL_W + 16;
		float w = Math.Min(460, Width() - x - 16);
		float y = 14;
		Label(x, y, w, 24, "Handles", 18);
		y += 30;
		ROOT_AE3_UI.Text(GetContent(), x, y, Width() - x - 16, 36, "Identities used to send and receive messages on this computer. Choose Message Center to chat.", 13, ROOT_AE3_Theme.Muted(), false, true);
		y += 44;
		m_wNewHandle = Field(x, y, w - 110, "", ID_HANDLE_NEW);
		ROOT_AE3_UI.Placeholder(m_wNewHandle, GetContent(), x + 8, y + 4, w - 126, "@handle", ROOT_AE3_Theme.Muted());
		MainBtn(x + w - 100, y - 1, 100, 28, "Create", ID_HANDLE_CREATE);
		y += 40;
		if (m_aHandles.IsEmpty())
			ROOT_AE3_UI.Text(GetContent(), x, y, w, 20, "No handles", 14, ROOT_AE3_Theme.Muted());

		foreach (int i, string hnd : m_aHandles)
		{
			ROOT_AE3_UI.Box(GetContent(), x, y, w, 36, ROOT_AE3_Theme.Field(), ROOT_AE3_Theme.Border());
			ROOT_AE3_UI.Icon(GetContent(), x + 10, y + 8, 20, "user");
			ROOT_AE3_UI.Text(GetContent(), x + 38, y + 9, w - 150, 20, hnd, 14, ROOT_AE3_Theme.Text());
			Btn(x + w - 90, y + 4, 84, 28, "Delete", ID_HANDLE_DEL + i);
			y += 42;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Redraw()
	{
		if (m_wMsg)
			m_sDraft = m_wMsg.GetText();

		super.Redraw();
	}

	//------------------------------------------------------------------------------------------------
	override void Update(float dt)
	{
		m_fTimer += dt;
		if (m_fTimer < 3)
			return;

		m_fTimer = 0;
		if (m_sView != "chats")
			return;

		Request0("chat_threads");
		if (!m_sPeer.IsEmpty())
			Request2("chat_read", m_sSelf, m_sPeer);
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenThread(string self, string peer)
	{
		m_sSelf = self;
		m_sPeer = peer;
		m_aLines.Clear();
		m_iMsgScroll = 0;
		m_sDraft = "";
		Redraw();
		Request2("chat_read", self, peer);
	}

	//------------------------------------------------------------------------------------------------
	protected string DefaultSelf()
	{
		if (m_sReading != "*")
			return m_sReading;

		if (m_aHandles.IsEmpty())
			return "";

		return m_aHandles[0];
	}

	//------------------------------------------------------------------------------------------------
	protected void StartWith(string peer)
	{
		peer = peer.Trim();
		if (peer.IsEmpty())
			return;

		if (!peer.StartsWith("@"))
			peer = "@" + peer;

		string self = DefaultSelf();
		if (self.IsEmpty())
		{
			Status("Create a handle first (Handles).");
			return;
		}

		bool found;
		foreach (array<string> t : m_aThreads)
		{
			if (t[0] == self && t[1] == peer)
				found = true;
		}

		if (!found)
		{
			array<string> nt = {self, peer, "", ""};
			m_aThreads.InsertAt(nt, 0);
		}

		OpenThread(self, peer);
	}

	//------------------------------------------------------------------------------------------------
	protected void SendMsg()
	{
		if (!m_wMsg || m_wMsg.GetText().Trim().IsEmpty() || m_sPeer.IsEmpty())
			return;

		array<string> args = {m_sSelf, m_sPeer, m_wMsg.GetText()};
		Request("chat_send", args);
		ROOT_AE3_UI.SetEdit(m_wMsg, "");
		m_sDraft = "";
	}

	//------------------------------------------------------------------------------------------------
	override void OnDialog(string tag, bool ok, string value)
	{
		if (tag == "newchat" && ok)
			StartWith(value);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuItem(int id)
	{
		if (id == MI_READ_ALL)
		{
			m_sReading = "*";
			Redraw();
			return;
		}

		if (id >= MI_READ && id < MI_READ + m_aHandles.Count())
		{
			m_sReading = m_aHandles[id - MI_READ];
			if (m_sSelf != m_sReading)
				m_sPeer = "";

			Redraw();
			return;
		}

		if (id >= MI_PEER && id < MI_PEER + m_aKnown.Count())
		{
			StartWith(m_aKnown[id - MI_PEER]);
			return;
		}

		if (id == MI_OTHER && m_Desktop)
			m_Desktop.ShowDialog(this, "newchat", "Start conversation", "Handle of the other person:", true, "@", "Start");
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
			case "handle_list":
			{
				m_aHandles.Clear();
				m_aHandles.Copy(payload);
				if (m_sReading != "*" && !m_aHandles.Contains(m_sReading))
					m_sReading = "*";

				Redraw();
				return;
			}

			case "handle_create":
			case "handle_delete":
			{
				Status(Arg(payload, 0));
				m_aThreads.Clear();
				Request0("handle_list");
				Request0("chat_threads");
				return;
			}

			case "handles_all":
			{
				m_aKnown.Clear();
				m_aKnown.Copy(payload);
				if (!m_Desktop)
					return;

				array<string> labels = {};
				array<int> ids = {};
				array<string> icons = {};
				foreach (int i, string k : m_aKnown)
				{
					if (i >= 12)
						break;

					labels.Insert(k);
					ids.Insert(MI_PEER + i);
					icons.Insert("user");
				}

				if (!labels.IsEmpty())
				{
					labels.Insert("-");
					ids.Insert(0);
					icons.Insert("");
				}

				labels.Insert("Other handle...");
				ids.Insert(MI_OTHER);
				icons.Insert("g_plus");
				m_Desktop.ShowMenu(this, labels, ids, icons);
				return;
			}

			case "chat_threads":
			{
				// Keep a new conversation that has no messages yet.
				array<string> pending;
				foreach (array<string> t : m_aThreads)
				{
					if (t[0] == m_sSelf && t[1] == m_sPeer)
						pending = t;
				}

				m_aThreads.Clear();
				bool hasActive;
				foreach (string p : payload)
				{
					array<string> e = ROOT_AE3_Codec.SplitSub(p);
					if (e.Count() < 4)
						continue;

					m_aThreads.Insert(e);
					if (e[0] == m_sSelf && e[1] == m_sPeer)
						hasActive = true;
				}

				if (pending && !hasActive)
					m_aThreads.InsertAt(pending, 0);

				RenderThreads();
				return;
			}

			case "chat_read":
			{
				if (Arg(payload, 0) != m_sSelf || Arg(payload, 1) != m_sPeer)
					return;

				array<string> lines = {};
				string log = Arg(payload, 2);
				log.Split("\n", lines, true);
				bool changed = lines.Count() != m_aLines.Count();
				m_aLines.Clear();
				m_aLines.Copy(lines);
				if (changed)
					RenderMessages();

				return;
			}

			case "chat_send":
			{
				Request2("chat_read", m_sSelf, m_sPeer);
				Request0("chat_threads");
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiChange(Widget w, int id, bool finished)
	{
		if (id == ID_SEARCH && m_wSearch)
		{
			m_sSearch = m_wSearch.GetText();
			RenderThreads();
			return true;
		}

		if (finished && id == ID_MSG)
		{
			SendMsg();
			return true;
		}

		if (finished && id == ID_HANDLE_NEW && m_wNewHandle)
		{
			Request1("handle_create", m_wNewHandle.GetText());
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiWheel(Widget w, int id, int wheel)
	{
		if (id == ID_MSGS)
		{
			m_iMsgScroll += wheel;
			RenderMessages();
			return true;
		}

		if (id >= ID_THREAD - 1 && id < ID_THREAD + m_aShown.Count())
		{
			m_iThreadScroll -= wheel;
			RenderThreads();
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (id >= ID_THREAD && id < ID_THREAD + m_aShown.Count())
		{
			array<string> t = m_aThreads[m_aShown[id - ID_THREAD]];
			OpenThread(t[0], t[1]);
			return true;
		}

		if (id >= ID_HANDLE_DEL && id < ID_HANDLE_DEL + m_aHandles.Count())
		{
			string hnd = m_aHandles[id - ID_HANDLE_DEL];
			if (m_sReading == hnd)
				m_sReading = "*";

			Request1("handle_delete", hnd);
			return true;
		}

		switch (id)
		{
			case ID_NAV_CHATS:
			{
				m_sView = "chats";
				Redraw();
				Request0("chat_threads");
				return true;
			}

			case ID_NAV_HANDLES:
			{
				m_sView = "handles";
				Redraw();
				return true;
			}

			case ID_READING:
			{
				if (!m_Desktop)
					return true;

				array<string> labels = {"All handles"};
				array<int> ids = {MI_READ_ALL};
				array<string> icons = {"g_check"};
				if (m_sReading != "*")
					icons[0] = "";

				foreach (int i, string hnd : m_aHandles)
				{
					labels.Insert(hnd);
					ids.Insert(MI_READ + i);
					if (hnd == m_sReading)
						icons.Insert("g_check");
					else
						icons.Insert("");
				}

				m_Desktop.ShowMenu(this, labels, ids, icons);
				return true;
			}

			case ID_NEW:
			{
				if (DefaultSelf().IsEmpty())
				{
					Status("Create a handle first (Handles).");
					return true;
				}

				Request0("handles_all");
				return true;
			}

			case ID_SEND: SendMsg(); return true;
			case ID_HANDLE_CREATE:
			{
				if (m_wNewHandle)
					Request1("handle_create", m_wNewHandle.GetText());

				return true;
			}
		}

		return false;
	}
}
