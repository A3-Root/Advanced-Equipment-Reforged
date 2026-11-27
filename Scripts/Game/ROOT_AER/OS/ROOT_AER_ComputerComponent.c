enum ROOT_AER_EScreen
{
	OFF,
	BOOT,
	TERMINAL,
	DESKTOP,
	STANDBY,
	CRASHED,
	SHUTDOWN
}

[ComponentEditorProps(category: "Advanced Equipment Reforged", description: "armaOS computer: filesystem, users, terminal, desktop")]
class ROOT_AER_ComputerComponentClass : ScriptComponentClass
{
}

//! armaOS computer. All OS state lives on the authority; clients get the screen mirror and session messages.
//! Replicated: hostname, screen mode, screen text, typed input line, operator flag, design.
class ROOT_AER_ComputerComponent : ScriptComponent
{
	[Attribute("-1", uiwidget: UIWidgets.ComboBox, desc: "Interface", enums: { ParamEnum("Setting default", "-1"), ParamEnum("Terminal only", "0"), ParamEnum("Desktop only", "1"), ParamEnum("Both", "2") }, category: "armaOS"), RplProp()]
	protected int m_iInterfaceMode;

	[RplProp()]
	protected int m_iUsbMask;	//!< Bit per occupied USB port

	[Attribute("", desc: "Hostname; empty = automatic", category: "armaOS")]
	protected string m_sHostname;

	[Attribute("0", uiwidget: UIWidgets.ComboBox, desc: "Root login", enums: { ParamEnum("Setting default", "0"), ParamEnum("Allow", "1"), ParamEnum("Deny", "2") }, category: "armaOS")]
	protected int m_iRootLogin;

	[Attribute("", desc: "Root password; empty = setting default", category: "armaOS")]
	protected string m_sRootPassword;

	[Attribute(desc: "User accounts", category: "armaOS")]
	protected ref array<ref ROOT_AER_UserEntry> m_aUserEntries;

	[Attribute(desc: "Additional sudoers", category: "armaOS")]
	protected ref array<string> m_aSudoerNames;

	[Attribute(desc: "Identity IDs, faction keys or player names allowed to use the terminal; empty = everyone", category: "armaOS")]
	protected ref array<string> m_aCliAccess;

	[Attribute(desc: "Identity IDs, faction keys or player names allowed to use the desktop; empty = everyone", category: "armaOS")]
	protected ref array<string> m_aGuiAccess;

	[Attribute("1", desc: "Install the default armaOS commands", category: "armaOS")]
	protected bool m_bInstallCommands;

	[Attribute("1", desc: "Install the games", category: "armaOS")]
	protected bool m_bInstallGames;

	[Attribute("0", desc: "Install crypto and crack", category: "armaOS")]
	protected bool m_bInstallSecurity;

	[Attribute("0 0.1 0", desc: "Model-relative position of USB port 0", category: "armaOS")]
	protected vector m_vUsb0;

	[Attribute("0 0.1 0", desc: "Model-relative position of USB port 1", category: "armaOS")]
	protected vector m_vUsb1;

	[Attribute("0", desc: "Number of USB ports (0-2)", params: "0 2 1", category: "armaOS")]
	protected int m_iUsbPorts;

	[RplProp()]
	protected string m_sHostnameRpl;

	[RplProp(onRplName: "OnScreenRpl")]
	protected int m_iScreen;

	protected string m_sScreenText;		//!< Sent by RPC to players in screen range only (ROOT_AER_EMsg.SCREEN)
	protected string m_sScreenInput;
	protected ref array<int> m_aViewers = {};

	[RplProp()]
	protected int m_iOperator;

	[RplProp(onRplName: "OnScreenRpl")]
	protected int m_iDesign = -1;

	[Attribute("-1", uiwidget: UIWidgets.ComboBox, desc: "Desktop style", enums: { ParamEnum("Setting", "-1"), ParamEnum("Civilian (Windows)", "0"), ParamEnum("Military (Tactical)", "1") }, category: "armaOS")]
	protected int m_iUiStyle;

	protected ref ROOT_AER_Filesystem m_Fs;
	protected ref map<string, string> m_mUsers = new map<string, string>();
	protected ref map<string, ref array<string>> m_mHistory = new map<string, ref array<string>>();
	protected ref array<string> m_aInstalled = {};
	protected static ref array<ref ROOT_AER_AccessCondition> s_aAccessConditions = {};		//!< Programs installed on this computer
	protected ref map<string, ref ROOT_AER_Command> m_mCustom = new map<string, ref ROOT_AER_Command>();	//!< Custom commands of this computer
	protected ref map<string, string> m_mPrefs = new map<string, string>();
	protected ref array<ref ROOT_AER_CalendarEvent> m_aCalendar = {};
	protected ref ROOT_AER_TerminalState m_Terminal = new ROOT_AER_TerminalState();
	protected ref array<string> m_aMailAddresses = {};
	protected ref map<string, ref ROOT_AER_Webpage> m_mWebpages = new map<string, ref ROOT_AER_Webpage>();
	protected ref array<ref ROOT_AER_UsbSlot> m_aUsbSlots = {null, null};
	protected bool m_bReady;
	protected bool m_bRestored;	//!< State came from persistence or a save; editor setup is not applied again.
	protected bool m_bBooting;
	protected string m_sLastMirror;
	protected float m_fLastMirrorTime;
	bool m_bAutoHostname;
	protected string m_sSudoersSource;
	protected ref array<string> m_aSudoersCache = {};
	protected string m_sPendingDesktop;
	protected RplComponent m_RplComponent;
	protected ROOT_AER_PowerComponent m_Power;
	protected static int s_iHostCounter;

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!m_aUserEntries)
			m_aUserEntries = {};

		if (!m_aSudoerNames)
			m_aSudoerNames = {};

		if (!m_aCliAccess)
			m_aCliAccess = {};

		if (!m_aGuiAccess)
			m_aGuiAccess = {};

		if (!GetGame().InPlayMode() || !IsAuthority())
			return;

		InventoryItemComponent item = InventoryItemComponent.Cast(owner.FindComponent(InventoryItemComponent));
		if (item)
			item.m_OnParentSlotChangedInvoker.Insert(OnSlotChanged);

		GetGame().GetCallqueue().CallLater(InitServer, 100, false);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		if (IsAuthority() && GetGame().InPlayMode() && !ROOT_AER_Manager.IsEnding())
			ROOT_AER_UsbManager.EjectAll(this);

		GetGame().GetCallqueue().Remove(InitServer);
		GetGame().GetCallqueue().Remove(FinishBoot);
		GetGame().GetCallqueue().Remove(FinishShutdown);
		GetGame().GetCallqueue().Remove(PushMirrorDelayed);
		GetGame().GetCallqueue().Remove(UpdateViewers);
		GetGame().GetCallqueue().Remove(PushDesktopDelayed);
		GetGame().GetCallqueue().Remove(RespawnUsbProps);
		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! Drops all state (game end).
	static void Reset()
	{
		s_aAccessConditions.Clear();
		s_iHostCounter = 0;
	}

	//------------------------------------------------------------------------------------------------
	static ROOT_AER_ComputerComponent Of(IEntity ent)
	{
		if (!ent)
			return null;

		return ROOT_AER_ComputerComponent.Cast(ent.FindComponent(ROOT_AER_ComputerComponent));
	}

	//------------------------------------------------------------------------------------------------
	bool IsAuthority()
	{
		return !m_RplComponent || !m_RplComponent.IsProxy();
	}

	//------------------------------------------------------------------------------------------------
	protected void InitServer()
	{
		m_Power = ROOT_AER_PowerComponent.Of(GetOwner());
		if (m_Power)
			m_Power.GetOnStateChanged().Insert(OnPowerState);

		if (m_sHostname.IsEmpty())
		{
			s_iHostCounter++;
			ROOT_AER_DeviceComponent dev = ROOT_AER_DeviceComponent.Of(GetOwner());
			string base = "computer";
			if (dev)
				base = ROOT_AER_Util.Lower(ROOT_AER_DeviceComponent.GenericName(dev.GetDeviceType()));

			m_bAutoHostname = true;
			m_sHostname = base + "-" + s_iHostCounter.ToString();
			while (HostnameUsed(m_sHostname))
			{
				s_iHostCounter++;
				m_sHostname = base + "-" + s_iHostCounter.ToString();
			}
		}

		m_sHostnameRpl = m_sHostname;
		if (m_bRestored)
		{
			FinishInit();
			return;
		}

		if (!m_Fs)
		{
			m_Fs = new ROOT_AER_Filesystem();
			m_Fs.m_Computer = this;
			m_Fs.SeedDefaults();
		}

		string e;
		for (int i = 0; i < m_iUsbPorts; i++)
		{
			m_Fs.CreateDir("/mnt/usb" + i.ToString(), "root", "root", 63, e);
		}

		RemountUsb();

		if (m_bInstallCommands)
			ROOT_AER_CommandRegistry.Get().InstallDefaults(this);

		if (m_bInstallSecurity)
		{
			ROOT_AER_CommandRegistry.Get().Install(this, "crypto");
			ROOT_AER_CommandRegistry.Get().Install(this, "crack");
		}

		if (m_bInstallGames)
			ROOT_AER_CommandRegistry.Get().InstallGames(this);

		foreach (ROOT_AER_UserEntry u : m_aUserEntries)
		{
			if (u && !u.m_sName.IsEmpty())
			{
				AddUser(u.m_sName, u.m_sPassword);
				if (u.m_bSudoer)
					AddSudoer(u.m_sName);
			}
		}

		foreach (string s : m_aSudoerNames)
		{
			AddSudoer(s);
		}

		ROOT_AER_SetupComponent setup = ROOT_AER_SetupComponent.Of(GetOwner());
		if (setup)
			setup.Apply(this);

		FinishInit();
	}

	//------------------------------------------------------------------------------------------------
	protected void FinishInit()
	{
		if (!m_bReady)
			GetGame().GetCallqueue().CallLater(UpdateViewers, 1000, true);

		m_bReady = true;
		UpdateBatteryFile();
		Replication.BumpMe();
		ROOT_AER_Intel.ProvisionIdentity(this);
		ROOT_AER_Manager.Get().NotifyDeviceReady(ROOT_AER_DeviceComponent.Of(GetOwner()), m_bRestored);
		if (m_Power && m_Power.IsOn())
			OnPowerState(m_Power, ROOT_AER_EPowerState.OFF, ROOT_AER_EPowerState.ON);
		else if (m_Power && m_Power.GetState() == ROOT_AER_EPowerState.STANDBY)
			SetScreen(ROOT_AER_EScreen.STANDBY);
	}

	//------------------------------------------------------------------------------------------------
	bool IsReady()
	{
		return m_bReady;
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AER_Filesystem GetFilesystem()
	{
		return m_Fs;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: replaces the filesystem (restore, persistence).
	void SetFilesystem(ROOT_AER_Filesystem fs)
	{
		if (!fs)
			return;

		m_Fs = fs;
		m_Fs.m_Computer = this;
		m_bRestored = true;
		RemountUsb();
		Print(string.Format("[AER] Filesystem of %1 replaced (restore).", m_sHostname), LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	int GetOperator()
	{
		return m_iOperator;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: program name installed (file written by ROOT_AER_CommandRegistry.Install).
	void MarkInstalled(string program)
	{
		if (!m_aInstalled.Contains(program))
			m_aInstalled.Insert(program);
	}

	//------------------------------------------------------------------------------------------------
	bool IsInstalled(string program)
	{
		return m_aInstalled.Contains(program);
	}

	//------------------------------------------------------------------------------------------------
	array<string> GetInstalled()
	{
		return m_aInstalled;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: custom command of this computer only.
	void AddCustomCommand(notnull ROOT_AER_Command cmd)
	{
		m_mCustom.Set(cmd.GetName(), cmd);
	}

	//------------------------------------------------------------------------------------------------
	//! Program of this computer: its own custom command, else the shared registry.
	ROOT_AER_Command FindCommand(string program)
	{
		ROOT_AER_Command c = m_mCustom.Get(program);
		if (c)
			return c;

		return ROOT_AER_CommandRegistry.Get().Find(program);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: new automatic hostname (a restored computer took this one).
	void RenameAuto()
	{
		string base = m_sHostname;
		int dash = base.LastIndexOf("-");
		if (dash > 0)
			base = base.Substring(0, dash);

		s_iHostCounter++;
		string name = base + "-" + s_iHostCounter.ToString();
		while (HostnameUsed(name))
		{
			s_iHostCounter++;
			name = base + "-" + s_iHostCounter.ToString();
		}

		m_sHostname = name;
		m_sHostnameRpl = name;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	protected bool HostnameUsed(string name)
	{
		foreach (ROOT_AER_DeviceComponent d : ROOT_AER_Manager.Get().GetDevices())
		{
			ROOT_AER_ComputerComponent other = ROOT_AER_ComputerComponent.Of(d.GetOwner());
			if (other && other != this && other.GetHostname() == name)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: drops users, preferences, mail addresses and web pages before a restore.
	void ClearAccounts()
	{
		ROOT_AER_Intel.ReleaseAccounts(this);
		m_mUsers.Clear();
		m_mPrefs.Clear();
		m_aMailAddresses.Clear();
		m_mWebpages.Clear();
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AER_TerminalState GetTerminal()
	{
		return m_Terminal;
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AER_PowerComponent GetPower()
	{
		if (!m_Power)
			m_Power = ROOT_AER_PowerComponent.Of(GetOwner());

		return m_Power;
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AER_NetworkComponent GetNetwork()
	{
		return ROOT_AER_NetworkComponent.Of(GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	string GetHostname()
	{
		if (!m_sHostnameRpl.IsEmpty())
			return m_sHostnameRpl;

		return m_sHostname;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority.
	void SetHostname(string name)
	{
		name = ROOT_AER_Util.Clip(ROOT_AER_Util.Ascii(ROOT_AER_Codec.Clean(name)), 32);
		name.Replace("\n", "");
		name.Replace("\t", "");
		name = name.Trim();
		if (name.IsEmpty())
			return;

		foreach (ROOT_AER_DeviceComponent d : ROOT_AER_Manager.Get().GetDevices())
		{
			ROOT_AER_ComputerComponent other = ROOT_AER_ComputerComponent.Of(d.GetOwner());
			if (other && other != this && other.GetHostname() == name && other.m_bAutoHostname)
				other.RenameAuto();
		}

		m_bAutoHostname = false;
		m_sHostname = name;
		m_sHostnameRpl = name;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! 0 terminal only, 1 desktop only, 2 both.
	int GetInterfaceMode()
	{
		if (m_iInterfaceMode < 0)
			return ROOT_AER_SettingsManager.Get().m_iDesktopDefaultMode;

		return m_iInterfaceMode;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority. -1 setting, 0 terminal, 1 desktop, 2 both.
	void SetInterfaceMode(int mode)
	{
		m_iInterfaceMode = Math.ClampInt(mode, -1, 2);
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! Replicated: a drive sits in a port.
	bool HasUsbDrive()
	{
		return m_iUsbMask != 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Replicated: bit per occupied USB port.
	int GetUsbMask()
	{
		return m_iUsbMask;
	}

	//------------------------------------------------------------------------------------------------
	void GetAccess(int mode, notnull array<string> outEntries)
	{
		if (mode == 0)
			outEntries.Copy(m_aCliAccess);
		else
			outEntries.Copy(m_aGuiAccess);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority. mode 0 terminal, 1 desktop; entries are identity IDs, faction keys or player names, empty = everyone.
	void SetAccess(int mode, notnull array<string> entries)
	{
		if (mode == 0)
			m_aCliAccess.Copy(entries);
		else
			m_aGuiAccess.Copy(entries);
	}

	//------------------------------------------------------------------------------------------------
	//! True when the player may use the interface (0 terminal, 1 desktop).
	bool CanAccess(int playerId, int mode)
	{
		if (!ListAccess(playerId, mode))
			return false;

		foreach (ROOT_AER_AccessCondition cond : s_aAccessConditions)
		{
			if (cond && !cond.Allow(playerId, this, mode))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static void AddAccessCondition(ROOT_AER_AccessCondition cond)
	{
		if (cond && !s_aAccessConditions.Contains(cond))
			s_aAccessConditions.Insert(cond);
	}

	//------------------------------------------------------------------------------------------------
	static void RemoveAccessCondition(ROOT_AER_AccessCondition cond)
	{
		s_aAccessConditions.RemoveItem(cond);
	}

	//------------------------------------------------------------------------------------------------
	protected bool ListAccess(int playerId, int mode)
	{
		array<string> list = m_aCliAccess;
		if (mode == 1)
			list = m_aGuiAccess;

		if (!list || list.IsEmpty())
			return true;

		UUID uid = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		if (!uid.IsNull() && list.Contains(uid))
			return true;

		if (list.Contains(GetGame().GetPlayerManager().GetPlayerName(playerId)))
			return true;

		Faction f = SCR_FactionManager.SGetPlayerFaction(playerId);
		return f && list.Contains(f.GetFactionKey());
	}

	//------------------------------------------------------------------------------------------------
	int GetDesign()
	{
		if (m_iDesign < 0)
			return ROOT_AER_SettingsManager.Get().m_iTerminalDesign;

		return m_iDesign;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority.
	void SetDesign(int design)
	{
		m_iDesign = Math.ClampInt(design, 0, ROOT_AER_TerminalDesigns.COUNT - 1);
		Replication.BumpMe();
	}

	// Users ------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Authority: adds or updates a user and creates its home directory.
	void AddUser(string name, string password)
	{
		name = ROOT_AER_Codec.Clean(name);
		if (!ROOT_AER_Filesystem.ValidName(name) || name == "root")
			return;

		m_mUsers.Set(name, ROOT_AER_Codec.Clean(password));
		EnsureHome(name);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: home folder with Desktop, Documents and Downloads, each created when missing.
	void EnsureHome(string name)
	{
		if (!m_Fs || !ROOT_AER_Filesystem.ValidName(name))
			return;

		string e;
		string home = ROOT_AER_Filesystem.HomeOf(name);
		if (!m_Fs.Exists(home))
			m_Fs.CreateDir(home, "root", name, 7, e, true);

		array<string> subs = {"/Desktop", "/Documents", "/Downloads"};
		foreach (string sub : subs)
		{
			if (!m_Fs.Exists(home + sub))
				m_Fs.CreateDir(home + sub, "root", name, 7, e);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Authority.
	void RemoveUser(string name)
	{
		m_mUsers.Remove(name);
		RemoveSudoer(name);
	}

	//------------------------------------------------------------------------------------------------
	bool HasUser(string name)
	{
		return name == "root" || m_mUsers.Contains(name);
	}

	//------------------------------------------------------------------------------------------------
	void GetUsers(notnull array<string> outUsers)
	{
		for (int i = 0, n = m_mUsers.Count(); i < n; i++)
		{
			outUsers.Insert(m_mUsers.GetKey(i));
		}

		outUsers.Sort();
	}

	//------------------------------------------------------------------------------------------------
	map<string, string> GetUserMap()
	{
		return m_mUsers;
	}

	//------------------------------------------------------------------------------------------------
	string GetRootPassword()
	{
		if (!m_sRootPassword.IsEmpty())
			return m_sRootPassword;

		return ROOT_AER_SettingsManager.Get().m_sDefaultRootPassword;
	}

	//------------------------------------------------------------------------------------------------
	//! Own root password; empty = setting default.
	string GetRootPasswordRaw()
	{
		return m_sRootPassword;
	}

	//------------------------------------------------------------------------------------------------
	//! -1 setting, 0 terminal, 1 desktop, 2 both.
	int GetInterfaceModeRaw()
	{
		return m_iInterfaceMode;
	}

	//------------------------------------------------------------------------------------------------
	//! Desktop style 0 Windows, 1 Tactical: Game Master / API choice (stored as computer pref "ui_style"),
	//! else the World Editor attribute, else the setting m_iDesktopStyle.
	int GetUiStyle()
	{
		int raw = GetUiStyleRaw();
		if (raw < 0)
			return ROOT_AER_SettingsManager.Get().m_iDesktopStyle;

		return raw;
	}

	//------------------------------------------------------------------------------------------------
	//! -1 = setting default.
	int GetUiStyleRaw()
	{
		string pref = GetPref("", "ui_style");
		if (!pref.IsEmpty())
			return pref.ToInt();

		return m_iUiStyle;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority. -1 = setting default; open sessions switch at once.
	void SetUiStyle(int style)
	{
		SetPref("", "ui_style", Math.ClampInt(style, -1, 1).ToString());
		ROOT_AER_Manager.Get().ResendSessionsOf(this);
	}

	//------------------------------------------------------------------------------------------------
	//! -1 = setting default.
	int GetDesignRaw()
	{
		return m_iDesign;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority.
	void SetRootPassword(string password)
	{
		m_sRootPassword = ROOT_AER_Codec.Clean(password);
	}

	//------------------------------------------------------------------------------------------------
	bool CheckPassword(string user, string password)
	{
		if (user == "root")
			return password == GetRootPassword();

		string pw;
		if (!m_mUsers.Find(user, pw))
			return false;

		return pw == password;
	}

	//------------------------------------------------------------------------------------------------
	bool AllowsRootLogin()
	{
		if (m_iRootLogin == 1)
			return true;

		if (m_iRootLogin == 2)
			return false;

		return ROOT_AER_SettingsManager.Get().m_bAllowRootLogin;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority. 0 setting, 1 allow, 2 deny.
	void SetRootLogin(int policy)
	{
		m_iRootLogin = Math.ClampInt(policy, 0, 2);
	}

	//------------------------------------------------------------------------------------------------
	int GetRootLoginPolicy()
	{
		return m_iRootLogin;
	}

	//------------------------------------------------------------------------------------------------
	//! Names listed in /etc/sudoers (one per line).
	void GetSudoers(notnull array<string> outNames)
	{
		string content, e;
		if (!m_Fs || !m_Fs.ReadFile("/etc/sudoers", "root", content, e))
			return;

		if (content != m_sSudoersSource)
		{
			m_sSudoersSource = content;
			m_aSudoersCache.Clear();
			array<string> lines = {};
			content.Split("\n", lines, true);
			foreach (string l : lines)
			{
				l = l.Trim();
				if (!l.IsEmpty() && !l.StartsWith("#"))
					m_aSudoersCache.Insert(l);
			}
		}

		outNames.Copy(m_aSudoersCache);
	}

	//------------------------------------------------------------------------------------------------
	bool IsSudoer(string user)
	{
		if (user == "root" || user == "admin")
			return true;

		array<string> names = {};
		GetSudoers(names);
		return names.Contains(user);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority.
	void AddSudoer(string user)
	{
		if (user.IsEmpty() || IsSudoer(user))
			return;

		string content, e;
		m_Fs.ReadFile("/etc/sudoers", "root", content, e);
		if (!content.IsEmpty() && !content.EndsWith("\n"))
			content += "\n";

		m_Fs.WriteFile("/etc/sudoers", content + user + "\n", "root", false, e, 3);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority.
	void RemoveSudoer(string user)
	{
		array<string> names = {};
		GetSudoers(names);
		if (!names.RemoveItem(user))
			return;

		string content;
		foreach (string n : names)
		{
			content += n + "\n";
		}

		string e;
		m_Fs.WriteFile("/etc/sudoers", content, "root", false, e);
	}

	//------------------------------------------------------------------------------------------------
	//! Filesystem user of a shell user: sudoers act as root when the setting allows it.
	//! Account used for permission checks: root and admin act as root; sudoers too while elevation is on.
	string FsUser(string user)
	{
		if (user == "root" || user == "admin")
			return "root";

		if (ROOT_AER_SettingsManager.Get().m_bCliElevateSudoers && IsSudoer(user))
			return "root";

		return user;
	}

	// History, prefs, calendar, mail -----------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	void AddHistory(string user, string line)
	{
		array<string> h = m_mHistory.Get(user);
		if (!h)
		{
			h = {};
			m_mHistory.Set(user, h);
		}

		h.Insert(line);
		if (h.Count() > 200)
			h.RemoveOrdered(0);
	}

	//------------------------------------------------------------------------------------------------
	array<string> GetHistory(string user)
	{
		array<string> h = m_mHistory.Get(user);
		if (!h)
		{
			h = {};
			m_mHistory.Set(user, h);
		}

		return h;
	}

	//------------------------------------------------------------------------------------------------
	map<string, ref array<string>> GetHistoryMap()
	{
		return m_mHistory;
	}

	//------------------------------------------------------------------------------------------------
	string GetPref(string user, string key, string def = "")
	{
		string v;
		if (m_mPrefs.Find(user + ":" + key, v))
			return v;

		return def;
	}

	//------------------------------------------------------------------------------------------------
	void SetPref(string user, string key, string value)
	{
		m_mPrefs.Set(user + ":" + key, ROOT_AER_Codec.Clean(value));
	}

	//------------------------------------------------------------------------------------------------
	map<string, string> GetPrefMap()
	{
		return m_mPrefs;
	}

	//------------------------------------------------------------------------------------------------
	//! Network transfer speed of a user in KB/s, 0 = device / router / setting.
	float GetUserSpeed(string user)
	{
		return GetPref(user, "speed", "0").ToFloat();
	}

	//------------------------------------------------------------------------------------------------
	array<ref ROOT_AER_CalendarEvent> GetCalendar()
	{
		return m_aCalendar;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority.
	void AddCalendarEvent(string date, string time, string title, string text, string location = "")
	{
		if (m_aCalendar.Count() >= 200)
			return;

		date = ROOT_AER_Util.Clip(date, 16);
		time = ROOT_AER_Util.Clip(time, 16);
		title = ROOT_AER_Util.Clip(title, 128);
		text = ROOT_AER_Util.Clip(text, 1024);
		ROOT_AER_CalendarEvent ev = new ROOT_AER_CalendarEvent();
		ev.m_sDate = date;
		ev.m_sTime = time;
		ev.m_sTitle = ROOT_AER_Codec.Clean(title);
		ev.m_sText = ROOT_AER_Codec.Clean(text);
		ev.m_sLocation = ROOT_AER_Codec.Clean(ROOT_AER_Util.Clip(location, 128));
		m_aCalendar.Insert(ev);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority. Removes events matching date and title; empty title removes every event of the date.
	int RemoveCalendarEvent(string date, string title)
	{
		int removed;
		for (int i = m_aCalendar.Count() - 1; i >= 0; i--)
		{
			ROOT_AER_CalendarEvent ev = m_aCalendar[i];
			if (ev.m_sDate == date && (title.IsEmpty() || ev.m_sTitle == title))
			{
				m_aCalendar.Remove(i);
				removed++;
			}
		}

		return removed;
	}

	//------------------------------------------------------------------------------------------------
	array<string> GetMailAddresses()
	{
		return m_aMailAddresses;
	}

	//------------------------------------------------------------------------------------------------
	//! Web pages visible only on this computer.
	map<string, ref ROOT_AER_Webpage> GetWebpages()
	{
		return m_mWebpages;
	}

	// USB --------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	int GetUsbPortCount()
	{
		return m_iUsbPorts;
	}

	//------------------------------------------------------------------------------------------------
	vector GetUsbPortPos(int port)
	{
		if (port == 1)
			return m_vUsb1;

		return m_vUsb0;
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AER_UsbSlot GetUsbDrive(int port)
	{
		if (port < 0 || port >= m_iUsbPorts)
			return null;

		return m_aUsbSlots[port];
	}

	//------------------------------------------------------------------------------------------------
	//! First free port, -1 when all are used.
	int FreeUsbPort()
	{
		for (int i = 0; i < m_iUsbPorts; i++)
		{
			if (!m_aUsbSlots[i])
				return i;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: occupies or frees a port. Mounts the drive at /mnt/usbN; unmounts on free.
	void SetUsbDrive(int port, ROOT_AER_UsbSlot drive)
	{
		if (port < 0 || port >= m_iUsbPorts)
			return;

		if (!drive)
			Unmount(port);

		m_aUsbSlots[port] = drive;
		if (drive && drive.m_Fs && drive.m_iCapacity > 0)
			drive.m_Fs.m_fCapacityMB = drive.m_iCapacity;

		if (drive)
			m_iUsbMask |= 1 << port;
		else
			m_iUsbMask &= ~(1 << port);

		if (drive && m_Fs && !IsMounted(port))
		{
			string reason;
			Mount(port, reason);
		}

		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	bool IsMounted(int port)
	{
		return m_Fs && m_Fs.GetMount("usb" + port.ToString()) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: mounts the drive in a port at /mnt/usbN.
	bool Mount(int port, out string reason)
	{
		ROOT_AER_UsbSlot drive = GetUsbDrive(port);
		if (!drive)
		{
			reason = "No device in usb" + port.ToString() + ".";
			return false;
		}

		if (IsMounted(port))
		{
			reason = "usb" + port.ToString() + " is already mounted.";
			return false;
		}

		string id = "usb" + port.ToString();
		string e;
		string dir = "/mnt/" + id;
		if (!m_Fs.Exists(dir))
			m_Fs.CreateDir(dir, "root", "root", 63, e);

		ROOT_AER_FsNode node = m_Fs.Resolve(dir, "root", e);
		if (!node)
		{
			reason = e;
			return false;
		}

		node.m_sMountId = id;
		m_Fs.Mount(id, drive.GetFilesystem());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority.
	bool Unmount(int port)
	{
		string id = "usb" + port.ToString();
		if (!m_Fs || !m_Fs.GetMount(id))
			return false;

		m_Fs.Mount(id, null);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Marks /mnt/usbN as mount points (writes fail while no drive is mounted) and mounts the drives in the ports.
	protected void RemountUsb()
	{
		for (int i = 0; i < m_iUsbPorts; i++)
		{
			string id = "usb" + i.ToString();
			string e;
			if (!m_Fs.Exists("/mnt/" + id))
				m_Fs.CreateDir("/mnt/" + id, "root", "root", 63, e, true);

			ROOT_AER_FsNode node = m_Fs.Resolve("/mnt/" + id, "root", e, false);
			if (node)
				node.m_sMountId = id;

			m_Fs.Mount(id, null);
			if (GetUsbDrive(i))
				Mount(i, e);
		}
	}

	// Power and screen ------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	protected void OnPowerState(ROOT_AER_PowerComponent power, int oldState, int newState)
	{
		if (!IsAuthority())
			return;

		GetGame().GetCallqueue().Remove(FinishBoot);
		GetGame().GetCallqueue().Remove(FinishShutdown);
		switch (newState)
		{
			case ROOT_AER_EPowerState.ON:
			{
				if (oldState == ROOT_AER_EPowerState.STANDBY)
				{
					if (m_Terminal.m_aBuffer.IsEmpty())
					{
						ROOT_AER_Shell.AddHeader(m_Terminal);
						m_Terminal.m_eInput = ROOT_AER_ETermInput.LOGIN;
					}

					SetScreen(ROOT_AER_EScreen.TERMINAL);
					PushMirror(true);
					break;
				}

				m_bBooting = true;
				m_Terminal.Reset();
				SetScreen(ROOT_AER_EScreen.BOOT);
				ROOT_AER_Sound.Play(GetOwner(), ROOT_AER_Sound.START);
				GetGame().GetCallqueue().CallLater(FinishBoot, ROOT_AER_SettingsManager.Get().m_fStartupTime * 1000, false);
				break;
			}

			case ROOT_AER_EPowerState.STANDBY:
			{
				m_bBooting = false;
				ROOT_AER_Manager.Get().EndSessionsOf(ROOT_AER_DeviceComponent.Of(GetOwner()));
				SetScreen(ROOT_AER_EScreen.STANDBY);
				ROOT_AER_Sound.Play(GetOwner(), ROOT_AER_Sound.STANDBY);
				break;
			}

			case ROOT_AER_EPowerState.CRASHED:
			{
				m_bBooting = false;
				ROOT_AER_Manager.Get().EndSessionsOf(ROOT_AER_DeviceComponent.Of(GetOwner()));
				SetScreen(ROOT_AER_EScreen.CRASHED);
				break;
			}

			default:
			{
				m_bBooting = false;
				ROOT_AER_Manager.Get().EndSessionsOf(ROOT_AER_DeviceComponent.Of(GetOwner()));
				ROOT_AER_GameTableSystem.Get().OnDeviceOff(this);
				m_Terminal.Reset();
				SetScreen(ROOT_AER_EScreen.OFF);
				if (oldState != ROOT_AER_EPowerState.OFF)
					ROOT_AER_Sound.Play(GetOwner(), ROOT_AER_Sound.STOP);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Picked up: flash drives stay plugged in (port props stay attached), power links drop, a computer nobody
	//! operates from this inventory goes to standby. Sessions of other players end by ROOT_AER_Session.PlayerValid.
	//! Placed again: missing port props come back.
	protected void OnSlotChanged(InventoryStorageSlot oldSlot, InventoryStorageSlot newSlot)
	{
		if (!IsAuthority())
			return;

		if (!newSlot)
		{
			GetGame().GetCallqueue().CallLater(RespawnUsbProps, 200, false);
			return;
		}

		ROOT_AER_PowerComponent p = GetPower();
		if (!p)
			return;

		p.Disconnect();
		IEntity carrier = ROOT_AER_Util.Carrier(GetOwner());
		int carrierId;
		if (carrier)
			carrierId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(carrier);

		string reason;
		if (p.IsOn() && (carrierId <= 0 || !ROOT_AER_Manager.Get().HasSession(carrierId, this)))
			p.Standby(reason);
	}

	//------------------------------------------------------------------------------------------------
	protected void RespawnUsbProps()
	{
		if (ROOT_AER_Util.IsCarried(GetOwner()))
			return;

		for (int i = 0; i < m_aUsbSlots.Count(); i++)
		{
			ROOT_AER_UsbSlot slot = m_aUsbSlots[i];
			if (slot && !slot.m_Prop)
				slot.m_Prop = ROOT_AER_UsbManager.SpawnProp(this, i, slot.m_sPropPrefab);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void FinishBoot()
	{
		m_bBooting = false;
		ROOT_AER_Shell.AddHeader(m_Terminal);
		m_Terminal.m_eInput = ROOT_AER_ETermInput.LOGIN;
		SetScreen(ROOT_AER_EScreen.TERMINAL);
		PushMirror(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: shutdown with screen delay.
	void Shutdown()
	{
		ROOT_AER_Manager.Get().EndSessionsOf(ROOT_AER_DeviceComponent.Of(GetOwner()));
		SetScreen(ROOT_AER_EScreen.SHUTDOWN);
		GetGame().GetCallqueue().CallLater(FinishShutdown, ROOT_AER_SettingsManager.Get().m_fShutdownTime * 1000, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void FinishShutdown()
	{
		if (GetPower())
			GetPower().TurnOff();
	}

	//------------------------------------------------------------------------------------------------
	bool IsBooting()
	{
		return m_bBooting;
	}

	//------------------------------------------------------------------------------------------------
	//! True when the computer is on and finished booting.
	bool IsRunning()
	{
		ROOT_AER_PowerComponent p = GetPower();
		if (p && !p.IsOn())
			return false;

		return m_bReady && !m_bBooting && m_iScreen != ROOT_AER_EScreen.SHUTDOWN;
	}

	//------------------------------------------------------------------------------------------------
	//! Server: session may start for this player.
	bool CanOpen(int playerId, out string reason)
	{
		if (!m_bReady)
		{
			reason = "The computer is starting.";
			return false;
		}

		ROOT_AER_PowerComponent p = GetPower();
		bool standby = p && p.GetState() == ROOT_AER_EPowerState.STANDBY;
		if (!standby && !IsRunning())
		{
			reason = "The computer is not running.";
			return false;
		}

		ROOT_AER_Session s = ROOT_AER_Manager.Get().GetSessionOf(this);
		if (s && s.m_iPlayerId != playerId)
		{
			reason = "The computer is in use.";
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: operator player id, 0 = free.
	void SetOperator(int playerId)
	{
		m_iOperator = playerId;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	bool IsInUse()
	{
		return m_iOperator != 0;
	}

	//------------------------------------------------------------------------------------------------
	int GetScreen()
	{
		return m_iScreen;
	}

	//------------------------------------------------------------------------------------------------
	string GetScreenText()
	{
		return m_sScreenText;
	}

	//------------------------------------------------------------------------------------------------
	string GetScreenInput()
	{
		return m_sScreenInput;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority.
	void SetScreen(ROOT_AER_EScreen screen)
	{
		if (m_iScreen == screen)
			return;

		m_iScreen = screen;
		Replication.BumpMe();
		OnScreenRpl();
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: mirrors the terminal tail to the screen, throttled by the UI-on-texture settings.
	void PushMirror(bool force = false)
	{
		ROOT_AER_Settings s = ROOT_AER_SettingsManager.Get();
		if (!s.m_bUiOnTexture)
			return;

		float now = GetGame().GetWorld().GetWorldTime() / 1000.0;
		if (!force && now - m_fLastMirrorTime < s.m_fUiOnTexUpdateInterval)
		{
			GetGame().GetCallqueue().Remove(PushMirrorDelayed);
			GetGame().GetCallqueue().CallLater(PushMirrorDelayed, s.m_fUiOnTexUpdateInterval * 1000, false);
			return;
		}

		string text;
		if (m_iScreen == ROOT_AER_EScreen.DESKTOP)
			text = m_sScreenText;
		else
			text = m_Terminal.Tail(Math.Min(s.m_iUiMaxTransmitLines, 22)) + "\n" + ROOT_AER_Shell.PromptOf(this, m_Terminal);

		if (!force && s.m_bUiEnableChangeDetection && text == m_sLastMirror)
			return;

		m_fLastMirrorTime = now;
		m_sLastMirror = text;
		m_sScreenText = text;
		BroadcastScreen();
	}

	//------------------------------------------------------------------------------------------------
	protected void PushMirrorDelayed()
	{
		PushMirror(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: desktop summary shown on the screen.
	void SetDesktopMirror(string summary)
	{
		summary = ROOT_AER_Codec.Clean(summary);
		if (summary == m_sScreenText)
			return;

		m_sPendingDesktop = summary;
		float now = GetGame().GetWorld().GetWorldTime() / 1000.0;
		if (now - m_fLastMirrorTime < 0.5)
		{
			GetGame().GetCallqueue().Remove(PushDesktopDelayed);
			GetGame().GetCallqueue().CallLater(PushDesktopDelayed, 500, false);
			return;
		}

		PushDesktopDelayed();
	}

	//------------------------------------------------------------------------------------------------
	protected void PushDesktopDelayed()
	{
		m_fLastMirrorTime = GetGame().GetWorld().GetWorldTime() / 1000.0;
		m_sScreenText = m_sPendingDesktop;
		BroadcastScreen();
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: players within the screen range (capped by the viewer setting) receive the mirror.
	protected void UpdateViewers()
	{
		ROOT_AER_Settings s = ROOT_AER_SettingsManager.Get();
		if (!s.m_bUiOnTexture || m_iScreen == ROOT_AER_EScreen.OFF || !ROOT_AER_ScreenComponent.Of(GetOwner()) || ROOT_AER_Util.IsCarried(GetOwner()))
		{
			m_aViewers.Clear();
			return;
		}

		float range = Math.Max(s.m_fUiPlayerRange, 1) + 1;
		vector pos = GetOwner().GetOrigin();
		array<int> players = {};
		GetGame().GetPlayerManager().GetPlayers(players);
		array<int> now = {};
		array<float> dist = {};
		foreach (int id : players)
		{
			if (id == m_iOperator)
				continue;

			IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(id);
			if (!ent)
				continue;

			float d = vector.Distance(ent.GetOrigin(), pos);
			if (d > range)
				continue;

			int at = 0;
			while (at < dist.Count() && dist[at] <= d)
			{
				at++;
			}

			dist.InsertAt(d, at);
			now.InsertAt(id, at);
		}

		if (s.m_iUiMaxConcurrentViewers >= 0)
			now.Resize(Math.Min(now.Count(), s.m_iUiMaxConcurrentViewers));

		foreach (int id : now)
		{
			if (!m_aViewers.Contains(id))
				SendScreenTo(id);
		}

		m_aViewers.Copy(now);
	}

	//------------------------------------------------------------------------------------------------
	protected void SendScreenTo(int playerId)
	{
		ROOT_AER_DeviceComponent dev = ROOT_AER_DeviceComponent.Of(GetOwner());
		if (!dev)
			return;

		int id = dev.GetRplId();
		array<string> f = {id.ToString(), m_sScreenText, m_sScreenInput};
		ROOT_AER_Net.SendToPlayer(playerId, ROOT_AER_EMsg.SCREEN, ROOT_AER_Codec.Join(f));
	}

	//------------------------------------------------------------------------------------------------
	protected void BroadcastScreen()
	{
		foreach (int id : m_aViewers)
		{
			SendScreenTo(id);
		}

		OnScreenRpl();
	}

	//------------------------------------------------------------------------------------------------
	//! Client: mirror received from the server.
	void ApplyScreenInput(string input)
	{
		if (IsAuthority())
			return;

		m_sScreenInput = input;
		OnScreenRpl();
	}

	//------------------------------------------------------------------------------------------------
	void ApplyScreen(string text, string input)
	{
		if (IsAuthority())
			return;

		m_sScreenText = text;
		m_sScreenInput = input;
		OnScreenRpl();
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: typed input line mirror.
	void SetScreenInput(string input)
	{
		input = ROOT_AER_Codec.Clean(input);
		if (m_sScreenInput == input)
			return;

		m_sScreenInput = input;
		ROOT_AER_DeviceComponent dev = ROOT_AER_DeviceComponent.Of(GetOwner());
		if (dev)
		{
			int id = dev.GetRplId();
			array<string> f = {id.ToString(), m_sScreenInput};
			string msg = ROOT_AER_Codec.Join(f);
			foreach (int viewer : m_aViewers)
			{
				ROOT_AER_Net.SendToPlayer(viewer, ROOT_AER_EMsg.SCREEN_INPUT, msg);
			}
		}

		OnScreenRpl();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnScreenRpl()
	{
		ROOT_AER_ScreenComponent screen = ROOT_AER_ScreenComponent.Of(GetOwner());
		if (screen)
			screen.OnComputerChanged();
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: writes the battery level to /sys/battery/capacity.
	void UpdateBatteryFile()
	{
		if (!m_Fs)
			return;

		ROOT_AER_PowerComponent p = GetPower();
		string text = "100";
		if (p && p.GetCapacityKWh() > 0)
			text = Math.Round(p.GetLevel() * 100).ToString();

		ROOT_AER_FsNode n;
		string e;
		n = m_Fs.Resolve("/sys/battery/capacity", "root", e);
		if (n)
			n.m_sContent = text;
	}
}
