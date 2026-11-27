//! Client side of a computer session: opens the menu, routes server messages, GUI requests and replies.
class ROOT_AER_Client
{
	protected static ref ROOT_AER_Client s_Instance;

	RplId m_DeviceId;
	string m_sHostname;
	int m_iMode;
	int m_iDesign;
	int m_iStyle;		//!< Desktop style 0 Windows, 1 Tactical (ROOT_AER_Theme.s_iStyle)
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
	protected ref map<int, ROOT_AER_GuiReplyHandler> m_mPending = new map<int, ROOT_AER_GuiReplyHandler>();
	ROOT_AER_ComputerMenu m_Menu;
	bool m_bMenuOpening;
	protected int m_iMenuOpeningAt;		//!< System.GetTickCount() of the last open attempt
	protected bool m_bSettingsRequested;
	protected int m_iDesktopSize = -1;	//!< Client window size 0-3, -1 = setting default
	protected bool m_bPrefsLoaded;
	protected static const string PREFS_FILE = "$profile:ROOT_AER/client.txt";

	//------------------------------------------------------------------------------------------------
	static ROOT_AER_Client Get()
	{
		if (!s_Instance)
			s_Instance = new ROOT_AER_Client();

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
		ROOT_AER_DeviceComponent d = ROOT_AER_DeviceComponent.Of(device);
		if (!d)
			return;

		m_DeviceId = d.GetRplId();
		ROOT_AER_Net.SendToServer(m_DeviceId, ROOT_AER_EMsg.OPEN, mode.ToString());
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

		return Math.ClampInt(ROOT_AER_SettingsManager.Get().m_iDesktopSize, 0, 3);
	}

	//------------------------------------------------------------------------------------------------
	//! Stores the player's window size in the profile.
	void SetDesktopSize(int size)
	{
		m_iDesktopSize = Math.ClampInt(size, 0, 3);
		FileIO.MakeDirectory("$profile:ROOT_AER");
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
		ROOT_AER_DeviceComponent d = ROOT_AER_DeviceComponent.Of(device);
		if (!d)
			return;

		m_DeviceId = d.GetRplId();
		ROOT_AER_Net.SendToServer(m_DeviceId, ROOT_AER_EMsg.OPEN_CARRIED, mode.ToString());
	}

	//------------------------------------------------------------------------------------------------
	//! Local player renames a device (server checks reach). Empty name = default name.
	static void Rename(IEntity device, string name)
	{
		ROOT_AER_DeviceComponent d = ROOT_AER_DeviceComponent.Of(device);
		if (d)
			ROOT_AER_Net.SendToServer(d.GetRplId(), ROOT_AER_EMsg.RENAME, name);
	}

	//------------------------------------------------------------------------------------------------
	//! Asks the server once for the client-relevant settings.
	void RequestSettings()
	{
		if (m_bSettingsRequested || Replication.IsServer())
			return;

		m_bSettingsRequested = true;
		ROOT_AER_Net.SendToServer(RplId.Invalid(), ROOT_AER_EMsg.SETTINGS, "");
	}

	//------------------------------------------------------------------------------------------------
	void Close()
	{
		ROOT_AER_Net.SendToServer(m_DeviceId, ROOT_AER_EMsg.CLOSE, "");
		m_mPending.Clear();
		m_Menu = null;
		m_DeviceId = RplId.Invalid();
	}

	//------------------------------------------------------------------------------------------------
	void Send(ROOT_AER_EMsg op, string data)
	{
		ROOT_AER_Net.SendToServer(m_DeviceId, op, data);
	}

	//------------------------------------------------------------------------------------------------
	//! GUI request; handler receives the reply (may be null).
	void Request(string cmd, notnull array<string> args, ROOT_AER_GuiReplyHandler handler)
	{
		int rid = m_iNextRid;
		m_iNextRid++;
		if (handler)
			m_mPending.Set(rid, handler);

		array<string> f = {rid.ToString(), cmd};
		foreach (string a : args)
		{
			f.Insert(ROOT_AER_Codec.Clean(a));
		}

		Send(ROOT_AER_EMsg.GUI, ROOT_AER_Codec.Join(f));
	}

	protected ref array<ref ROOT_AER_Window> m_aParked = {};
	protected RplId m_ParkedDevice = RplId.Invalid();
	protected string m_sParkedUser;

	//------------------------------------------------------------------------------------------------
	//! Keeps detached desktop windows of a computer and user until the next session (ROOT_AER_DesktopView.Destroy).
	void Park(RplId device, string user, notnull array<ref ROOT_AER_Window> windows)
	{
		DropParked();
		m_ParkedDevice = device;
		m_sParkedUser = user;
		foreach (ROOT_AER_Window w : windows)
		{
			m_aParked.Insert(w);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Parked windows of that computer and user (moved to outWindows); other parked windows are closed.
	bool TakeParked(RplId device, string user, notnull array<ref ROOT_AER_Window> outWindows)
	{
		if (m_aParked.IsEmpty())
			return false;

		if (device != m_ParkedDevice || user != m_sParkedUser || user.IsEmpty())
		{
			DropParked();
			return false;
		}

		foreach (ROOT_AER_Window w : m_aParked)
		{
			outWindows.Insert(w);
		}

		m_aParked.Clear();
		m_ParkedDevice = RplId.Invalid();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void DropParked()
	{
		foreach (ROOT_AER_Window w : m_aParked)
		{
			if (w && w.m_App)
				w.m_App.OnClose();
		}

		m_aParked.Clear();
		m_ParkedDevice = RplId.Invalid();
	}

	//------------------------------------------------------------------------------------------------
	void Forget(ROOT_AER_GuiReplyHandler handler)
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
		if (op == ROOT_AER_EMsg.CHUNK)
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
			case ROOT_AER_EMsg.SESSION: OnSession(data); return;
			case ROOT_AER_EMsg.ENDED: OnEnded(data); return;
			case ROOT_AER_EMsg.SETTINGS:
			{
				if (!Replication.IsServer())
					ROOT_AER_SettingsManager.ApplyClient(data);

				return;
			}
			case ROOT_AER_EMsg.SCREEN: OnScreen(data); return;
			case ROOT_AER_EMsg.SCREEN_INPUT: OnScreen(data); return;
			case ROOT_AER_EMsg.SOUND: ROOT_AER_Sound.PlayLocal(data.ToInt()); return;
			case ROOT_AER_EMsg.EWO_STATE: ROOT_AER_EwoPanel.OnState(data); return;
			case ROOT_AER_EMsg.CABLE_HAND: ROOT_AER_CableHand.OnMessage(data); return;
		}

		if (op == ROOT_AER_EMsg.NOTIFY && !m_Menu)
		{
			array<string> n = ROOT_AER_Codec.Split(data);
			if (n.Count() >= 2)
				SCR_HintManagerComponent.ShowCustomHint(n[1], n[0], 5);

			return;
		}

		if (!m_Menu)
			return;

		switch (op)
		{
			case ROOT_AER_EMsg.TERM_OUT:
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

			case ROOT_AER_EMsg.TERM_CLEAR:
			{
				m_aLines.Clear();
				m_Menu.OnTerminalChanged();
				break;
			}

			case ROOT_AER_EMsg.TERM_PROMPT:
			{
				array<string> f = ROOT_AER_Codec.Split(data);
				if (f.Count() >= 3)
				{
					m_sPrompt = f[0];
					m_bHidden = f[1] == "1";
					m_bBusy = f[2] == "1";
				}

				m_Menu.OnPromptChanged();
				break;
			}

			case ROOT_AER_EMsg.TERM_SET_LINE: m_Menu.SetInputLine(data); break;
			case ROOT_AER_EMsg.MODE:
			{
				m_iMode = data.ToInt();
				m_Menu.ShowMode(m_iMode);
				break;
			}

			case ROOT_AER_EMsg.NOTIFY:
			{
				array<string> f = ROOT_AER_Codec.Split(data);
				if (f.Count() >= 2)
					m_Menu.Toast(f[0], f[1]);

				break;
			}

			case ROOT_AER_EMsg.GUI_REPLY: OnGuiReply(data); break;
			case ROOT_AER_EMsg.GAME_STATE: m_Menu.OnGameState(data); break;
			case ROOT_AER_EMsg.SNAKE: m_Menu.StartSnake(data == "1"); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	void AddLine(string l)
	{
		m_aLines.Insert(l);
		if (m_aLines.Count() > ROOT_AER_TerminalState.MAX_LINES)
			m_aLines.RemoveOrdered(0);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSession(string data)
	{
		array<string> f = ROOT_AER_Codec.Split(data);
		if (f.Count() < 12)
			return;

		m_sHostname = f[0];
		m_iMode = f[1].ToInt();
		if (!Replication.IsServer())
			ROOT_AER_SettingsManager.ApplyClient(f[2]);

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
		m_aHistory = ROOT_AER_Codec.SplitSub(f[9]);
		m_sTheme = f[10];
		m_sWallpaper = f[11];
		if (f.Count() > 12 && f[12].ToInt() >= 0)
			m_DeviceId = f[12].ToInt();

		if (f.Count() > 13)
			m_iStyle = f[13].ToInt();

		m_bBusy = false;
		if (m_Menu)
		{
			m_Menu.Rebuild();
			return;
		}

		// An open attempt older than 2 s did not reach OnMenuOpen.
		if (m_bMenuOpening && System.GetTickCount() - m_iMenuOpeningAt < 2000)
			return;

		m_bMenuOpening = true;
		m_iMenuOpeningAt = System.GetTickCount();
		GetGame().GetCallqueue().Remove(OpenMenu);
		GetGame().GetCallqueue().CallLater(OpenMenu, 0, false, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! Next frame after SESSION, so a closing inventory or dialog menu does not block it; retried while
	//! another menu still closes.
	protected void OpenMenu(int tries)
	{
		if (m_Menu || !m_bMenuOpening || m_DeviceId == RplId.Invalid())
			return;

		if (GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ROOT_AER_ComputerMenu))
			return;

		if (tries >= 10)
		{
			m_bMenuOpening = false;
			return;
		}

		m_iMenuOpeningAt = System.GetTickCount();
		GetGame().GetCallqueue().CallLater(OpenMenu, 200, false, tries + 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnScreen(string data)
	{
		array<string> f = ROOT_AER_Codec.Split(data);
		if (f.Count() < 2)
			return;

		RplComponent rpl = RplComponent.Cast(Replication.FindItem(f[0].ToInt()));
		if (!rpl)
			return;

		ROOT_AER_ComputerComponent c = ROOT_AER_ComputerComponent.Of(rpl.GetEntity());
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
			ROOT_AER_ComputerMenu menu = m_Menu;
			m_Menu = null;
			menu.CloseFromServer();
		}

		if (!reason.IsEmpty())
			SCR_HintManagerComponent.ShowCustomHint(reason, "Advanced Equipment Reforged", 4);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnGuiReply(string data)
	{
		array<string> f = ROOT_AER_Codec.Split(data);
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

		ROOT_AER_GuiReplyHandler h;
		if (m_mPending.Find(rid, h))
		{
			m_mPending.Remove(rid);
			if (h)
				h.OnGuiReply(cmd, ok, payload);
		}
	}
}
