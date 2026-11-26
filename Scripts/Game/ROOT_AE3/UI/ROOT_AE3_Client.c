//! Client side of a computer session: opens the menu, routes server messages, GUI requests and replies.
class ROOT_AE3_Client
{
	protected static ref ROOT_AE3_Client s_Instance;

	RplId m_DeviceId;
	string m_sHostname;
	int m_iMode;
	int m_iDesign;
	int m_iInterface;
	string m_sUser;
	string m_sTheme = "0";
	string m_sWallpaper = "0";
	ref array<string> m_aLines = {};
	string m_sPrompt;
	bool m_bHidden;
	bool m_bBusy;
	ref array<string> m_aHistory = {};
	protected string m_sChunk;
	protected int m_iNextRid = 1;
	protected ref map<int, ROOT_AE3_GuiReplyHandler> m_mPending = new map<int, ROOT_AE3_GuiReplyHandler>();
	ROOT_AE3_ComputerMenu m_Menu;
	bool m_bMenuOpening;
	protected bool m_bSettingsRequested;
	protected int m_iDesktopSize = -1;	//!< Client window size 0-3, -1 = setting default
	protected bool m_bPrefsLoaded;
	protected static const string PREFS_FILE = "$profile:ROOT_AE3/client.txt";

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_Client Get()
	{
		if (!s_Instance)
			s_Instance = new ROOT_AE3_Client();

		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	//! Drops all state (game end).
	static void Reset()
	{
		s_Instance = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Local player asks to use a computer. mode 0 terminal, 1 desktop.
	void Open(IEntity device, int mode)
	{
		ROOT_AE3_DeviceComponent d = ROOT_AE3_DeviceComponent.Of(device);
		if (!d)
			return;

		m_DeviceId = d.GetRplId();
		ROOT_AE3_Net.SendToServer(m_DeviceId, ROOT_AE3_EMsg.OPEN, mode.ToString());
	}

	//------------------------------------------------------------------------------------------------
	//! Desktop window size 0 fullscreen, 1 large, 2 medium, 3 small: the player's choice, else the setting.
	int DesktopSize()
	{
		if (!m_bPrefsLoaded)
		{
			m_bPrefsLoaded = true;
			FileHandle fh = FileIO.OpenFile(PREFS_FILE, FileMode.READ);
			if (fh)
			{
				string line;
				if (fh.ReadLine(line) > 0)
					m_iDesktopSize = line.ToInt(-1);

				fh.Close();
			}
		}

		if (m_iDesktopSize >= 0 && m_iDesktopSize <= 3)
			return m_iDesktopSize;

		return Math.ClampInt(ROOT_AE3_SettingsManager.Get().m_iDesktopSize, 0, 3);
	}

	//------------------------------------------------------------------------------------------------
	//! Stores the player's window size in the profile.
	void SetDesktopSize(int size)
	{
		m_iDesktopSize = Math.ClampInt(size, 0, 3);
		FileIO.MakeDirectory("$profile:ROOT_AE3");
		FileHandle fh = FileIO.OpenFile(PREFS_FILE, FileMode.WRITE);
		if (!fh)
			return;

		fh.WriteLine(m_iDesktopSize.ToString());
		fh.Close();
	}

	//------------------------------------------------------------------------------------------------
	//! Local player asks to use a computer from the own inventory. mode -1 auto, 0 terminal, 1 desktop.
	void OpenCarried(IEntity device, int mode = -1)
	{
		ROOT_AE3_DeviceComponent d = ROOT_AE3_DeviceComponent.Of(device);
		if (!d)
			return;

		ROOT_AE3_Net.SendToServer(d.GetRplId(), ROOT_AE3_EMsg.OPEN_CARRIED, mode.ToString());
	}

	//------------------------------------------------------------------------------------------------
	//! Local player renames a device (server checks reach). Empty name = default name.
	static void Rename(IEntity device, string name)
	{
		ROOT_AE3_DeviceComponent d = ROOT_AE3_DeviceComponent.Of(device);
		if (d)
			ROOT_AE3_Net.SendToServer(d.GetRplId(), ROOT_AE3_EMsg.RENAME, name);
	}

	//------------------------------------------------------------------------------------------------
	//! Asks the server once for the client-relevant settings.
	void RequestSettings()
	{
		if (m_bSettingsRequested || Replication.IsServer())
			return;

		m_bSettingsRequested = true;
		ROOT_AE3_Net.SendToServer(RplId.Invalid(), ROOT_AE3_EMsg.SETTINGS, "");
	}

	//------------------------------------------------------------------------------------------------
	void Close()
	{
		ROOT_AE3_Net.SendToServer(m_DeviceId, ROOT_AE3_EMsg.CLOSE, "");
		m_mPending.Clear();
		m_Menu = null;
		m_DeviceId = RplId.Invalid();
	}

	//------------------------------------------------------------------------------------------------
	void Send(ROOT_AE3_EMsg op, string data)
	{
		ROOT_AE3_Net.SendToServer(m_DeviceId, op, data);
	}

	//------------------------------------------------------------------------------------------------
	//! GUI request; handler receives the reply (may be null).
	void Request(string cmd, notnull array<string> args, ROOT_AE3_GuiReplyHandler handler)
	{
		int rid = m_iNextRid;
		m_iNextRid++;
		if (handler)
			m_mPending.Set(rid, handler);

		array<string> f = {rid.ToString(), cmd};
		foreach (string a : args)
		{
			f.Insert(ROOT_AE3_Codec.Clean(a));
		}

		Send(ROOT_AE3_EMsg.GUI, ROOT_AE3_Codec.Join(f));
	}

	//------------------------------------------------------------------------------------------------
	void Forget(ROOT_AE3_GuiReplyHandler handler)
	{
		for (int i = m_mPending.Count() - 1; i >= 0; i--)
		{
			if (m_mPending.GetElement(i) == handler)
				m_mPending.RemoveElement(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnServerMessage(int op, string data)
	{
		if (op == ROOT_AE3_EMsg.CHUNK)
		{
			m_sChunk += data;
			return;
		}

		if (!m_sChunk.IsEmpty())
		{
			data = m_sChunk + data;
			m_sChunk = "";
		}

		switch (op)
		{
			case ROOT_AE3_EMsg.SESSION: OnSession(data); return;
			case ROOT_AE3_EMsg.ENDED: OnEnded(data); return;
			case ROOT_AE3_EMsg.SETTINGS:
			{
				if (!Replication.IsServer())
					ROOT_AE3_SettingsManager.ApplyClient(data);

				return;
			}
			case ROOT_AE3_EMsg.SCREEN: OnScreen(data); return;
			case ROOT_AE3_EMsg.SCREEN_INPUT: OnScreen(data); return;
			case ROOT_AE3_EMsg.SOUND: ROOT_AE3_Sound.PlayLocal(data.ToInt()); return;
			case ROOT_AE3_EMsg.EWO_STATE: ROOT_AE3_EwoPanel.OnState(data); return;
		}

		if (op == ROOT_AE3_EMsg.NOTIFY && !m_Menu)
		{
			array<string> n = ROOT_AE3_Codec.Split(data);
			if (n.Count() >= 2)
				SCR_HintManagerComponent.ShowCustomHint(n[1], n[0], 5);

			return;
		}

		if (!m_Menu)
			return;

		switch (op)
		{
			case ROOT_AE3_EMsg.TERM_OUT:
			{
				array<string> lines = {};
				data.Split("\n", lines, false);
				foreach (string l : lines)
				{
					AddLine(l);
				}

				m_Menu.OnTerminalChanged();
				break;
			}

			case ROOT_AE3_EMsg.TERM_CLEAR:
			{
				m_aLines.Clear();
				m_Menu.OnTerminalChanged();
				break;
			}

			case ROOT_AE3_EMsg.TERM_PROMPT:
			{
				array<string> f = ROOT_AE3_Codec.Split(data);
				if (f.Count() >= 3)
				{
					m_sPrompt = f[0];
					m_bHidden = f[1] == "1";
					m_bBusy = f[2] == "1";
				}

				m_Menu.OnPromptChanged();
				break;
			}

			case ROOT_AE3_EMsg.TERM_SET_LINE: m_Menu.SetInputLine(data); break;
			case ROOT_AE3_EMsg.MODE:
			{
				m_iMode = data.ToInt();
				m_Menu.ShowMode(m_iMode);
				break;
			}

			case ROOT_AE3_EMsg.NOTIFY:
			{
				array<string> f = ROOT_AE3_Codec.Split(data);
				if (f.Count() >= 2)
					m_Menu.Toast(f[0], f[1]);

				break;
			}

			case ROOT_AE3_EMsg.GUI_REPLY: OnGuiReply(data); break;
			case ROOT_AE3_EMsg.GAME_STATE: m_Menu.OnGameState(data); break;
			case ROOT_AE3_EMsg.SNAKE: m_Menu.StartSnake(data == "1"); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	void AddLine(string l)
	{
		m_aLines.Insert(l);
		if (m_aLines.Count() > ROOT_AE3_TerminalState.MAX_LINES)
			m_aLines.RemoveOrdered(0);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSession(string data)
	{
		array<string> f = ROOT_AE3_Codec.Split(data);
		if (f.Count() < 12)
			return;

		m_sHostname = f[0];
		m_iMode = f[1].ToInt();
		if (!Replication.IsServer())
			ROOT_AE3_SettingsManager.ApplyClient(f[2]);

		m_iDesign = f[3].ToInt();
		m_sUser = f[4];
		m_iInterface = f[5].ToInt();
		m_aLines.Clear();
		array<string> lines = {};
		f[6].Split("\n", lines, false);
		foreach (string l : lines)
		{
			AddLine(l);
		}

		m_sPrompt = f[7];
		m_bHidden = f[8] == "1";
		m_aHistory = ROOT_AE3_Codec.SplitSub(f[9]);
		m_sTheme = f[10];
		m_sWallpaper = f[11];
		if (f.Count() > 12 && f[12].ToInt() >= 0)
			m_DeviceId = f[12].ToInt();

		m_bBusy = false;
		if (m_Menu)
		{
			m_Menu.Rebuild();
		}
		else if (!m_bMenuOpening)
		{
			m_bMenuOpening = true;
			if (!GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ROOT_AE3_ComputerMenu))
				m_bMenuOpening = false;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnScreen(string data)
	{
		array<string> f = ROOT_AE3_Codec.Split(data);
		if (f.Count() < 2)
			return;

		RplComponent rpl = RplComponent.Cast(Replication.FindItem(f[0].ToInt()));
		if (!rpl)
			return;

		ROOT_AE3_ComputerComponent c = ROOT_AE3_ComputerComponent.Of(rpl.GetEntity());
		if (!c)
			return;

		if (f.Count() == 2)
			c.ApplyScreenInput(f[1]);
		else
			c.ApplyScreen(f[1], f[2]);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEnded(string reason)
	{
		m_bMenuOpening = false;
		m_DeviceId = RplId.Invalid();
		m_mPending.Clear();
		if (m_Menu)
		{
			ROOT_AE3_ComputerMenu menu = m_Menu;
			m_Menu = null;
			menu.CloseFromServer();
		}

		if (!reason.IsEmpty())
			SCR_HintManagerComponent.ShowCustomHint(reason, "Advanced Equipment", 4);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnGuiReply(string data)
	{
		array<string> f = ROOT_AE3_Codec.Split(data);
		if (f.Count() < 3)
			return;

		int rid = f[0].ToInt();
		bool ok = f[1] == "1";
		string cmd = f[2];
		array<string> payload = {};
		for (int i = 3; i < f.Count(); i++)
		{
			payload.Insert(f[i]);
		}

		if (cmd == "user")
		{
			if (!payload.IsEmpty())
				m_sUser = payload[0];

			if (m_Menu)
				m_Menu.OnUserChanged();
		}

		ROOT_AE3_GuiReplyHandler h;
		if (m_mPending.Find(rid, h))
		{
			m_mPending.Remove(rid);
			if (h)
				h.OnGuiReply(cmd, ok, payload);
		}
	}
}
