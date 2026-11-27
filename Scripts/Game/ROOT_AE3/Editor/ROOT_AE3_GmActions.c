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
			"global_web_proxy", "Web image proxy", "Mission settings",
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
			"ui_style", "Desktop style (civilian / military)", "System",
			"power_source", "Power source", "Power",
			"power_cable", "Power cable length", "Power",
			"connect_power", "Connect power (selection)", "Power",
			"disconnect_power", "Disconnect power", "Power",
			"router", "Router name and password", "Network",
			"connect_wifi", "Connect to network", "Network",
			"static_ip", "Static IP", "Network",
			"network_speed", "Transfer speed", "Network",
			"connect_network", "Connect selection to router", "Network",
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
	//! False for actions that run without a form.
	static bool HasFields(string key)
	{
		switch (key)
		{
			case "open_root":
			case "crash":
			case "power_on":
			case "power_off":
			case "standby":
			case "connect_power":
			case "connect_network":
			case "disconnect_power":
			case "disconnect_network":
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static ROOT_AE3_GmField F(notnull array<ref ROOT_AE3_GmField> fields, ROOT_AE3_EGmField kind, string label, string value)
	{
		ROOT_AE3_GmField f = ROOT_AE3_GmField.Make(kind, label, value);
		fields.Insert(f);
		return f;
	}

	//------------------------------------------------------------------------------------------------
	protected static ROOT_AE3_GmField Toggle(notnull array<ref ROOT_AE3_GmField> fields, string label, bool value)
	{
		ROOT_AE3_GmField f = ROOT_AE3_GmField.Toggle(label, value);
		fields.Insert(f);
		return f;
	}

	//------------------------------------------------------------------------------------------------
	//! Choice field; a text field when there is nothing to choose from.
	protected static ROOT_AE3_GmField Choice(notnull array<ref ROOT_AE3_GmField> fields, string label, notnull array<string> values, notnull array<string> labels, string value)
	{
		ROOT_AE3_GmField f;
		if (values.IsEmpty())
			f = ROOT_AE3_GmField.Make(ROOT_AE3_EGmField.TEXT, label, value);
		else
			f = ROOT_AE3_GmField.Choice(label, values, labels, value);

		fields.Insert(f);
		return f;
	}

	//------------------------------------------------------------------------------------------------
	//! root and the users of a computer (only root for other storage).
	protected static void Owners(ROOT_AE3_ComputerComponent c, notnull array<string> names)
	{
		names.Insert("root");
		if (!c)
			return;

		array<string> users = {};
		c.GetUsers(users);
		foreach (string u : users)
		{
			if (u != "root")
				names.Insert(u);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Fields of "Add file": where (desktop or home of a user, top folder, other folder), file name, content,
	//! who can open it, owner, protection (password, cipher or both), one key field per cipher, shown size.
	//! Field order is read by Apply ("add_file") and AddFilePath; owner "_new" creates the account (fields 6, 7).
	protected static void AddFileFields(ROOT_AE3_ComputerComponent c, notnull array<string> users, notnull array<string> owners, notnull array<ref ROOT_AE3_GmField> fields)
	{
		array<string> wv = {};
		array<string> wl = {};
		foreach (string u : users)
		{
			wv.Insert("d:" + u);
			wl.Insert("Desktop of " + u);
			wv.Insert("h:" + u);
			wl.Insert("Home folder of " + u);
		}

		wv.Insert("/");
		wl.Insert("Top folder (/)");
		wv.Insert("custom");
		wl.Insert("Other folder");
		string where = "/";
		if (!users.IsEmpty())
			where = "d:" + users[0];

		Choice(fields, "Where to put the file", wv, wl, where);
		F(fields, ROOT_AE3_EGmField.TEXT, "Folder (e.g. /home/user/intel)", "/intel").ShowIf(0, "custom");
		F(fields, ROOT_AE3_EGmField.TEXT, "File name (e.g. orders.txt)", "notes.txt");
		F(fields, ROOT_AE3_EGmField.MULTILINE, "Text in the file", "");
		array<string> av = {"all", "owner", "root"};
		array<string> al = {"Everyone", "Owner and root", "Root only"};
		Choice(fields, "Who can open it", av, al, "all");
		array<string> ov = {};
		array<string> ol = {};
		ov.Copy(owners);
		ol.Copy(owners);
		ov.Insert("_new");
		ol.Insert("New account");
		Choice(fields, "Owner (user account)", ov, ol, "root").ShowIf(4, "all owner");
		F(fields, ROOT_AE3_EGmField.TEXT, "New account name", "").ShowIf(5, "_new");
		F(fields, ROOT_AE3_EGmField.PASSWORD, "New account password", "").ShowIf(5, "_new");
		array<string> pv = {"none", "password", "cipher", "both"};
		array<string> pl = {"None", "Password", "Cipher", "Password + cipher"};
		Choice(fields, "Protection", pv, pl, "none");
		F(fields, ROOT_AE3_EGmField.PASSWORD, "Password", "").ShowIf(8, "password both");
		array<string> cv = {};
		array<string> cl = {};
		for (int i = 1; i < ROOT_AE3_Encryption.COUNT; i++)
		{
			cv.Insert(i.ToString());
			cl.Insert(ROOT_AE3_Encryption.Label(i));
		}

		Choice(fields, "Cipher", cv, cl, "1").ShowIf(8, "cipher both");
		F(fields, ROOT_AE3_EGmField.TEXT, "Caesar shift, a whole number", "3").ShowIf(10, ROOT_AE3_Encryption.CAESAR.ToString());
		F(fields, ROOT_AE3_EGmField.TEXT, "Key word, 2 letters or more", "KEY").ShowIf(10, ROOT_AE3_Encryption.COLUMNAR.ToString());
		F(fields, ROOT_AE3_EGmField.TEXT, "Key word", "LEMON").ShowIf(10, ROOT_AE3_Encryption.VIGENERE.ToString());
		array<string> rv = {"rot13", "rot5", "rot18", "rot47"};
		array<string> rl = {"ROT13 letters", "ROT5 digits", "ROT18 both", "ROT47 all"};
		Choice(fields, "ROT variant", rv, rl, "rot13").ShowIf(10, ROOT_AE3_Encryption.ROT.ToString());
		F(fields, ROOT_AE3_EGmField.TEXT, "Key a,b (a odd, not 13; b 0-25)", "5,8").ShowIf(10, ROOT_AE3_Encryption.AFFINE.ToString());
		F(fields, ROOT_AE3_EGmField.TEXT, "Number of rails, 2 or more", "3").ShowIf(10, ROOT_AE3_Encryption.RAILFENCE.ToString());
		array<string> bv = {"", "extended"};
		array<string> bl = {"24 letters", "26 letters"};
		Choice(fields, "Bacon alphabet", bv, bl, "").ShowIf(10, ROOT_AE3_Encryption.BACON.ToString());
		F(fields, ROOT_AE3_EGmField.TEXT, "Substitution alphabet, 26 letters", ROOT_AE3_Encryption.DefaultKey(ROOT_AE3_Encryption.SUBSTITUTION)).ShowIf(10, ROOT_AE3_Encryption.SUBSTITUTION.ToString());
		array<string> xv = {"16", "2", "8", "10"};
		array<string> xl = {"Hex", "Binary", "Octal", "Decimal"};
		Choice(fields, "Number base", xv, xl, "16").ShowIf(10, ROOT_AE3_Encryption.INTEGER.ToString());
		F(fields, ROOT_AE3_EGmField.TEXT, "Shown size in bytes (empty = real)", "");
	}

	//------------------------------------------------------------------------------------------------
	//! Path from the "Add file" values.
	protected static string AddFilePath(notnull array<string> v)
	{
		string where = V(v, 0);
		string folder = "/";
		if (where.StartsWith("d:"))
			folder = ROOT_AE3_Filesystem.HomeOf(where.Substring(2, where.Length() - 2)) + "/Desktop";
		else if (where.StartsWith("h:"))
			folder = ROOT_AE3_Filesystem.HomeOf(where.Substring(2, where.Length() - 2));
		else if (where == "custom")
			folder = V(v, 1).Trim();

		string name = V(v, 2).Trim();
		if (folder.EndsWith("/"))
			return folder + name;

		return folder + "/" + name;
	}

	//------------------------------------------------------------------------------------------------
	//! Server: title and fields of an action with the current values of the first selected device it applies
	//! to. No fields when no device applies.
	static void Fields(string key, notnull array<IEntity> ents, out string title, notnull array<ref ROOT_AE3_GmField> fields)
	{
		IEntity e;
		foreach (IEntity cand : ents)
		{
			if (cand && Applies(key, cand))
			{
				e = cand;
				break;
			}
		}

		if (!e || !HasFields(key))
			return;

		title = Label(key);
		ROOT_AE3_ComputerComponent c = ROOT_AE3_ComputerComponent.Of(e);
		ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(e);
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(e);
		ROOT_AE3_DeviceComponent d = ROOT_AE3_DeviceComponent.Of(e);
		array<string> users = {};
		if (c)
			c.GetUsers(users);

		switch (key)
		{
			case "add_user":
			{
				F(fields, ROOT_AE3_EGmField.TEXT, "User name", "user");
				F(fields, ROOT_AE3_EGmField.PASSWORD, "Password", "password");
				Toggle(fields, "Sudoer", false);
				return;
			}

			case "add_sudoer":
			case "remove_sudoer":
			case "user_speed":
			{
				array<string> names = {};
				if (key == "remove_sudoer")
					c.GetSudoers(names);
				else
					names.Copy(users);

				string first;
				if (!names.IsEmpty())
					first = names[0];

				Choice(fields, "User", names, names, first);
				if (key == "user_speed")
					F(fields, ROOT_AE3_EGmField.TEXT, "Transfer speed in KB/s (0 = default)", "0");

				return;
			}

			case "root_password":
			{
				F(fields, ROOT_AE3_EGmField.PASSWORD, "Root password (empty = mission default)", c.GetRootPasswordRaw());
				return;
			}

			case "root_login":
			{
				array<string> rv = {"0", "1", "2"};
				array<string> rl = {"Mission setting", "Allow", "Deny"};
				Choice(fields, "Direct root login", rv, rl, c.GetRootLoginPolicy().ToString());
				return;
			}

			case "access":
			{
				array<string> cli = {};
				array<string> gui = {};
				c.GetAccess(0, cli);
				c.GetAccess(1, gui);
				F(fields, ROOT_AE3_EGmField.TEXT, "Terminal: identity IDs, faction keys or player names, comma separated (empty = everyone)", ROOT_AE3_Util.JoinFrom(cli, 0, ", "));
				F(fields, ROOT_AE3_EGmField.TEXT, "Desktop: identity IDs, faction keys or player names, comma separated (empty = everyone)", ROOT_AE3_Util.JoinFrom(gui, 0, ", "));
				return;
			}

			case "global_root_password":
			case "global_ewo_prefabs":
			case "global_web_proxy":
			{
				string text;
				string label = "Root password of computers without their own";
				string setting = "root_password";
				ROOT_AE3_EGmField kind = ROOT_AE3_EGmField.PASSWORD;
				if (key == "global_ewo_prefabs")
				{
					label = "Prefab names of EWO packs, comma separated";
					setting = "ewo_prefabs";
					kind = ROOT_AE3_EGmField.TEXT;
				}
				else if (key == "global_web_proxy")
				{
					label = "Address that returns images as base64 text, %1 = image address (empty = direct)";
					setting = "web_image_proxy";
					kind = ROOT_AE3_EGmField.TEXT;
				}

				ROOT_AE3_SettingsManager.Get().GetText(setting, text);
				F(fields, kind, label, text);
				return;
			}

			case "rename": F(fields, ROOT_AE3_EGmField.TEXT, "Name (empty = default)", d.GetCustomName()); return;
			case "hostname": F(fields, ROOT_AE3_EGmField.TEXT, "Hostname", c.GetHostname()); return;
			case "interface":
			{
				array<string> iv = {"-1", "0", "1", "2"};
				array<string> il = {"Mission setting", "Terminal", "Desktop", "Both"};
				Choice(fields, "Interface", iv, il, c.GetInterfaceModeRaw().ToString());
				return;
			}

			case "add_file":
			case "add_dir":
			{
				array<string> owners = {};
				Owners(c, owners);
				string home = "/home/user";
				if (!users.IsEmpty())
					home = ROOT_AE3_Filesystem.HomeOf(users[0]);

				if (!c)
					home = "";

				if (key == "add_dir")
				{
					F(fields, ROOT_AE3_EGmField.TEXT, "Path", home + "/intel");
					Choice(fields, "Owner", owners, owners, "root");
					F(fields, ROOT_AE3_EGmField.PERMS, "Permissions", "rwxr-x");
					return;
				}

				AddFileFields(c, users, owners, fields);
				return;
			}

			case "add_email":
			{
				F(fields, ROOT_AE3_EGmField.TEXT, "From", "hq@rootnet.home");
				F(fields, ROOT_AE3_EGmField.TEXT, "Subject", "");
				F(fields, ROOT_AE3_EGmField.MULTILINE, "Text", "");
				F(fields, ROOT_AE3_EGmField.TEXT, "To (empty = first address)", "");
				return;
			}

			case "mail_address": F(fields, ROOT_AE3_EGmField.TEXT, "Address", "user@rootnet.home"); return;
			case "chat_handle": F(fields, ROOT_AE3_EGmField.TEXT, "Handle", ""); return;
			case "add_webpage":
			{
				F(fields, ROOT_AE3_EGmField.TEXT, "Address", "intel.local");
				F(fields, ROOT_AE3_EGmField.TEXT, "Title", "");
				F(fields, ROOT_AE3_EGmField.MULTILINE, "Markdown content", "# Title");
				Toggle(fields, "On every computer", false);
				return;
			}

			case "add_history": F(fields, ROOT_AE3_EGmField.TEXT, "Address", "intel.local"); return;
			case "add_media":
			{
				F(fields, ROOT_AE3_EGmField.TEXT, "Path", "/home/user/photo.img");
				F(fields, ROOT_AE3_EGmField.TEXT, "Texture {GUID}path.edds, web address or base64", "");
				F(fields, ROOT_AE3_EGmField.TEXT, "Caption", "");
				return;
			}

			case "add_calendar":
			{
				F(fields, ROOT_AE3_EGmField.TEXT, "Date YYYY-MM-DD", ROOT_AE3_Util.Today());
				F(fields, ROOT_AE3_EGmField.TEXT, "Time HH:MM", "12:00");
				F(fields, ROOT_AE3_EGmField.TEXT, "Title", "");
				F(fields, ROOT_AE3_EGmField.MULTILINE, "Description", "");
				F(fields, ROOT_AE3_EGmField.TEXT, "Location", "");
				return;
			}

			case "add_games": F(fields, ROOT_AE3_EGmField.TEXT, "Games separated by spaces (empty = all)", ""); return;
			case "static_ip": F(fields, ROOT_AE3_EGmField.TEXT, "Address inside the router subnet (empty = automatic)", n.GetStaticIp()); return;
			case "network_speed":
			{
				float speed = n.GetDeviceSpeed();
				if (n.IsRouter())
					speed = n.GetRouterSpeed();

				F(fields, ROOT_AE3_EGmField.TEXT, "Transfer speed in KB/s (0 = default)", Math.Round(speed).ToString());
				return;
			}

			case "battery": F(fields, ROOT_AE3_EGmField.TEXT, "Charge in percent 0-100", Math.Round(p.GetLevel() * 100).ToString()); return;
			case "fuel": F(fields, ROOT_AE3_EGmField.TEXT, "Fuel in liters", p.GetFuel().ToString()); return;
			case "ui_style":
			{
				array<string> uv = {"-1", "0", "1"};
				array<string> ul = {"Mission setting", "Civilian (Windows)", "Military (Tactical)"};
				Choice(fields, "Desktop style", uv, ul, c.GetUiStyleRaw().ToString());
				return;
			}

			case "power_cable": F(fields, ROOT_AE3_EGmField.TEXT, "Length in m of the power cable players take from this source", p.GetCableLength().ToString()); return;
			case "power_source":
			{
				array<string> sv = {""};
				array<string> sl = {"None (unplug)"};
				vector at = ROOT_AE3_Util.WorldPos(e);
				foreach (ROOT_AE3_DeviceComponent sd : ROOT_AE3_Manager.Get().GetDevices())
				{
					ROOT_AE3_PowerComponent sp;
					if (sd && sd.GetOwner() != e)
						sp = ROOT_AE3_PowerComponent.Of(sd.GetOwner());

					if (!sp || !sp.IsProvider() || sp.GetProvider() == p || ROOT_AE3_Util.IsCarried(sd.GetOwner()))
						continue;

					float dist = vector.Distance(at, sd.GetOwner().GetOrigin());
					if (dist > 500)
						continue;

					int sid = sd.GetRplId();
					sv.Insert(sid.ToString());
					sl.Insert(string.Format("%1 (%2 m, cable %3 m)", sd.GetDeviceName(), Math.Round(dist), sp.GetCableLength()));
				}

				string cur;
				ROOT_AE3_DeviceComponent pd;
				if (p.GetProvider())
					pd = ROOT_AE3_DeviceComponent.Of(p.GetProvider().GetOwner());

				if (pd)
				{
					int cid = pd.GetRplId();
					cur = cid.ToString();
				}

				Choice(fields, "Power source within 500 m (the cable reaches any distance)", sv, sl, cur);
				return;
			}
			case "router":
			{
				F(fields, ROOT_AE3_EGmField.TEXT, "Network name (SSID)", n.GetSsid());
				F(fields, ROOT_AE3_EGmField.PASSWORD, "Password (empty = open network)", n.GetPassword());
				string allow;
				if (n.GetExternalAllow())
					allow = ROOT_AE3_Util.JoinFrom(n.GetExternalAllow(), 0, "\n");

				F(fields, ROOT_AE3_EGmField.MULTILINE, "Source addresses allowed to SSH in from other networks, one regular expression per line (empty = all)", allow);
				return;
			}

			case "connect_wifi":
			{
				array<string> nv = {};
				array<string> nl = {};
				array<ROOT_AE3_NetworkComponent> inRange = {};
				vector pos = ROOT_AE3_Util.WorldPos(e);
				ROOT_AE3_NetworkSystem sys = ROOT_AE3_NetworkSystem.Get();
				sys.Scan(pos, inRange);
				foreach (ROOT_AE3_NetworkComponent r : sys.GetRouters())
				{
					if (!r || r == n || r.GetGateway().IsEmpty())
						continue;

					ROOT_AE3_DeviceComponent rd = ROOT_AE3_DeviceComponent.Of(r.GetOwner());
					if (rd && rd.IsHidden() && !inRange.Contains(r))
						continue;

					string rlabel = string.Format("%1 (%2, %3 m)", r.GetSsid(), r.GetGateway(), Math.Round(vector.Distance(pos, ROOT_AE3_Util.WorldPos(r.GetOwner()))));
					if (!inRange.Contains(r))
						rlabel += " out of range";

					nv.Insert(r.GetGateway());
					nl.Insert(rlabel);
				}

				string current;
				if (n.GetParent())
					current = n.GetParent().GetGateway();

				Choice(fields, "Network", nv, nl, current);
				array<string> mv = {"wifi", "cable"};
				array<string> ml = {"Wireless", "Cable"};
				Choice(fields, "Connection", mv, ml, "wifi");
				Toggle(fields, "Ignore range", false).ShowIf(1, "wifi");
				return;
			}

			case "save": F(fields, ROOT_AE3_EGmField.TEXT, "Save name", "laptop1"); return;
			case "restore":
			{
				array<string> saves = {};
				ROOT_AE3_SaveLoad.ListSaves(saves);
				string firstSave;
				if (!saves.IsEmpty())
					firstSave = saves[0];

				Choice(fields, "Save", saves, saves, firstSave);
				return;
			}
		}
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
			case "global_web_proxy":
				return "module";

			case "battery": return "battery";
			case "fuel": return "fuel";
			case "power_source": return "consumer";
			case "power_cable": return "source";
			case "router": return "router";
			case "connect_network":
			case "disconnect_network":
			case "network_speed":
			case "static_ip":
				return "network";

			case "connect_wifi": return "client";
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

			case "fuel":
			{
				ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(e);
				return p && p.GetFuelCapacity() > 0;
			}

			case "consumer":
			{
				ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(e);
				return p && !p.IsMains() && (p.GetRole() == ROOT_AE3_EPowerRole.CONSUMER || p.GetRole() == ROOT_AE3_EPowerRole.BATTERY);
			}

			case "source":
			{
				ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(e);
				return p && p.IsProvider();
			}

			case "router":
			{
				ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(e);
				return n && n.IsRouter();
			}

			case "network": return ROOT_AE3_NetworkComponent.Of(e) != null;
			case "client":
			{
				ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(e);
				return n && !n.IsRouter();
			}

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

		s_sReason = "";
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
		else if (!s_sReason.IsEmpty())
			Notify(playerId, string.Format("%1: failed on %2 of %3 devices: %4", Label(key), total - done, total, s_sReason));
		else
			Notify(playerId, string.Format("%1: failed on %2 of %3 devices. Check the values.", Label(key), total - done, total));
	}

	protected static string s_sReason;	//!< First failure text of the running Execute

	//------------------------------------------------------------------------------------------------
	protected static int Fail(string reason)
	{
		if (s_sReason.IsEmpty())
			s_sReason = reason;

		return 0;
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
	static void Notify(int playerId, string text)
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
			case "global_web_proxy": return R(ROOT_AE3_API.SetGlobalText("web_image_proxy", V(v, 0)));
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
				c.SetAccess(0, ROOT_AE3_Util.List(V(v, 0)));
				c.SetAccess(1, ROOT_AE3_Util.List(V(v, 1)));
				return 1;
			}

			case "user_speed": c.SetPref(V(v, 0), "speed", V(v, 1)); return 1;
			case "add_file":
			{
				if (V(v, 2).Trim().IsEmpty())
					return Fail("Enter a file name.");

				string protection = V(v, 8);
				int enc;
				string fileKey;
				if (protection == "cipher" || protection == "both")
				{
					enc = V(v, 10).ToInt();
					map<int, int> keyField = new map<int, int>();
					keyField.Set(ROOT_AE3_Encryption.CAESAR, 11);
					keyField.Set(ROOT_AE3_Encryption.COLUMNAR, 12);
					keyField.Set(ROOT_AE3_Encryption.VIGENERE, 13);
					keyField.Set(ROOT_AE3_Encryption.ROT, 14);
					keyField.Set(ROOT_AE3_Encryption.AFFINE, 15);
					keyField.Set(ROOT_AE3_Encryption.RAILFENCE, 16);
					keyField.Set(ROOT_AE3_Encryption.BACON, 17);
					keyField.Set(ROOT_AE3_Encryption.SUBSTITUTION, 18);
					keyField.Set(ROOT_AE3_Encryption.INTEGER, 19);
					if (keyField.Contains(enc))
						fileKey = V(v, keyField.Get(enc)).Trim();
				}

				string password;
				if (protection == "password" || protection == "both")
				{
					password = V(v, 9);
					if (password.IsEmpty())
						return Fail("Enter a password or choose another protection.");
				}

				string owner = V(v, 5);
				if (owner == "_new" && V(v, 4) != "root")
				{
					owner = V(v, 6).Trim();
					if (!ROOT_AE3_Filesystem.ValidName(owner) || owner == "root")
						return Fail("Enter a valid new account name (letters, digits, - and _).");

					array<string> existing = {};
					c.GetUsers(existing);
					if (!existing.Contains(owner) && !ROOT_AE3_API.AddUser(e, owner, V(v, 7)))
						return Fail("The account could not be created.");
				}
				string perms = "rw-r--";
				if (V(v, 4) == "owner")
				{
					perms = "rw----";
				}
				else if (V(v, 4) == "root")
				{
					owner = "root";
					perms = "rw----";
				}

				int size = -1;
				if (!V(v, 20).Trim().IsEmpty())
					size = V(v, 20).ToInt();

				if (!ROOT_AE3_API.AddFile(e, AddFilePath(v), V(v, 3), owner, perms, enc, fileKey, password, size))
					return Fail(ROOT_AE3_API.GetLastError());

				return 1;
			}

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
			case "power_cable": return R(ROOT_AE3_API.SetPowerCableLength(e, V(v, 0).ToFloat()));
			case "ui_style": return R(ROOT_AE3_API.SetUiStyle(e, V(v, 0).ToInt()));
			case "power_source":
			{
				if (V(v, 0).IsEmpty())
					return R(ROOT_AE3_API.DisconnectPower(e));

				ROOT_AE3_DeviceComponent src = ROOT_AE3_DeviceComponent.FromRplId(V(v, 0).ToInt());
				if (!src)
					return Fail("Power source not found.");

				string preason;
				if (!ROOT_AE3_ConnectionComponent.ConnectPower(e, src.GetOwner(), preason))
					return Fail(preason);

				return 1;
			}
			case "power_on": return R(ROOT_AE3_API.SetPowerState(e, 1));
			case "power_off": return R(ROOT_AE3_API.SetPowerState(e, 0));
			case "standby": return R(ROOT_AE3_API.SetPowerState(e, 2));
			case "crash": return R(ROOT_AE3_API.CrashDevice(e));
			case "disconnect_power": return R(ROOT_AE3_API.DisconnectPower(e));
			case "disconnect_network": return R(ROOT_AE3_API.DisconnectNetwork(e));
			case "router":
			{
				n.ConfigureRouter(V(v, 0).Trim(), V(v, 1), 0, -1, n.AcceptsConnections(), n.AllowsExternalSsh(), -1);
				array<string> lines = {};
				V(v, 2).Split("\n", lines, true);
				array<string> allow = {};
				foreach (string line : lines)
				{
					line.TrimInPlace();
					if (!line.IsEmpty())
						allow.Insert(line);
				}

				n.SetExternalAllow(allow);
				return 1;
			}

			case "connect_wifi":
			{
				ROOT_AE3_NetworkSystem sys = ROOT_AE3_NetworkSystem.Get();
				ROOT_AE3_NetworkComponent r = sys.FindRouterBySubnet(ROOT_AE3_Util.Subnet(V(v, 0)));
				if (!r)
					return Fail("Network not found.");

				string reason;
				bool ok;
				if (V(v, 1) == "cable")
					ok = ROOT_AE3_ConnectionComponent.ConnectNetwork(e, r.GetOwner(), reason);
				else
					ok = sys.Connect(n, r, r.GetPassword(), reason, V(v, 2) == "1");

				if (!ok)
					return Fail(reason);

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
