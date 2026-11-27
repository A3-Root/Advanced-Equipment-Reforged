//! Server: desktop requests "rid|cmd|args..." of a session, answered with GUI_REPLY "rid|ok|cmd|payload...".
//! Filesystem access uses the session user (elevated for sudoers when the setting allows it).
class ROOT_AE3_GuiRouter
{
	//------------------------------------------------------------------------------------------------
	static void Reply(ROOT_AE3_Session s, string rid, bool ok, string cmd, array<string> payload = null)
	{
		array<string> f = {rid, ROOT_AE3_Util.BoolStr(ok), cmd};
		if (payload)
		{
			foreach (string p : payload)
			{
				f.Insert(p);
			}
		}

		ROOT_AE3_Net.SendToPlayer(s.m_iPlayerId, ROOT_AE3_EMsg.GUI_REPLY, ROOT_AE3_Codec.Join(f));
	}

	//------------------------------------------------------------------------------------------------
	static void Msg(ROOT_AE3_Session s, string rid, bool ok, string cmd, string text)
	{
		array<string> p = {text};
		Reply(s, rid, ok, cmd, p);
	}

	//------------------------------------------------------------------------------------------------
	protected static string A(array<string> f, int i)
	{
		if (i < f.Count())
			return f[i];

		return "";
	}

	static const int MAX_LIST = 500;	//!< Entries per folder listing
	protected static ref map<string, ref ROOT_AE3_GuiCommand> s_mExternal = new map<string, ref ROOT_AE3_GuiCommand>();

	//------------------------------------------------------------------------------------------------
	//! Server: request name handled by another mod.
	static void RegisterExternal(string cmd, ROOT_AE3_GuiCommand handler)
	{
		if (!cmd.IsEmpty() && handler)
			s_mExternal.Set(cmd, handler);
	}

	//------------------------------------------------------------------------------------------------
	static void Handle(ROOT_AE3_Session s, string data)
	{
		array<string> f = ROOT_AE3_Codec.Split(data);
		if (f.Count() < 2)
			return;

		foreach (int i, string field : f)
		{
			f[i] = ROOT_AE3_Codec.Clean(field);
		}

		ROOT_AE3_Regex.SetOwner(s.m_iPlayerId);
		string rid = f[0];
		string cmd = f[1];
		ROOT_AE3_ComputerComponent c = s.m_Computer;
		ROOT_AE3_TerminalState t = s.GetTerminal();
		string user = t.CurrentUser();

		bool termCmd = cmd == "sys_set";
		if (!(s.m_iMode == 1 && s.HasAccess(1)) && !(termCmd && s.HasAccess(0)))
		{
			Msg(s, rid, false, cmd, "Access denied.");
			return;
		}

		if (cmd == "login")
		{
			Login(s, rid, A(f, 2), A(f, 3));
			return;
		}

		if (cmd == "ui_mirror")
		{
			if (t.CurrentUser().IsEmpty())
				return;

			string mirror = ROOT_AE3_Util.Clip(ROOT_AE3_Util.Ascii(A(f, 2)), 256);

			c.SetDesktopMirror(mirror);
			return;
		}

		if (user.IsEmpty())
		{
			Msg(s, rid, false, cmd, "Not logged in.");
			return;
		}

		ROOT_AE3_Filesystem fs = c.GetFilesystem();
		string fsUser = c.FsUser(user);
		string err;
		string cwd = ROOT_AE3_Filesystem.HomeOf(user);
		string p1 = ROOT_AE3_Filesystem.Normalize(cwd, A(f, 2), user);
		string p2 = ROOT_AE3_Filesystem.Normalize(cwd, A(f, 3), user);
		bool fileCmd = cmd.StartsWith("fs_") || cmd.StartsWith("trash") || cmd.StartsWith("ssh_") || cmd == "run_file";
		if (fileCmd && !ROOT_AE3_SettingsManager.Get().m_bDesktopEnableFileBrowsing)
		{
			bool editorFile = cmd == "fs_read" || cmd == "fs_save" || cmd == "fs_media";
			if (!editorFile || !InHome(fs, p1, cwd))
			{
				Msg(s, rid, false, cmd, "File browsing is disabled.");
				return;
			}
		}

		switch (cmd)
		{
			case "signout":
			{
				s.m_GuiSshTarget = null;
				s.m_sGuiSshUser = "";
				ROOT_AE3_Shell.Logout(s);
				Msg(s, rid, true, "user", "");
				return;
			}

			case "shutdown":
			{
				c.Shutdown();
				Msg(s, rid, true, cmd, "Shutting down.");
				return;
			}

			case "standby":
			{
				string reason;
				if (!c.GetPower() || !c.GetPower().Standby(reason))
					Msg(s, rid, false, cmd, reason);
				else
					Msg(s, rid, true, cmd, "Standby.");

				return;
			}

			case "sys_set":
			{
				string key = A(f, 2);
				string value = A(f, 3);
				if (key == "design")
				{
					if (!c.IsSudoer(user))
					{
						Msg(s, rid, false, cmd, "Changing the terminal design needs a sudoer.");
						return;
					}

					c.SetDesign(value.ToInt());
				}
				else if (key == "theme" || key == "wallpaper")
					c.SetPref(user, key, value);

				Msg(s, rid, true, cmd, "Saved.");
				return;
			}

			case "tray":
			{
				Reply(s, rid, true, cmd, Tray(c));
				return;
			}

			case "sysinfo":
			{
				Msg(s, rid, true, cmd, SysInfo(c, user));
				return;
			}

			case "fs_list":
			{
				ROOT_AE3_FsNode dir = fs.Resolve(p1, fsUser, err);
				if (dir && !fs.GetLastUnmounted().IsEmpty())
				{
					Msg(s, rid, false, cmd, ROOT_AE3_Filesystem.NoDrive(fs.GetLastUnmounted()) + ".");
					return;
				}

				if (!dir || !dir.m_bDir || !dir.HasPermission(fsUser, 0))
				{
					if (err.IsEmpty())
						err = ROOT_AE3_Filesystem.ERR_PERMISSION;

					Msg(s, rid, false, cmd, err);
					return;
				}

				array<string> res = {p1};
				foreach (string n : dir.GetSortedNames())
				{
					if (res.Count() > MAX_LIST)
						break;

					ROOT_AE3_FsNode node = dir.GetChild(n);
					string full = p1 + "/" + n;
					if (p1 == "/")
						full = "/" + n;

					res.Insert(Entry(full, node));
				}

				Reply(s, rid, true, cmd, res);
				return;
			}

			case "fs_stat":
			{
				ROOT_AE3_FsNode node = fs.Resolve(p1, fsUser, err, false);
				if (!node)
				{
					Msg(s, rid, false, cmd, err);
					return;
				}

				string type = "file";
				if (node.m_bDir)
					type = "directory";
				else if (node.IsLink())
					type = "link to " + node.m_sLink;

				Msg(s, rid, true, cmd, string.Format("%1  %2  owner %3  %4  %5", p1, type, node.m_sOwner, node.PermString(), ROOT_AE3_Util.FormatSize(node.GetSize())));
				return;
			}

			case "fs_read":
			{
				string content;
				if (!fs.ReadFile(p1, fsUser, content, err))
				{
					Msg(s, rid, false, cmd, err);
					return;
				}

				if (ROOT_AE3_Intel.IsLocked(content))
				{
					Msg(s, rid, false, cmd, "The file is password protected.");
					return;
				}

				Msg(s, rid, true, cmd, content);
				return;
			}

			case "fs_save":
			{
				if (!fs.WriteFile(p1, A(f, 3), fsUser, false, err, -1, user))
					Msg(s, rid, false, cmd, err);
				else
				{
					Msg(s, rid, true, cmd, "Saved " + p1);
				}

				return;
			}

			case "fs_mkdir":
			{
				if (!fs.CreateDir(p1, fsUser, user, ROOT_AE3_FsNode.PERM_DIR_DEFAULT, err))
					Msg(s, rid, false, cmd, err);
				else
					Msg(s, rid, true, cmd, "Created " + p1);

				return;
			}

			case "fs_move":
			{
				if (!fs.Move(p1, p2, fsUser, err))
					Msg(s, rid, false, cmd, err);
				else
					Msg(s, rid, true, cmd, "Moved to " + p2);

				return;
			}

			case "fs_copy":
			{
				ROOT_AE3_FsNode node = fs.Resolve(p1, fsUser, err);
				if (!node)
				{
					Msg(s, rid, false, cmd, err);
					return;
				}

				if (s.m_iCopyJobs >= 8)
				{
					Msg(s, rid, false, cmd, "Too many copies running.");
					return;
				}

				s.m_iCopyJobs++;
				ROOT_AE3_GuiCopyJob job = new ROOT_AE3_GuiCopyJob();
				job.m_Session = s;
				job.m_sRid = rid;
				job.m_sSrc = p1;
				job.m_sDst = p2;
				job.m_sUser = fsUser;
				job.m_sLoginUser = user;
				job.Schedule(ROOT_AE3_Transfer.Duration(node.GetSize(), ROOT_AE3_Transfer.MediumOf(p1, p2)) * 1000);
				return;
			}

			case "fs_delete":
			{
				if (p1 == "/.trash")
				{
					Msg(s, rid, false, cmd, ROOT_AE3_Filesystem.ERR_PERMISSION);
					return;
				}

				if (p1.StartsWith("/.trash/"))
				{
					string entry = p1.Substring(8, p1.Length() - 8);
					if (!TrashOwner(fs, entry, fsUser))
					{
						Msg(s, rid, false, cmd, ROOT_AE3_Filesystem.ERR_PERMISSION);
						return;
					}

					fs.m_mTrashMeta.Remove(entry);
					if (!fs.Remove(p1, "root", true, err))
						Msg(s, rid, false, cmd, err);
					else
						Msg(s, rid, true, cmd, "Deleted.");

					return;
				}

				if (!fs.Trash(p1, fsUser, err))
					Msg(s, rid, false, cmd, err);
				else
				{
					if (err == "deleted")
						Msg(s, rid, true, cmd, "Deleted (flash drives have no recycle bin).");
					else
						Msg(s, rid, true, cmd, "Moved to the recycle bin.");
				}

				return;
			}

			case "trash_list":
			{
				array<string> res = {};
				ROOT_AE3_FsNode trash = fs.Resolve("/.trash", "root", err);
				if (trash)
				{
					foreach (string n : trash.GetSortedNames())
					{
						ROOT_AE3_FsNode entry = trash.GetChild(n);
						if (fsUser != "root" && entry && entry.m_sOwner != fsUser)
							continue;

						res.Insert(n);
						res.Insert(fs.m_mTrashMeta.Get(n));
					}
				}

				Reply(s, rid, true, cmd, res);
				return;
			}

			case "fs_restore":
			{
				if (!TrashOwner(fs, A(f, 2), fsUser))
				{
					Msg(s, rid, false, cmd, ROOT_AE3_Filesystem.ERR_PERMISSION);
					return;
				}

				if (!fs.Restore(A(f, 2), fsUser, err))
					Msg(s, rid, false, cmd, err);
				else
					Msg(s, rid, true, cmd, "Restored.");

				return;
			}

			case "fs_purge":
			{
				if (!TrashOwner(fs, A(f, 2), fsUser))
				{
					Msg(s, rid, false, cmd, ROOT_AE3_Filesystem.ERR_PERMISSION);
					return;
				}

				if (!fs.Remove("/.trash/" + A(f, 2), "root", true, err))
					Msg(s, rid, false, cmd, err);
				else
				{
					fs.m_mTrashMeta.Remove(A(f, 2));
					Msg(s, rid, true, cmd, "Deleted.");
				}

				return;
			}

			case "fs_empty_trash":
			{
				if (fsUser != "root")
				{
					Msg(s, rid, false, cmd, "Only root or sudoers can empty the recycle bin.");
					return;
				}

				fs.EmptyTrash();
				Msg(s, rid, true, cmd, "Recycle bin emptied.");
				return;
			}

			case "fs_chmod":
			{
				int perms = ROOT_AE3_FsNode.ParsePerms(A(f, 3), -1);
				if (perms < 0)
				{
					Msg(s, rid, false, cmd, "Use rwxr-x style permissions.");
					return;
				}

				if (!fs.Chmod(p1, perms, fsUser, err))
					Msg(s, rid, false, cmd, err);
				else
					Msg(s, rid, true, cmd, "Permissions changed.");

				return;
			}

			case "fs_chown":
			{
				if (!c.HasUser(A(f, 3)))
				{
					Msg(s, rid, false, cmd, "Unknown user.");
					return;
				}

				if (!fs.Chown(p1, A(f, 3), fsUser, err))
					Msg(s, rid, false, cmd, err);
				else
					Msg(s, rid, true, cmd, "Owner changed.");

				return;
			}

			case "fs_symlink":
			{
				if (!fs.Symlink(p1, p2, fsUser, err))
					Msg(s, rid, false, cmd, err);
				else
					Msg(s, rid, true, cmd, "Link created.");

				return;
			}

			case "fs_find":
			{
				array<string> found = {};
				fs.Find(p1, A(f, 3), fsUser, found, 12);
				array<string> rows = {A(f, 3)};
				foreach (string fp : found)
				{
					string ferr;
					ROOT_AE3_FsNode fnode = fs.Resolve(fp, fsUser, ferr);
					if (fnode)
						rows.Insert(Entry(fp, fnode));
				}

				Reply(s, rid, true, cmd, rows);
				return;
			}

			case "fs_search":
			{
				array<string> results = {};
				fs.Find(p1, A(f, 3), fsUser, results, 200);
				Reply(s, rid, true, cmd, results);
				return;
			}

			case "fs_unlock":
			{
				string content, payload;
				if (!fs.ReadFile(p1, fsUser, content, err))
				{
					Msg(s, rid, false, cmd, err);
					return;
				}

				if (!s.CheckAuth(ROOT_AE3_Intel.Unlock(content, A(f, 3), payload)))
				{
					ROOT_AE3_Shell.WriteLog(c, "/var/log/auth.log", string.Format("unlock: failed attempt on '%1' by '%2'", p1, user));
					Msg(s, rid, false, cmd, "Wrong password.");
					return;
				}

				if (!fs.WriteFile(p1, payload, fsUser, false, err))
					Msg(s, rid, false, cmd, err);
				else
					Msg(s, rid, true, cmd, "Protection removed.");

				return;
			}

			case "fs_media":
			{
				string content;
				if (!fs.ReadFile(p1, fsUser, content, err))
				{
					Msg(s, rid, false, cmd, err);
					return;
				}

				if (!ROOT_AE3_Intel.IsMedia(content))
				{
					Msg(s, rid, false, cmd, "Not an image file.");
					return;
				}

				array<string> res = {p1, ROOT_AE3_Intel.MediaResource(content), ROOT_AE3_Intel.MediaCaption(content), ROOT_AE3_Intel.MediaType(content)};
				Reply(s, rid, true, cmd, res);
				return;
			}

			case "run_file":
			{
				if (t.m_bBusy || t.m_App || t.m_eInput != ROOT_AE3_ETermInput.SHELL || t.InSsh())
				{
					Msg(s, rid, false, cmd, "The terminal is busy.");
					return;
				}

				if (!s.HasAccess(0) || c.GetInterfaceMode() == 1)
				{
					Msg(s, rid, false, cmd, "Terminal access denied.");
					return;
				}

				ROOT_AE3_FsNode exe = fs.Resolve(p1, fsUser, err);
				if (!exe || exe.m_bDir || !exe.HasPermission(fsUser, 2) || p1.Contains("\"") || p1.Contains("\\"))
				{
					Msg(s, rid, false, cmd, "Not an executable file.");
					return;
				}

				ROOT_AE3_Shell.Process(s, "\"" + p1 + "\"");
				Msg(s, rid, true, cmd, "Started " + p1 + " in the terminal.");
				return;
			}

			case "vol_list":
			{
				array<string> res = {};
				array<string> root = {"root", "/", c.GetHostname(), "mounted", ROOT_AE3_Util.FormatSize(fs.m_Root.GetSize()), fs.m_Root.GetSize().ToString(), "0"};
				res.Insert(ROOT_AE3_Codec.JoinSub(root));
				for (int i = 0; i < c.GetUsbPortCount(); i++)
				{
					string id = "usb" + i.ToString();
					ROOT_AE3_UsbSlot slot = c.GetUsbDrive(i);
					string dev = "empty";
					string state = "-";
					string used = "-";
					int bytes;
					int capacityMb;
					if (slot)
					{
						dev = slot.GetLabel();
						bytes = slot.GetFilesystem().m_Root.GetSize();
						capacityMb = slot.m_iCapacity;
						used = ROOT_AE3_Util.FormatSize(bytes);
						state = "not mounted";
						if (c.IsMounted(i))
							state = "mounted";
					}

					array<string> row = {id, "/mnt/" + id, dev, state, used, bytes.ToString(), capacityMb.ToString()};
					res.Insert(ROOT_AE3_Codec.JoinSub(row));
				}

				// Flash drives the player carries: "drive", id, name.
				array<IEntity> drives = {};
				if (!s.IsGmRoot())
					ROOT_AE3_UsbManager.Drives(GetGame().GetPlayerManager().GetPlayerControlledEntity(s.m_iPlayerId), drives);

				foreach (IEntity d : drives)
				{
					array<string> drow = {"drive", ROOT_AE3_UsbManager.DriveId(d).ToString(), ROOT_AE3_UsbManager.DriveName(d)};
					res.Insert(ROOT_AE3_Codec.JoinSub(drow));
				}

				Reply(s, rid, true, cmd, res);
				return;
			}

			case "vol_mount":
			{
				string reason;
				if (!c.Mount(ROOT_AE3_Cmd_Lsusb.PortOf(A(f, 2)), reason))
					Msg(s, rid, false, cmd, reason);
				else
					Msg(s, rid, true, cmd, "Mounted.");

				return;
			}

			case "vol_unmount":
			{
				if (!c.Unmount(ROOT_AE3_Cmd_Lsusb.PortOf(A(f, 2))))
					Msg(s, rid, false, cmd, "Not mounted.");
				else
					Msg(s, rid, true, cmd, "Unmounted.");

				return;
			}

			case "vol_insert":
			{
				string reason = "That flash drive is not in your inventory.";
				IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(s.m_iPlayerId);
				IEntity drive;
				if (!s.IsGmRoot())
					drive = ROOT_AE3_UsbManager.FindDriveById(player, A(f, 2).ToInt());

				if (drive && ROOT_AE3_UsbManager.Insert(c, player, reason, drive))
					Msg(s, rid, true, cmd, "Drive inserted.");
				else
					Msg(s, rid, false, cmd, reason);

				return;
			}

			case "vol_eject":
			{
				string reason;
				IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(s.m_iPlayerId);
				if (s.IsGmRoot())
					player = null;

				if (!ROOT_AE3_UsbManager.Eject(c, ROOT_AE3_Cmd_Lsusb.PortOf(A(f, 2)), player, reason))
					Msg(s, rid, false, cmd, reason);
				else
					Msg(s, rid, true, cmd, "Drive ejected.");

				return;
			}
		}

		if (ROOT_AE3_GuiRouterGames.Handle(s, rid, cmd, f, user))
			return;

		if (ROOT_AE3_GuiRouterComms.Handle(s, rid, cmd, f, user))
			return;

		ROOT_AE3_GuiCommand ext = s_mExternal.Get(cmd);
		if (!ext)
		{
			Msg(s, rid, false, cmd, "Unknown request " + cmd);
			return;
		}

		array<string> rest = {};
		for (int i = 2; i < f.Count(); i++)
		{
			rest.Insert(f[i]);
		}

		ext.Handle(s, rid, cmd, rest, user);
	}

	//------------------------------------------------------------------------------------------------
	//! "path|type|size|owner|perms|media|locked|program"
	static string Entry(string full, ROOT_AE3_FsNode node)
	{
		string type = "file";
		if (node.m_bDir || !node.m_sMountId.IsEmpty())
			type = "dir";
		else if (node.IsLink())
			type = "link";

		string size = "-";
		if (!node.m_bDir)
			size = ROOT_AE3_Util.FormatSize(node.GetSize());

		bool program = !ROOT_AE3_CommandRegistry.ProgramOf(node.m_sContent).IsEmpty();
		array<string> e = {full, type, size, node.m_sOwner, node.PermString(), ROOT_AE3_Util.BoolStr(ROOT_AE3_Intel.IsMedia(node.m_sContent)), ROOT_AE3_Util.BoolStr(ROOT_AE3_Intel.IsLocked(node.m_sContent)), ROOT_AE3_Util.BoolStr(program), node.GetSize().ToString()};
		return ROOT_AE3_Codec.JoinSub(e);
	}

	//------------------------------------------------------------------------------------------------
	//! Path inside home, reached without symbolic links (file access while file browsing is off).
	protected static bool InHome(ROOT_AE3_Filesystem fs, string path, string home)
	{
		if (path != home && !path.StartsWith(home + "/"))
			return false;

		array<string> parts = {};
		path.Substring(home.Length(), path.Length() - home.Length()).Split("/", parts, true);
		string walked = home;
		string err;
		ROOT_AE3_FsNode homeNode = fs.Resolve(home, "root", err, false);
		if (!homeNode || homeNode.IsLink())
			return false;

		foreach (string p : parts)
		{
			if (p == "." || p == "..")
				return false;

			walked += "/" + p;
			ROOT_AE3_FsNode node = fs.Resolve(walked, "root", err, false);
			if (node && node.IsLink())
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Trash entry owned by user (root: any).
	protected static bool TrashOwner(ROOT_AE3_Filesystem fs, string entry, string user)
	{
		if (!ROOT_AE3_Filesystem.ValidName(entry))
			return false;

		ROOT_AE3_FsNode trash = fs.m_Root.GetChild(".trash");
		if (!trash)
			return false;

		ROOT_AE3_FsNode node = trash.GetChild(entry);
		return node && (user == "root" || node.m_sOwner == user);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Login(ROOT_AE3_Session s, string rid, string user, string password)
	{
		ROOT_AE3_ComputerComponent c = s.m_Computer;
		user = user.Trim();
		if (user == "root" && !c.AllowsRootLogin())
		{
			Msg(s, rid, false, "login", "Direct root login is disabled. Log in as a user and use sudo or su.");
			return;
		}

		bool debugAny = ROOT_AE3_SettingsManager.Get().m_bDebugAnyLogin && !c.HasUser(user) && ROOT_AE3_Filesystem.ValidName(user);
		if (!debugAny && !s.CheckAuth(c.HasUser(user) && c.CheckPassword(user, password)))
		{
			ROOT_AE3_Shell.WriteLog(c, "/var/log/auth.log", string.Format("desktop login failed for '%1'", user));
			Msg(s, rid, false, "login", "Login failed.");
			return;
		}

		if (debugAny)
			c.AddUser(user, password);

		ROOT_AE3_Shell.Login(s, user);
		s.SendPrompt();
		Msg(s, rid, true, "user", user);
	}

	//------------------------------------------------------------------------------------------------
	//! Taskbar tray: external power (0/1), battery percent (empty: none), address (empty: offline), network name,
	//! wired (0/1), plugged flash drives.
	static array<string> Tray(ROOT_AE3_ComputerComponent c)
	{
		array<string> f = {"0", "", "", "", "0", "0"};
		ROOT_AE3_PowerComponent p = c.GetPower();
		if (p)
		{
			f[0] = ROOT_AE3_Util.BoolStr(p.HasExternalPower() || p.IsMains());
			if (p.GetCapacityKWh() > 0)
				f[1] = Math.Round(p.GetLevel() * 100).ToString();
		}

		ROOT_AE3_NetworkComponent n = c.GetNetwork();
		if (n)
		{
			f[2] = n.GetAddress();
			if (n.GetParent())
				f[3] = n.GetParent().GetSsid();

			ROOT_AE3_ConnectionComponent link = ROOT_AE3_ConnectionComponent.Of(c.GetOwner());
			f[4] = ROOT_AE3_Util.BoolStr(link && link.GetNetPeer() != RplId.Invalid());
		}

		int drives;
		for (int i = 0; i < c.GetUsbPortCount(); i++)
		{
			if (c.GetUsbDrive(i))
				drives++;
		}

		f[5] = drives.ToString();
		return f;
	}

	//------------------------------------------------------------------------------------------------
	static string SysInfo(ROOT_AE3_ComputerComponent c, string user)
	{
		string s = string.Format("Host:      %1\nUser:      %2\n", c.GetHostname(), user);
		ROOT_AE3_PowerComponent p = c.GetPower();
		if (p)
		{
			string ext = "no";
			if (p.HasExternalPower())
				ext = "yes";

			s += string.Format("Power:     on, external %1\n", ext);
			if (p.GetCapacityKWh() > 0)
				s += string.Format("Battery:   %1%% of %2 kWh\n", Math.Round(p.GetLevel() * 100), p.GetCapacityKWh());

			s += string.Format("Draw:      %1 W\n", ROOT_AE3_Util.FormatFloat(p.GetConsumerDrawW(), 1));
		}

		ROOT_AE3_NetworkComponent n = c.GetNetwork();
		if (n)
		{
			string ip = n.GetAddress();
			if (ip.IsEmpty())
				ip = "not connected";

			s += string.Format("Address:   %1\nGateway:   %2\n", ip, n.GetGateway());
		}

		s += string.Format("Disk used: %1\nUSB ports: %2\nDate:      %3", ROOT_AE3_Util.FormatSize(c.GetFilesystem().m_Root.GetSize()), c.GetUsbPortCount(), ROOT_AE3_Util.Now());
		return s;
	}
}
