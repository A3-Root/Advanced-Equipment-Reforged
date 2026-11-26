//! Game Master actions on devices. Fields() describes the dialog, Execute() runs on the server after a
//! Game Master permission check. Values arrive in field order.
class ROOT_AE3_GmActions
{
	//------------------------------------------------------------------------------------------------
	//! Panel entries in display order: action key, label, group.
	static void List(notnull array<string> keys, notnull array<string> labels, notnull array<string> groups)
	{
		array<string> table = {
			"add_user", "Add user", "Users",
			"add_sudoer", "Add sudoer", "Users",
			"remove_sudoer", "Remove sudoer", "Users",
			"user_speed", "User transfer speed", "Users",
			"root_password", "Root password", "Users",
			"root_login", "Root login policy", "Users",
			"access", "Interface access", "Users",
			"global_root_password", "Default root password", "Mission settings",
			"global_ewo_prefabs", "EWO pack prefabs", "Mission settings",
			"rename", "Rename device", "System",
			"hostname", "Hostname", "System",
			"interface", "Interface mode", "System",
			"open_root", "Open interface as root", "System",
			"crash", "Crash", "System",
			"save", "Save computer", "System",
			"restore", "Restore computer", "System",
			"add_file", "Add file", "Files and intel",
			"add_dir", "Add directory", "Files and intel",
			"add_media", "Add image", "Files and intel",
			"add_email", "Add email", "Files and intel",
			"mail_address", "Add mail address", "Files and intel",
			"chat_handle", "Chat handle", "Files and intel",
			"add_webpage", "Add web page", "Files and intel",
			"add_history", "Add browser history", "Files and intel",
			"add_calendar", "Add calendar event", "Files and intel",
			"add_games", "Install games", "Files and intel",
			"power_on", "Power on", "Power",
			"power_off", "Power off", "Power",
			"standby", "Standby", "Power",
			"battery", "Battery level", "Power",
			"fuel", "Fuel", "Power",
			"connect_power", "Connect power (selection)", "Power",
			"disconnect_power", "Disconnect power", "Power",
			"router", "Router settings", "Network",
			"static_ip", "Static IP", "Network",
			"network_speed", "Transfer speed", "Network",
			"connect_network", "Connect network (selection)", "Network",
			"disconnect_network", "Disconnect network", "Network"
		};

		for (int i = 0; i + 2 < table.Count(); i += 3)
		{
			keys.Insert(table[i]);
			labels.Insert(table[i + 1]);
			groups.Insert(table[i + 2]);
		}
	}

	//------------------------------------------------------------------------------------------------
	static string Label(string key)
	{
		array<string> keys = {};
		array<string> labels = {};
		array<string> groups = {};
		List(keys, labels, groups);
		int i = keys.Find(key);
		if (i < 0)
			return key;

		return labels[i];
	}

	//------------------------------------------------------------------------------------------------
	//! True when the action can run on one of the devices (connect actions need two).
	static bool AppliesToAny(string key, notnull array<IEntity> ents)
	{
		if (key == "connect_power" || key == "connect_network")
			return ents.Count() >= 2;

		foreach (IEntity e : ents)
		{
			if (Applies(key, e))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Title, field labels and defaults of an action; false when the action needs no dialog.
	static bool Fields(string key, out string title, notnull array<string> labels, notnull array<string> defaults)
	{
		switch (key)
		{
			case "add_user": title = "Add user"; labels.Insert("User name"); labels.Insert("Password"); labels.Insert("Sudoer (0/1)"); defaults.Insert("user"); defaults.Insert("password"); defaults.Insert("0"); return true;
			case "add_sudoer": title = "Add sudoer"; labels.Insert("User name"); defaults.Insert("user"); return true;
			case "remove_sudoer": title = "Remove sudoer"; labels.Insert("User name"); defaults.Insert("user"); return true;
			case "root_password": title = "Root password"; labels.Insert("Password (empty = setting default)"); defaults.Insert(""); return true;
			case "root_login": title = "Root login"; labels.Insert("0 setting, 1 allow, 2 deny"); defaults.Insert("0"); return true;
			case "global_root_password":
			{
				string pw;
				ROOT_AE3_SettingsManager.Get().GetText("root_password", pw);
				title = "Default root password";
				labels.Insert("Root password of computers without their own");
				defaults.Insert(pw);
				return true;
			}

			case "global_ewo_prefabs":
			{
				string prefabs;
				ROOT_AE3_SettingsManager.Get().GetText("ewo_prefabs", prefabs);
				title = "EWO pack prefabs";
				labels.Insert("Prefab names, comma separated");
				defaults.Insert(prefabs);
				return true;
			}

			case "rename": title = "Rename device"; labels.Insert("Name (empty = default)"); defaults.Insert(""); return true;
			case "hostname": title = "Hostname"; labels.Insert("Hostname"); defaults.Insert(""); return true;
			case "interface": title = "Interface"; labels.Insert("-1 setting, 0 terminal, 1 desktop, 2 both"); defaults.Insert("-1"); return true;
			case "add_file":
			{
				title = "Add file";
				labels.Insert("Path");
				labels.Insert("Content");
				labels.Insert("Owner");
				labels.Insert("Permissions (rwxr-x)");
				labels.Insert("Encryption 0 none, 1 caesar, 2 columnar");
				labels.Insert("Key");
				labels.Insert("Password (empty = none)");
				labels.Insert("Virtual size in bytes (-1 = content)");
				defaults.Insert("/home/user/notes.txt");
				defaults.Insert("");
				defaults.Insert("root");
				defaults.Insert("rw-r--");
				defaults.Insert("0");
				defaults.Insert("");
				defaults.Insert("");
				defaults.Insert("-1");
				return true;
			}

			case "add_dir": title = "Add directory"; labels.Insert("Path"); labels.Insert("Owner"); labels.Insert("Permissions (rwxr-x)"); defaults.Insert("/home/user/intel"); defaults.Insert("root"); defaults.Insert("rwxr-x"); return true;
			case "add_email": title = "Add email"; labels.Insert("From"); labels.Insert("Subject"); labels.Insert("Text"); labels.Insert("To (empty = first address)"); defaults.Insert("hq@rootnet.home"); defaults.Insert(""); defaults.Insert(""); defaults.Insert(""); return true;
			case "mail_address": title = "Mail address"; labels.Insert("Address"); defaults.Insert("user@rootnet.home"); return true;
			case "chat_handle": title = "Chat handle"; labels.Insert("Handle"); defaults.Insert(""); return true;
			case "add_webpage": title = "Add web page"; labels.Insert("Address"); labels.Insert("Title"); labels.Insert("Markdown content"); labels.Insert("Every computer (0/1)"); defaults.Insert("intel.local"); defaults.Insert(""); defaults.Insert("# Title"); defaults.Insert("0"); return true;
			case "add_history": title = "Add browser history"; labels.Insert("Address"); defaults.Insert("intel.local"); return true;
			case "add_media": title = "Add image"; labels.Insert("Path"); labels.Insert("Texture {GUID}path.edds, web address or base64"); labels.Insert("Caption"); defaults.Insert("/home/user/photo.img"); defaults.Insert(""); defaults.Insert(""); return true;
			case "add_calendar": title = "Add calendar event"; labels.Insert("Date YYYY-MM-DD"); labels.Insert("Time HH:MM"); labels.Insert("Title"); labels.Insert("Description"); labels.Insert("Location"); defaults.Insert(ROOT_AE3_Util.Today()); defaults.Insert("12:00"); defaults.Insert(""); defaults.Insert(""); defaults.Insert(""); return true;
			case "add_games": title = "Install games"; labels.Insert("Games separated by spaces (empty = all)"); defaults.Insert(""); return true;
			case "static_ip": title = "Static IP"; labels.Insert("Address inside the router subnet (empty = DHCP)"); defaults.Insert(""); return true;
			case "network_speed": title = "Transfer speed"; labels.Insert("KB/s (0 = default)"); defaults.Insert("0"); return true;
			case "battery": title = "Battery level"; labels.Insert("Percent 0-100"); defaults.Insert("100"); return true;
			case "fuel": title = "Fuel"; labels.Insert("Liters"); defaults.Insert("5"); return true;
			case "router":
			{
				title = "Router settings";
				labels.Insert("SSID (empty = keep)");
				labels.Insert("Password (* = keep, empty = open)");
				labels.Insert("Range m (0 = keep)");
				labels.Insert("Accept connections (0/1, * = keep)");
				labels.Insert("External SSH (0/1, * = keep)");
				labels.Insert("External SSH allow list, regex separated by spaces (* = keep)");
				defaults.Insert("");
				defaults.Insert("*");
				defaults.Insert("0");
				defaults.Insert("*");
				defaults.Insert("*");
				defaults.Insert("*");
				return true;
			}

			case "access":
			{
				title = "Interface access";
				labels.Insert("Terminal: identity IDs / faction keys / player names, comma or space separated (empty = everyone, * = keep)");
				labels.Insert("Desktop: identity IDs / faction keys / player names, comma or space separated (empty = everyone, * = keep)");
				defaults.Insert("*");
				defaults.Insert("*");
				return true;
			}

			case "user_speed": title = "User transfer speed"; labels.Insert("User"); labels.Insert("KB/s (0 = default)"); defaults.Insert("user"); defaults.Insert("0"); return true;
			case "save": title = "Save computer"; labels.Insert("Save name"); defaults.Insert("laptop1"); return true;
			case "restore": title = "Restore computer"; labels.Insert("Save name (empty = list saves)"); defaults.Insert(""); return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Which devices an action applies to: computer, power, router, generator, battery, network, any.
	static string Target(string key)
	{
		switch (key)
		{
			case "power_on":
			case "power_off":
			case "connect_power":
			case "disconnect_power":
				return "power";

			case "add_file":
			case "add_dir":
			case "add_media":
				return "storage";

			case "rename": return "any";
			case "global_root_password":
			case "global_ewo_prefabs":
				return "module";

			case "battery": return "battery";
			case "fuel": return "generator";
			case "router": return "router";
			case "connect_network":
			case "disconnect_network":
			case "network_speed":
			case "static_ip":
				return "network";
		}

		return "computer";
	}

	//------------------------------------------------------------------------------------------------
	static bool Applies(string key, IEntity e)
	{
		switch (Target(key))
		{
			case "computer": return ROOT_AE3_ComputerComponent.Of(e) != null;
			case "storage": return ROOT_AE3_ComputerComponent.Of(e) != null || ROOT_AE3_UsbDriveComponent.Of(e) != null;
			case "power": return ROOT_AE3_PowerComponent.Of(e) != null;
			case "battery":
			{
				ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(e);
				return p && p.GetCapacityKWh() > 0;
			}

			case "generator":
			{
				ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(e);
				return p && p.GetRole() == ROOT_AE3_EPowerRole.GENERATOR;
			}

			case "router":
			{
				ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(e);
				return n && n.IsRouter();
			}

			case "network": return ROOT_AE3_NetworkComponent.Of(e) != null;
			case "any": return ROOT_AE3_DeviceComponent.Of(e) != null;
			case "module": return ROOT_AE3_SystemModuleComponent.IsActiveModule(e);
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsGameMaster(int playerId)
	{
		if (!Replication.IsRunning())
			return true;

		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return false;

		SCR_EditorManagerEntity em = core.GetEditorManager(playerId);
		return em && !em.IsLimited();
	}

	//------------------------------------------------------------------------------------------------
	protected static string V(array<string> v, int i)
	{
		if (i < v.Count())
			return v[i];

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Server. targets: device entities of the selection.
	static void Execute(int playerId, string key, notnull array<IEntity> targets, notnull array<string> v)
	{
		if (!IsGameMaster(playerId))
			return;

		if (key == "connect_power" || key == "connect_network")
		{
			Connect(playerId, key, targets);
			return;
		}

		int done, total;
		foreach (IEntity e : targets)
		{
			if (!e || !Applies(key, e))
				continue;

			total++;
			int result = ExecuteOne(playerId, key, e, v);
			if (result < 0)
				return;

			done += result;
			if (key == "open_root")
				break;
		}

		if (total == 0)
			Notify(playerId, Label(key) + ": no matching device selected.");
		else if (done == total)
			Notify(playerId, string.Format("%1: done (%2 devices).", Label(key), done));
		else
			Notify(playerId, string.Format("%1: failed on %2 of %3 devices. Check the values.", Label(key), total - done, total));
	}

	//------------------------------------------------------------------------------------------------
	//! Links the first selected source / router with every other selected device.
	protected static void Connect(int playerId, string key, array<IEntity> targets)
	{
		IEntity hub;
		foreach (IEntity e : targets)
		{
			if (key == "connect_power")
			{
				ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(e);
				if (p && p.IsProvider() && p.GetRole() != ROOT_AE3_EPowerRole.BATTERY && !hub)
					hub = e;
			}
			else
			{
				ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(e);
				if (n && n.IsRouter() && !hub)
					hub = e;
			}
		}

		if (!hub && key == "connect_power")
		{
			foreach (IEntity e : targets)
			{
				ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(e);
				if (p && p.IsProvider() && !hub)
					hub = e;
			}
		}

		if (!hub)
		{
			Notify(playerId, "Select a power source or router together with the devices.");
			return;
		}

		int linked;
		string reason;
		foreach (IEntity e : targets)
		{
			if (e == hub)
				continue;

			bool ok;
			if (key == "connect_power")
				ok = ROOT_AE3_ConnectionComponent.ConnectPower(e, hub, reason);
			else
				ok = ROOT_AE3_ConnectionComponent.ConnectNetwork(e, hub, reason);

			if (ok)
				linked++;
		}

		Notify(playerId, string.Format("%1 devices connected. %2", linked, reason));
	}

	//------------------------------------------------------------------------------------------------
	protected static void Notify(int playerId, string text)
	{
		array<string> f = {"Advanced Equipment", text};
		ROOT_AE3_Net.SendToPlayer(playerId, ROOT_AE3_EMsg.NOTIFY, ROOT_AE3_Codec.Join(f));
	}

	//------------------------------------------------------------------------------------------------
	protected static int R(bool ok)
	{
		if (ok)
			return 1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns 1 done, 0 failed, -1 reported by the action itself.
	protected static int ExecuteOne(int playerId, string key, IEntity e, array<string> v)
	{
		ROOT_AE3_ComputerComponent c = ROOT_AE3_ComputerComponent.Of(e);
		ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(e);
		// Text fields take "\\n" for a line break (form fields are single-line).
		int textField = -1;
		if (key == "add_file")
			textField = 1;
		else if (key == "add_email" || key == "add_webpage")
			textField = 2;
		else if (key == "add_calendar")
			textField = 3;

		if (textField >= 0 && textField < v.Count())
		{
			string text = v[textField];
			text.Replace("\\n", "\n");
			v[textField] = text;
		}

		switch (key)
		{
			case "add_user": return R(ROOT_AE3_API.AddUser(e, V(v, 0), V(v, 1), V(v, 2) == "1"));
			case "add_sudoer": return R(ROOT_AE3_API.AddSudoer(e, V(v, 0)));
			case "remove_sudoer": return R(ROOT_AE3_API.RemoveSudoer(e, V(v, 0)));
			case "root_password": return R(ROOT_AE3_API.SetRootPassword(e, V(v, 0)));
			case "root_login": return R(ROOT_AE3_API.SetRootLogin(e, V(v, 0).ToInt()));
			case "rename": return R(ROOT_AE3_API.SetDeviceName(e, V(v, 0)));
			case "global_root_password": return R(ROOT_AE3_API.SetGlobalText("root_password", V(v, 0)));
			case "global_ewo_prefabs": return R(ROOT_AE3_API.SetGlobalText("ewo_prefabs", V(v, 0)));
			case "hostname": return R(ROOT_AE3_API.SetHostname(e, V(v, 0)));
			case "interface":
			{
				int mode = -1;
				if (!V(v, 0).Trim().IsEmpty())
					mode = V(v, 0).ToInt();

				return R(ROOT_AE3_API.SetInterfaceMode(e, mode));
			}

			case "access":
			{
				if (V(v, 0) != "*")
					c.SetAccess(0, ROOT_AE3_Util.List(V(v, 0)));

				if (V(v, 1) != "*")
					c.SetAccess(1, ROOT_AE3_Util.List(V(v, 1)));

				return 1;
			}

			case "user_speed": c.SetPref(V(v, 0), "speed", V(v, 1)); return 1;
			case "add_file": return R(ROOT_AE3_API.AddFile(e, V(v, 0), V(v, 1), V(v, 2), V(v, 3), V(v, 4).ToInt(), V(v, 5), V(v, 6), V(v, 7).ToInt()));
			case "add_dir": return R(ROOT_AE3_API.AddDirectory(e, V(v, 0), V(v, 1), V(v, 2)));
			case "add_email": return R(ROOT_AE3_API.AddEmail(e, V(v, 0), V(v, 1), V(v, 2), V(v, 3)));
			case "mail_address": return R(ROOT_AE3_API.AddMailAddress(e, V(v, 0)));
			case "chat_handle": return R(ROOT_AE3_API.SetChatHandle(e, V(v, 0)));
			case "add_webpage":
			{
				if (V(v, 3) == "1")
					return R(ROOT_AE3_API.RegisterWebpage(V(v, 0), V(v, 1), V(v, 2)));

				return R(ROOT_AE3_API.RegisterWebpage(V(v, 0), V(v, 1), V(v, 2), e));
			}

			case "add_history": return R(ROOT_AE3_API.AddBrowserHistory(e, V(v, 0)));
			case "add_media": return R(ROOT_AE3_API.AddMedia(e, V(v, 0), V(v, 1), V(v, 2)));
			case "add_calendar": return R(ROOT_AE3_API.AddCalendarEvent(e, V(v, 0), V(v, 1), V(v, 2), V(v, 3), V(v, 4)));
			case "add_games": return R(ROOT_AE3_API.AddGames(e, ROOT_AE3_Util.Words(V(v, 0))));
			case "network_speed": return R(ROOT_AE3_API.SetTransferSpeed(e, V(v, 0).ToFloat()));
			case "static_ip": return R(ROOT_AE3_API.SetStaticIp(e, V(v, 0)));
			case "battery": return R(ROOT_AE3_API.SetBatteryLevel(e, V(v, 0).ToFloat() / 100));
			case "fuel": return R(ROOT_AE3_API.SetFuel(e, V(v, 0).ToFloat()));
			case "power_on": return R(ROOT_AE3_API.SetPowerState(e, 1));
			case "power_off": return R(ROOT_AE3_API.SetPowerState(e, 0));
			case "standby": return R(ROOT_AE3_API.SetPowerState(e, 2));
			case "crash": return R(ROOT_AE3_API.CrashDevice(e));
			case "disconnect_power": return R(ROOT_AE3_API.DisconnectPower(e));
			case "disconnect_network": return R(ROOT_AE3_API.DisconnectNetwork(e));
			case "router":
			{
				float range = V(v, 2).ToFloat();
				string pw = V(v, 1);
				if (pw == "*")
					pw = n.GetPassword();

				bool accept = V(v, 3) == "1";
				if (V(v, 3) == "*")
					accept = n.AcceptsConnections();

				bool ssh = V(v, 4) == "1";
				if (V(v, 4) == "*")
					ssh = n.AllowsExternalSsh();

				n.ConfigureRouter(V(v, 0), pw, range, -1, accept, ssh, -1);
				if (V(v, 5) != "*")
				{
					array<string> allow = ROOT_AE3_Util.Words(V(v, 5));
					n.SetExternalAllow(allow);
				}

				return 1;
			}

			case "save":
			{
				if (ROOT_AE3_API.SaveComputer(e, V(v, 0)))
					Notify(playerId, "Saved " + V(v, 0));
				else
					Notify(playerId, "Save failed.");

				return -1;
			}

			case "restore":
			{
				if (V(v, 0).IsEmpty())
				{
					array<string> saves = {};
					ROOT_AE3_SaveLoad.ListSaves(saves);
					Notify(playerId, "Saves: " + ROOT_AE3_Util.JoinFrom(saves, 0, ", "));
					return -1;
				}

				if (ROOT_AE3_API.RestoreComputer(e, V(v, 0)))
					Notify(playerId, "Restored " + V(v, 0));
				else
					Notify(playerId, "No save named " + V(v, 0));

				return -1;
			}

			case "open_root":
			{
				ROOT_AE3_Manager.Get().OpenAsRoot(playerId, c);
				return -1;
			}
		}

		return 0;
	}
}
