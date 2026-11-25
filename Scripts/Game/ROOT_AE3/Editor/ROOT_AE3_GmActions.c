//! Game Master actions on devices. Fields() describes the dialog, Execute() runs on the server after a
//! Game Master permission check. Values arrive in field order.
class ROOT_AE3_GmActions
{
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
			case "add_media": title = "Add image"; labels.Insert("Path"); labels.Insert("Texture {GUID}path.edds"); labels.Insert("Caption"); defaults.Insert("/home/user/photo.img"); defaults.Insert(""); defaults.Insert(""); return true;
			case "add_calendar": title = "Add calendar event"; labels.Insert("Date YYYY-MM-DD"); labels.Insert("Time HH:MM"); labels.Insert("Title"); labels.Insert("Description"); defaults.Insert(ROOT_AE3_Util.Today()); defaults.Insert("12:00"); defaults.Insert(""); defaults.Insert(""); return true;
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

		foreach (IEntity e : targets)
		{
			if (!e || !Applies(key, e))
				continue;

			ExecuteOne(playerId, key, e, v);
			if (key == "open_root")
				break;
		}
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
	protected static void ExecuteOne(int playerId, string key, IEntity e, array<string> v)
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
			case "add_user": ROOT_AE3_API.AddUser(e, V(v, 0), V(v, 1), V(v, 2) == "1"); break;
			case "add_sudoer": ROOT_AE3_API.AddSudoer(e, V(v, 0)); break;
			case "remove_sudoer": ROOT_AE3_API.RemoveSudoer(e, V(v, 0)); break;
			case "root_password": ROOT_AE3_API.SetRootPassword(e, V(v, 0)); break;
			case "root_login": ROOT_AE3_API.SetRootLogin(e, V(v, 0).ToInt()); break;
			case "hostname": ROOT_AE3_API.SetHostname(e, V(v, 0)); break;
			case "interface":
			{
				int mode = -1;
				if (!V(v, 0).Trim().IsEmpty())
					mode = V(v, 0).ToInt();

				c.SetInterfaceMode(mode);
				break;
			}
			case "access":
			{
				array<string> cli = ROOT_AE3_Util.List(V(v, 0));
				array<string> gui = ROOT_AE3_Util.List(V(v, 1));
				if (V(v, 0) != "*")
					c.SetAccess(0, cli);

				if (V(v, 1) != "*")
					c.SetAccess(1, gui);
				break;
			}

			case "user_speed": c.SetPref(V(v, 0), "speed", V(v, 1)); break;
			case "add_file": ROOT_AE3_API.AddFile(e, V(v, 0), V(v, 1), V(v, 2), V(v, 3), V(v, 4).ToInt(), V(v, 5), V(v, 6), V(v, 7).ToInt()); break;
			case "add_dir": ROOT_AE3_API.AddDirectory(e, V(v, 0), V(v, 1), V(v, 2)); break;
			case "add_email": ROOT_AE3_API.AddEmail(e, V(v, 0), V(v, 1), V(v, 2), V(v, 3)); break;
			case "mail_address": ROOT_AE3_API.AddMailAddress(e, V(v, 0)); break;
			case "chat_handle": ROOT_AE3_API.SetChatHandle(e, V(v, 0)); break;
			case "add_webpage":
			{
				if (V(v, 3) == "1")
					ROOT_AE3_API.RegisterWebpage(V(v, 0), V(v, 1), V(v, 2));
				else
					ROOT_AE3_API.RegisterWebpage(V(v, 0), V(v, 1), V(v, 2), e);

				break;
			}

			case "add_history": ROOT_AE3_API.AddBrowserHistory(e, V(v, 0)); break;
			case "add_media": ROOT_AE3_API.AddMedia(e, V(v, 0), V(v, 1), V(v, 2)); break;
			case "add_calendar": ROOT_AE3_API.AddCalendarEvent(e, V(v, 0), V(v, 1), V(v, 2), V(v, 3)); break;
			case "add_games":
			{
				array<string> names = ROOT_AE3_Util.Words(V(v, 0));
				ROOT_AE3_API.AddGames(e, names);
				break;
			}

			case "network_speed": ROOT_AE3_API.SetTransferSpeed(e, V(v, 0).ToFloat()); break;
			case "static_ip": ROOT_AE3_API.SetStaticIp(e, V(v, 0)); break;
			case "battery": ROOT_AE3_API.SetBatteryLevel(e, V(v, 0).ToFloat() / 100); break;
			case "fuel": ROOT_AE3_API.SetFuel(e, V(v, 0).ToFloat()); break;
			case "power_on": ROOT_AE3_API.SetPowerState(e, 1); break;
			case "power_off": ROOT_AE3_API.SetPowerState(e, 0); break;
			case "standby": ROOT_AE3_API.SetPowerState(e, 2); break;
			case "crash": ROOT_AE3_API.CrashDevice(e); break;
			case "disconnect_power": ROOT_AE3_API.DisconnectPower(e); break;
			case "disconnect_network": ROOT_AE3_API.DisconnectNetwork(e); break;
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
				break;
			}

			case "save":
			{
				if (ROOT_AE3_API.SaveComputer(e, V(v, 0)))
					Notify(playerId, "Saved " + V(v, 0));
				else
					Notify(playerId, "Save failed.");

				break;
			}

			case "restore":
			{
				if (V(v, 0).IsEmpty())
				{
					array<string> saves = {};
					ROOT_AE3_SaveLoad.ListSaves(saves);
					Notify(playerId, "Saves: " + ROOT_AE3_Util.JoinFrom(saves, 0, ", "));
					break;
				}

				if (ROOT_AE3_API.RestoreComputer(e, V(v, 0)))
					Notify(playerId, "Restored " + V(v, 0));
				else
					Notify(playerId, "No save named " + V(v, 0));

				break;
			}

			case "open_root":
			{
				ROOT_AE3_Manager.Get().OpenAsRoot(playerId, c);
				break;
			}
		}
	}
}
