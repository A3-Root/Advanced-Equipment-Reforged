//! Public script API. Every function changes state and must be called on the authority (server, SP, listen host);
//! on a proxy it does nothing and returns false. Devices are passed as entities.
class ROOT_AE3_API
{
	protected static ref ScriptInvokerBase<ROOT_AE3_LoginInvoker> s_OnLogin;
	protected static ref ScriptInvokerBase<ROOT_AE3_CommandInvoker> s_OnCommand;
	protected static ref ScriptInvokerBase<ROOT_AE3_FileInvoker> s_OnFileChanged;

	//------------------------------------------------------------------------------------------------
	protected static ROOT_AE3_ComputerComponent Computer(IEntity device)
	{
		if (!Replication.IsServer())
			return null;

		ROOT_AE3_ComputerComponent c = ROOT_AE3_ComputerComponent.Of(device);
		if (!c || !c.GetFilesystem())
			return null;

		return c;
	}

	//------------------------------------------------------------------------------------------------
	//! Filesystem of a computer or a flash drive (server).
	protected static ROOT_AE3_Filesystem Fs(IEntity device)
	{
		if (!Replication.IsServer() || !device)
			return null;

		ROOT_AE3_ComputerComponent c = ROOT_AE3_ComputerComponent.Of(device);
		if (c)
			return c.GetFilesystem();

		ROOT_AE3_UsbDriveComponent u = ROOT_AE3_UsbDriveComponent.Of(device);
		if (u)
			return u.GetFilesystem();

		return null;
	}

	// Events -----------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Server. Params: computer, user.
	static ScriptInvokerBase<ROOT_AE3_LoginInvoker> GetOnLogin()
	{
		if (!s_OnLogin)
			s_OnLogin = new ScriptInvokerBase<ROOT_AE3_LoginInvoker>();

		return s_OnLogin;
	}

	//------------------------------------------------------------------------------------------------
	//! Server. Params: computer, user, program, arguments.
	static ScriptInvokerBase<ROOT_AE3_CommandInvoker> GetOnCommand()
	{
		if (!s_OnCommand)
			s_OnCommand = new ScriptInvokerBase<ROOT_AE3_CommandInvoker>();

		return s_OnCommand;
	}

	//------------------------------------------------------------------------------------------------
	//! Server. Params: computer, absolute path.
	static ScriptInvokerBase<ROOT_AE3_FileInvoker> GetOnFileChanged()
	{
		if (!s_OnFileChanged)
			s_OnFileChanged = new ScriptInvokerBase<ROOT_AE3_FileInvoker>();

		return s_OnFileChanged;
	}

	//------------------------------------------------------------------------------------------------
	//! Server. Params: device. Fired when a computer finished its setup.
	static ScriptInvokerBase<ROOT_AE3_DeviceInvoker> GetOnDeviceReady()
	{
		return ROOT_AE3_Manager.Get().GetOnDeviceReady();
	}

	//------------------------------------------------------------------------------------------------
	static void InvokeLogin(ROOT_AE3_ComputerComponent c, string user)
	{
		if (s_OnLogin)
			s_OnLogin.Invoke(c, user);
	}

	//------------------------------------------------------------------------------------------------
	static void InvokeCommand(ROOT_AE3_ComputerComponent c, string user, string program, array<string> args)
	{
		if (s_OnCommand)
			s_OnCommand.Invoke(c, user, program, args);
	}

	//------------------------------------------------------------------------------------------------
	static void InvokeFileChanged(ROOT_AE3_ComputerComponent c, string path)
	{
		if (s_OnFileChanged)
			s_OnFileChanged.Invoke(c, path);
	}

	// Users ------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	static bool AddUser(IEntity device, string user, string password, bool sudoer = false)
	{
		ROOT_AE3_ComputerComponent c = Computer(device);
		if (!c || !ROOT_AE3_Filesystem.ValidName(user) || user == "root")
			return false;

		c.AddUser(user, password);
		if (sudoer)
			c.AddSudoer(user);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool AddSudoer(IEntity device, string user)
	{
		ROOT_AE3_ComputerComponent c = Computer(device);
		if (!c)
			return false;

		c.AddSudoer(user);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool RemoveSudoer(IEntity device, string user)
	{
		ROOT_AE3_ComputerComponent c = Computer(device);
		if (!c)
			return false;

		c.RemoveSudoer(user);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool SetRootPassword(IEntity device, string password)
	{
		ROOT_AE3_ComputerComponent c = Computer(device);
		if (!c)
			return false;

		c.SetRootPassword(password);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! policy: 0 setting, 1 allow, 2 deny.
	static bool SetRootLogin(IEntity device, int policy)
	{
		ROOT_AE3_ComputerComponent c = Computer(device);
		if (!c)
			return false;

		c.SetRootLogin(policy);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Server. Device name shown in the inventory, the status action, network lists and the Game Master dialog.
	//! Printable ASCII, at most 32 characters; empty = settings fallback.
	static bool SetDeviceName(IEntity device, string name)
	{
		ROOT_AE3_DeviceComponent d = ROOT_AE3_DeviceComponent.Of(device);
		if (!d || !d.IsAuthority())
			return false;

		d.SetDeviceName(name);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Server. Mission-wide number or on/off setting by key (ROOT_AE3_Settings.SetNumber keys), sent to clients.
	static bool SetGlobalNumber(string key, float value)
	{
		if (!Replication.IsServer() || !ROOT_AE3_SettingsManager.Get().SetNumber(key, value))
			return false;

		ROOT_AE3_SettingsManager.Broadcast();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Server. Mission-wide text setting: "root_password" (default root password), "ewo_prefabs" (EWO pack prefab names).
	static bool SetGlobalText(string key, string value)
	{
		if (!Replication.IsServer() || !ROOT_AE3_SettingsManager.Get().SetText(key, value))
			return false;

		ROOT_AE3_SettingsManager.Broadcast();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool SetHostname(IEntity device, string hostname)
	{
		ROOT_AE3_ComputerComponent c = Computer(device);
		if (!c)
			return false;

		c.SetHostname(hostname);
		return true;
	}

	// Files ------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Creates or overwrites a file. perms like "rw-r--". encryption 0 none, 1 caesar, 2 columnar.
	static bool AddFile(IEntity device, string path, string content, string owner = "root", string perms = "rw-r--", int encryption = 0, string key = "", string password = "", int size = -1)
	{
		ROOT_AE3_Filesystem fs = Fs(device);
		if (!fs)
			return false;

		ROOT_AE3_FileEntry f = new ROOT_AE3_FileEntry();
		f.m_sPath = path;
		f.m_sContent = content;
		f.m_sOwner = owner;
		f.m_sPermissions = perms;
		f.m_iEncryption = encryption;
		f.m_sKey = key;
		f.m_sPassword = password;
		f.m_iSize = size;
		string err;
		return f.Apply(fs, err);
	}

	//------------------------------------------------------------------------------------------------
	static bool AddDirectory(IEntity device, string path, string owner = "root", string perms = "rwxr-x")
	{
		ROOT_AE3_Filesystem fs = Fs(device);
		if (!fs)
			return false;

		ROOT_AE3_FileEntry f = new ROOT_AE3_FileEntry();
		f.m_sPath = path;
		f.m_bDirectory = true;
		f.m_sOwner = owner;
		f.m_sPermissions = perms;
		string err;
		return f.Apply(fs, err);
	}

	//------------------------------------------------------------------------------------------------
	static bool ReadFile(IEntity device, string path, out string content)
	{
		ROOT_AE3_Filesystem fs = Fs(device);
		if (!fs)
			return false;

		string err;
		return fs.ReadFile(path, "root", content, err);
	}

	//------------------------------------------------------------------------------------------------
	static bool RemovePath(IEntity device, string path)
	{
		ROOT_AE3_Filesystem fs = Fs(device);
		if (!fs)
			return false;

		string err;
		return fs.Remove(path, "root", true, err);
	}

	//------------------------------------------------------------------------------------------------
	//! Image file for the desktop Photos app.
	//! source: texture resource, http(s) address or base64 image text (PNG, JPEG, GIF; data: URI allowed).
	static bool AddMedia(IEntity device, string path, string source, string caption = "")
	{
		return AddFile(device, path, ROOT_AE3_Intel.Media(ROOT_AE3_WebMedia.KindOf(source), source.Trim(), caption), "root", "rw-r--");
	}

	// Commands ---------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Registers a program and installs it on a computer (device null = register only).
	static bool AddCustomCommand(IEntity device, string name, string description, string manual, ROOT_AE3_CommandCallback callback, string path = "")
	{
		if (!Replication.IsServer() || !ROOT_AE3_Filesystem.ValidName(name) || !callback)
			return false;

		ROOT_AE3_CustomCommand cmd = new ROOT_AE3_CustomCommand();
		cmd.m_sName = name;
		cmd.m_sDescription = description;
		cmd.m_sManual = manual;
		cmd.m_sPath = path;
		cmd.m_Callback = callback;
		if (!device)
		{
			ROOT_AE3_CommandRegistry.Get().Register(cmd);
			return true;
		}

		ROOT_AE3_ComputerComponent c = Computer(device);
		if (!c)
			return false;

		c.AddCustomCommand(cmd);
		return ROOT_AE3_CommandRegistry.Get().Install(c, name);
	}

	//------------------------------------------------------------------------------------------------
	//! Installs an already registered program (including the games).
	static bool InstallCommand(IEntity device, string name)
	{
		ROOT_AE3_ComputerComponent c = Computer(device);
		return c && ROOT_AE3_CommandRegistry.Get().Install(c, name);
	}

	// Intel ------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	static bool AddMailAddress(IEntity device, string address)
	{
		ROOT_AE3_ComputerComponent c = Computer(device);
		return c && ROOT_AE3_Intel.RegisterMailAddress(c, address);
	}

	//------------------------------------------------------------------------------------------------
	static bool AddEmail(IEntity device, string from, string subject, string body, string to = "", string date = "")
	{
		ROOT_AE3_ComputerComponent c = Computer(device);
		if (!c)
			return false;

		if (to.IsEmpty() && !c.GetMailAddresses().IsEmpty())
			to = c.GetMailAddresses()[0];

		ROOT_AE3_Intel.StoreMail(c, from, to, subject, body, date);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool SetChatHandle(IEntity device, string handle)
	{
		ROOT_AE3_ComputerComponent c = Computer(device);
		return c && ROOT_AE3_Intel.RegisterHandle(c, handle);
	}

	//------------------------------------------------------------------------------------------------
	//! device null = page on every computer.
	static bool RegisterWebpage(string url, string title, string markdown, IEntity device = null)
	{
		if (!Replication.IsServer())
			return false;

		ROOT_AE3_ComputerComponent c;
		if (device)
		{
			c = Computer(device);
			if (!c)
				return false;
		}

		ROOT_AE3_Intel.RegisterWebpage(url, title, markdown, c);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool RegisterSite(string domain, notnull array<string> paths, notnull array<string> titles, notnull array<string> contents)
	{
		if (!Replication.IsServer())
			return false;

		ROOT_AE3_Intel.RegisterSite(domain, paths, titles, contents);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool AddBrowserHistory(IEntity device, string url, string date = "")
	{
		ROOT_AE3_ComputerComponent c = Computer(device);
		if (!c)
			return false;

		ROOT_AE3_Intel.AddBrowserHistory(c, url, date);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool AddCalendarEvent(IEntity device, string date, string time, string title, string text, string location = "")
	{
		ROOT_AE3_ComputerComponent c = Computer(device);
		if (!c)
			return false;

		c.AddCalendarEvent(date, time, title, text, location);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static int RemoveCalendarEvent(IEntity device, string date, string title = "")
	{
		ROOT_AE3_ComputerComponent c = Computer(device);
		if (!c)
			return 0;

		return c.RemoveCalendarEvent(date, title);
	}

	//------------------------------------------------------------------------------------------------
	//! Camera feed for the desktop CCTV app: any entity; the camera looks along its forward axis.
	static bool RegisterCamera(IEntity camera, string name)
	{
		if (!camera)
			return false;

		ROOT_AE3_Cctv.Register(camera, name);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Installs games on a computer. names empty = every game.
	static bool AddGames(IEntity device, array<string> names = null)
	{
		ROOT_AE3_ComputerComponent c = Computer(device);
		if (!c)
			return false;

		if (!names || names.IsEmpty())
		{
			ROOT_AE3_CommandRegistry.Get().InstallGames(c);
			return true;
		}

		foreach (string n : names)
		{
			ROOT_AE3_CommandRegistry.Get().Install(c, n);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Server: opens a table of game hosted by device with bots bot seats and starts it when the game allows.
	//! Returns the table number, -1 on failure.
	static int HostGameWithBots(IEntity device, string game, int bots = 1)
	{
		ROOT_AE3_ComputerComponent c = Computer(device);
		if (!c || !Replication.IsServer())
			return -1;

		ROOT_AE3_GameTableSystem sys = ROOT_AE3_GameTableSystem.Get();
		string status;
		ROOT_AE3_GameTable t = sys.Host(c, game, status);
		if (!t)
			return -1;

		for (int i = 0; i < bots; i++)
		{
			if (!sys.AddBot(c, t.m_iId, status))
				break;
		}

		sys.StartTable(c, t.m_iId, status);
		return t.m_iId;
	}

	// Power and network ------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	static bool ConnectPower(IEntity consumer, IEntity source)
	{
		if (!Replication.IsServer())
			return false;

		string reason;
		return ROOT_AE3_ConnectionComponent.ConnectPower(consumer, source, reason);
	}

	//------------------------------------------------------------------------------------------------
	static bool DisconnectPower(IEntity consumer)
	{
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(consumer);
		if (!Replication.IsServer() || !p)
			return false;

		p.Disconnect();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool ConnectNetwork(IEntity device, IEntity router)
	{
		if (!Replication.IsServer())
			return false;

		string reason;
		return ROOT_AE3_ConnectionComponent.ConnectNetwork(device, router, reason);
	}

	//------------------------------------------------------------------------------------------------
	static bool DisconnectNetwork(IEntity device)
	{
		ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(device);
		if (!Replication.IsServer() || !n)
			return false;

		if (n.IsRouter())
			ROOT_AE3_NetworkSystem.Get().UnlinkRouter(n);
		else
			ROOT_AE3_NetworkSystem.Get().Disconnect(n);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! state: 0 off, 1 on, 2 standby.
	static bool SetPowerState(IEntity device, int state)
	{
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(device);
		if (!Replication.IsServer() || !p)
			return false;

		string reason;
		if (state == 1)
			return p.TurnOn(reason);

		if (state == 2)
			return p.Standby(reason);

		ROOT_AE3_ComputerComponent c = ROOT_AE3_ComputerComponent.Of(device);
		if (c && c.IsRunning())
			c.Shutdown();
		else
			p.TurnOff();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool CrashDevice(IEntity device)
	{
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(device);
		if (!Replication.IsServer() || !p)
			return false;

		p.Crash();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Battery level 0-1 of a battery or internal battery.
	static bool SetBatteryLevel(IEntity device, float level)
	{
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(device);
		if (!Replication.IsServer() || !p)
			return false;

		p.SetLevel(level);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool SetFuel(IEntity generator, float liters)
	{
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(generator);
		if (!Replication.IsServer() || !p)
			return false;

		p.SetFuel(liters);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Transfer speed in KB/s of a router or device (0 = default).
	static bool SetTransferSpeed(IEntity device, float kbps)
	{
		ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(device);
		if (!Replication.IsServer() || !n)
			return false;

		if (n.IsRouter())
			n.SetRouterSpeed(kbps);
		else
			n.SetDeviceSpeed(kbps);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Extra condition for terminal / desktop access on every computer (checked after the access lists).
	static void AddAccessCondition(ROOT_AE3_AccessCondition condition)
	{
		ROOT_AE3_ComputerComponent.AddAccessCondition(condition);
	}

	//------------------------------------------------------------------------------------------------
	static void RemoveAccessCondition(ROOT_AE3_AccessCondition condition)
	{
		ROOT_AE3_ComputerComponent.RemoveAccessCondition(condition);
	}

	//------------------------------------------------------------------------------------------------
	//! Condition every network link must pass (connect and wireless sweep).
	static void AddConnectCondition(ROOT_AE3_ConnectCondition condition)
	{
		ROOT_AE3_NetworkSystem.Get().AddConnectCondition(condition);
	}

	//------------------------------------------------------------------------------------------------
	static void RemoveConnectCondition(ROOT_AE3_ConnectCondition condition)
	{
		ROOT_AE3_NetworkSystem.Get().RemoveConnectCondition(condition);
	}

	//------------------------------------------------------------------------------------------------
	//! Modifier applied to every router's wireless range.
	static void AddRangeModifier(ROOT_AE3_RangeModifier modifier)
	{
		ROOT_AE3_NetworkSystem.Get().AddRangeModifier(modifier);
	}

	//------------------------------------------------------------------------------------------------
	static void RemoveRangeModifier(ROOT_AE3_RangeModifier modifier)
	{
		ROOT_AE3_NetworkSystem.Get().RemoveRangeModifier(modifier);
	}

	//------------------------------------------------------------------------------------------------
	//! mode -1 setting default, 0 terminal, 1 desktop, 2 both.
	static bool SetInterfaceMode(IEntity device, int mode)
	{
		ROOT_AE3_ComputerComponent c = Computer(device);
		if (!c)
			return false;

		c.SetInterfaceMode(mode);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! mode 0 terminal, 1 desktop; entries: identity IDs, faction keys or player names, empty = everyone.
	static bool SetInterfaceAccess(IEntity device, int mode, notnull array<string> entries)
	{
		ROOT_AE3_ComputerComponent c = Computer(device);
		if (!c)
			return false;

		c.SetAccess(mode, entries);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Permanent mains power on or off.
	static bool SetMainsPowered(IEntity device, bool mains)
	{
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(device);
		if (!Replication.IsServer() || !p)
			return false;

		p.SetMains(mains);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! ROOT_AE3_EPowerState of the device, -1 without power component.
	static int GetPowerState(IEntity device)
	{
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(device);
		if (!p)
			return -1;

		return p.GetState();
	}

	//------------------------------------------------------------------------------------------------
	//! Battery level 0-1, -1 without power component.
	static float GetBatteryLevel(IEntity device)
	{
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(device);
		if (!p)
			return -1;

		return p.GetLevel();
	}

	//------------------------------------------------------------------------------------------------
	//! Fuel in liters, -1 without power component.
	static float GetFuel(IEntity device)
	{
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(device);
		if (!p)
			return -1;

		return p.GetFuel();
	}

	//------------------------------------------------------------------------------------------------
	//! Static address inside the router subnet (empty = DHCP); reconnects a connected device.
	static bool SetStaticIp(IEntity device, string ip)
	{
		ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(device);
		if (!Replication.IsServer() || !n || n.IsRouter())
			return false;

		int a, b, cc, d;
		if (!ip.IsEmpty() && !ROOT_AE3_Util.ParseIp(ip, a, b, cc, d))
			return false;

		n.SetStaticIp(ip);
		ROOT_AE3_NetworkComponent r = n.GetParent();
		if (!r)
			return true;

		ROOT_AE3_ConnectionComponent wire = ROOT_AE3_ConnectionComponent.Of(device);
		bool wired = wire && wire.IsWiredTo(r.GetOwner());
		string reason;
		bool ok = ROOT_AE3_NetworkSystem.Get().Connect(n, r, r.GetPassword(), reason, true);
		if (ok && wired)
			wire.AddWired(r.GetOwner());

		return ok;
	}

	//------------------------------------------------------------------------------------------------
	static string GetAddress(IEntity device)
	{
		ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(device);
		if (!n)
			return "";

		return n.GetAddress();
	}

	// Desktop extension -------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Client: registers a desktop app class (subclass of ROOT_AE3_App) shown in the start menu.
	static void RegisterExtApp(typename appType)
	{
		ROOT_AE3_AppRegistry.RegisterExternal(appType);
	}

	//------------------------------------------------------------------------------------------------
	//! Drops event subscribers of the previous mission (game end).
	static void Reset()
	{
		s_OnLogin = null;
		s_OnCommand = null;
		s_OnFileChanged = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Server: desktop request name handled by handler; an extension app sends it with ROOT_AE3_App.Request.
	static void RegisterGuiCommand(string cmd, ROOT_AE3_GuiCommand handler)
	{
		ROOT_AE3_GuiRouter.RegisterExternal(cmd, handler);
	}

	// Save / restore ---------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Saves the computer state under a name ($profile:ROOT_AE3/saves/<name>.txt).
	static bool SaveComputer(IEntity device, string name)
	{
		ROOT_AE3_ComputerComponent c = Computer(device);
		return c && ROOT_AE3_SaveLoad.Save(c, name);
	}

	//------------------------------------------------------------------------------------------------
	static bool RestoreComputer(IEntity device, string name)
	{
		ROOT_AE3_ComputerComponent c = Computer(device);
		return c && ROOT_AE3_SaveLoad.Restore(c, name);
	}
}

void ROOT_AE3_LoginInvoker(ROOT_AE3_ComputerComponent computer, string user);
typedef func ROOT_AE3_LoginInvoker;

void ROOT_AE3_CommandInvoker(ROOT_AE3_ComputerComponent computer, string user, string program, array<string> args);
typedef func ROOT_AE3_CommandInvoker;

void ROOT_AE3_FileInvoker(ROOT_AE3_ComputerComponent computer, string path);
typedef func ROOT_AE3_FileInvoker;
