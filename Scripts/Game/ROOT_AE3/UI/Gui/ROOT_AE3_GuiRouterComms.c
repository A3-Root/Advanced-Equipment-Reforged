//! Server: desktop requests for mail, chat, calendar, web, network, router administration, SSH, map and CCTV.
class ROOT_AE3_GuiRouterComms
{
	//------------------------------------------------------------------------------------------------
	protected static string A(array<string> f, int i)
	{
		if (i < f.Count())
			return f[i];

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected static void Msg(ROOT_AE3_Session s, string rid, bool ok, string cmd, string text)
	{
		ROOT_AE3_GuiRouter.Msg(s, rid, ok, cmd, text);
	}

	//------------------------------------------------------------------------------------------------
	protected static string Header(string mail, string key)
	{
		array<string> lines = {};
		mail.Split("\n", lines, false);
		foreach (string l : lines)
		{
			if (l.IsEmpty())
				break;

			if (l.StartsWith(key + ": "))
				return l.Substring(key.Length() + 2, l.Length() - key.Length() - 2);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Conversation row: self, peer, last message, last time.
	protected static string ThreadRow(string self, string peer, string log)
	{
		array<string> lines = {};
		log.Split("\n", lines, true);
		string text;
		string time;
		if (!lines.IsEmpty())
		{
			string last = lines[lines.Count() - 1];
			int close = last.IndexOf("] ");
			if (last.StartsWith("[") && close > 0)
			{
				time = last.Substring(1, close - 1);
				last = last.Substring(close + 2, last.Length() - close - 2);
			}

			if (last.StartsWith(self + ": "))
				text = "You: " + last.Substring(self.Length() + 2, last.Length() - self.Length() - 2);
			else if (last.StartsWith(peer + ": "))
				text = last.Substring(peer.Length() + 2, last.Length() - peer.Length() - 2);
			else
				text = last;
		}

		array<string> e = {self, peer, text, time};
		return ROOT_AE3_Codec.JoinSub(e);
	}

	//------------------------------------------------------------------------------------------------
	static bool Handle(ROOT_AE3_Session s, string rid, string cmd, array<string> f, string user)
	{
		ROOT_AE3_ComputerComponent c = s.m_Computer;
		ROOT_AE3_Filesystem fs = c.GetFilesystem();
		string err;
		switch (cmd)
		{
			case "mail_list":
			{
				string folder = "/var/mail";
				if (A(f, 2) == "sent")
					folder = "/var/sent";

				array<string> res = {};
				ROOT_AE3_FsNode dir = fs.Resolve(folder, "root", err);
				if (dir && dir.m_bDir)
				{
					array<string> names = dir.GetSortedNames();
					for (int i = names.Count() - 1; i >= 0; i--)
					{
						ROOT_AE3_FsNode n = dir.GetChild(names[i]);
						if (!names[i].EndsWith(".mail"))
							continue;

						string who = Header(n.m_sContent, "From");
						if (A(f, 2) == "sent")
							who = Header(n.m_sContent, "To");

						array<string> e = {folder + "/" + names[i], who, Header(n.m_sContent, "Subject"), Header(n.m_sContent, "Date")};
						res.Insert(ROOT_AE3_Codec.JoinSub(e));
					}
				}

				ROOT_AE3_GuiRouter.Reply(s, rid, true, cmd, res);
				return true;
			}

			case "mail_read":
			{
				ROOT_AE3_FsNode mail = MailNode(fs, A(f, 2));
				if (!mail)
				{
					Msg(s, rid, false, cmd, "No such mail.");
					return true;
				}

				string body = Display(mail.m_sContent);
				int split = body.IndexOf("\n\n");
				if (split >= 0)
					body = body.Substring(split + 2, body.Length() - split - 2);

				array<string> m = {Header(mail.m_sContent, "From"), Header(mail.m_sContent, "To"), Header(mail.m_sContent, "Subject"), Header(mail.m_sContent, "Date"), body};
				ROOT_AE3_GuiRouter.Reply(s, rid, true, cmd, m);
				return true;
			}

			case "mail_delete":
			{
				if (!MailNode(fs, A(f, 2)) || !fs.Remove(A(f, 2), "root", false, err))
					Msg(s, rid, false, cmd, "No such mail.");
				else
					Msg(s, rid, true, cmd, "Mail deleted.");

				return true;
			}

			case "mail_send":
			{
				string from = A(f, 2);
				if (!c.GetMailAddresses().Contains(ROOT_AE3_Util.Lower(from)))
				{
					Msg(s, rid, false, cmd, "Create a mail address first (Addresses).");
					return true;
				}

				string reason;
				if (!ROOT_AE3_Intel.SendMail(c, from, A(f, 3), A(f, 4), A(f, 5), reason))
					Msg(s, rid, false, cmd, reason);
				else
					Msg(s, rid, true, cmd, "Mail sent.");

				return true;
			}

			case "addr_list":
			{
				array<string> res = {};
				res.Copy(c.GetMailAddresses());
				ROOT_AE3_GuiRouter.Reply(s, rid, true, cmd, res);
				return true;
			}

			case "addr_create":
			{
				if (!ROOT_AE3_Intel.RegisterMailAddress(c, A(f, 2)))
					Msg(s, rid, false, cmd, "Address taken or invalid.");
				else
					Msg(s, rid, true, cmd, "Address created.");

				return true;
			}

			case "addr_delete":
			{
				if (!ROOT_AE3_Intel.RemoveMailAddress(c, A(f, 2)))
					Msg(s, rid, false, cmd, "No such address.");
				else
					Msg(s, rid, true, cmd, "Address deleted.");

				return true;
			}

			case "handle_list":
			{
				ROOT_AE3_GuiRouter.Reply(s, rid, true, cmd, ROOT_AE3_Intel.HandlesOf(c));
				return true;
			}

			case "handle_create":
			{
				if (!ROOT_AE3_Intel.RegisterHandle(c, A(f, 2)))
					Msg(s, rid, false, cmd, "Handle taken or invalid.");
				else
					Msg(s, rid, true, cmd, "Handle created.");

				return true;
			}

			case "handle_delete":
			{
				if (!ROOT_AE3_Intel.RemoveHandle(c, A(f, 2)))
					Msg(s, rid, false, cmd, "No such handle.");
				else
					Msg(s, rid, true, cmd, "Handle deleted.");

				return true;
			}

			case "handles_all":
			{
				array<string> own = ROOT_AE3_Intel.HandlesOf(c);
				array<string> all = {};
				ROOT_AE3_Intel.GetHandles(all);
				array<string> res = {};
				foreach (string h : all)
				{
					if (!own.Contains(h))
						res.Insert(h);
				}

				ROOT_AE3_GuiRouter.Reply(s, rid, true, cmd, res);
				return true;
			}

			case "chat_threads":
			{
				array<string> res = {};
				array<string> own = ROOT_AE3_Intel.HandlesOf(c);
				foreach (int hi, string h : own)
				{
					ROOT_AE3_FsNode dir = fs.Resolve("/var/chat/" + h, "root", err, false);
					if (dir && dir.m_bDir && !dir.IsLink())
					{
						foreach (string name : dir.GetSortedNames())
						{
							ROOT_AE3_FsNode log = dir.GetChild(name);
							if (log && !log.m_bDir && !log.IsLink() && name.EndsWith(".log"))
								res.Insert(ThreadRow(h, name.Substring(0, name.Length() - 4), log.m_sContent));
						}
					}

					// Logs of the single-handle layout belong to the default handle.
					if (hi > 0)
						continue;

					ROOT_AE3_FsNode root = fs.Resolve("/var/chat", "root", err, false);
					if (!root || !root.m_bDir)
						continue;

					foreach (string name : root.GetSortedNames())
					{
						ROOT_AE3_FsNode old = root.GetChild(name);
						string peer = name.Substring(0, Math.Max(0, name.Length() - 4));
						if (old && !old.m_bDir && !old.IsLink() && name.EndsWith(".log") && !(dir && dir.m_bDir && dir.GetChild(name)))
							res.Insert(ThreadRow(h, peer, old.m_sContent));
					}
				}

				ROOT_AE3_GuiRouter.Reply(s, rid, true, cmd, res);
				return true;
			}

			case "chat_read":
			{
				string self = A(f, 2);
				string peer = A(f, 3);
				string content;
				if (ROOT_AE3_Intel.HandlesOf(c).Contains(self))
				{
					ROOT_AE3_FsNode log = PlainNode(fs, "/var/chat/" + self, peer + ".log");
					if (!log && self == ROOT_AE3_Intel.HandleOf(c))
						log = PlainNode(fs, "/var/chat", peer + ".log");

					if (log)
						content = Display(log.m_sContent);
				}

				array<string> res = {self, peer, content};
				ROOT_AE3_GuiRouter.Reply(s, rid, true, cmd, res);
				return true;
			}

			case "chat_send":
			{
				string reason;
				if (!ROOT_AE3_Intel.SendChat(c, A(f, 2), A(f, 3), A(f, 4), reason))
					Msg(s, rid, false, cmd, reason);
				else
					Msg(s, rid, true, cmd, "Sent.");

				return true;
			}

			case "cal_list":
			{
				array<string> res = {};
				foreach (ROOT_AE3_CalendarEvent ev : c.GetCalendar())
				{
					array<string> e = {ev.m_sDate, ev.m_sTime, ev.m_sTitle, ev.m_sText, ev.m_sLocation};
					res.Insert(ROOT_AE3_Codec.JoinSub(e));
				}

				res.Sort();
				ROOT_AE3_GuiRouter.Reply(s, rid, true, cmd, res);
				return true;
			}

			case "cal_add":
			{
				if (A(f, 4).Trim().IsEmpty())
				{
					Msg(s, rid, false, cmd, "Title required.");
					return true;
				}

				c.AddCalendarEvent(A(f, 2), A(f, 3), A(f, 4), A(f, 5), A(f, 6));
				Msg(s, rid, true, cmd, "Event added.");
				return true;
			}

			case "cal_remove":
			{
				c.RemoveCalendarEvent(A(f, 2), A(f, 3));
				Msg(s, rid, true, cmd, "Event removed.");
				return true;
			}

			case "web_get":
			{
				string url = A(f, 2);
				ROOT_AE3_Webpage page = ROOT_AE3_Intel.FindPage(c, url);
				if (!page)
					page = RouterPage(c, url);

				if (!page)
				{
					Msg(s, rid, false, cmd, "No page at this address.");
					return true;
				}

				ROOT_AE3_Intel.AddBrowserHistory(c, page.m_sUrl);
				array<string> res = {page.m_sTitle, page.m_sContent};
				ROOT_AE3_GuiRouter.Reply(s, rid, true, cmd, res);
				return true;
			}

			case "web_pages":
			{
				array<string> res = {};
				ROOT_AE3_Intel.ListPages(c, res);
				ROOT_AE3_GuiRouter.Reply(s, rid, true, cmd, res);
				return true;
			}

			case "web_history":
			{
				string content;
				fs.ReadFile("/var/log/browser_history", "root", content, err);
				array<string> lines = {};
				content.Split("\n", lines, true);
				array<string> res = {};
				for (int i = lines.Count() - 1; i >= 0 && res.Count() < 60; i--)
				{
					res.Insert(lines[i]);
				}

				ROOT_AE3_GuiRouter.Reply(s, rid, true, cmd, res);
				return true;
			}
		}

		return HandleNet(s, rid, cmd, f, user);
	}

	//------------------------------------------------------------------------------------------------
	//! Status page of a router when the address is a gateway.
	protected static ROOT_AE3_Webpage RouterPage(ROOT_AE3_ComputerComponent c, string url)
	{
		ROOT_AE3_NetworkComponent r = ROOT_AE3_NetworkSystem.Get().FindRouterBySubnet(ROOT_AE3_Util.Subnet(url));
		if (!r || r.GetGateway() != url)
			return null;

		int hops;
		if (!ROOT_AE3_NetworkSystem.Get().CanReach(c.GetNetwork(), r, hops))
			return null;

		ROOT_AE3_Webpage p = new ROOT_AE3_Webpage();
		p.m_sUrl = url;
		p.m_sTitle = r.GetSsid() + " - router";
		p.m_sContent = string.Format("# %1\n\n- Gateway: %2\n- Clients: %3\n- Range: %4 m\n- External SSH: %5",
			r.GetSsid(), r.GetGateway(), r.GetClients().Count(), Math.Round(r.GetRange()), ROOT_AE3_Util.BoolStr(r.AllowsExternalSsh()));
		return p;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool HandleNet(ROOT_AE3_Session s, string rid, string cmd, array<string> f, string user)
	{
		ROOT_AE3_ComputerComponent c = s.m_Computer;
		ROOT_AE3_NetworkComponent n = c.GetNetwork();
		ROOT_AE3_NetworkSystem sys = ROOT_AE3_NetworkSystem.Get();
		string reason;
		if (cmd.StartsWith("net_") || cmd == "ping_host" || cmd.StartsWith("ssh_"))
		{
			if (!n)
			{
				Msg(s, rid, false, cmd, "This computer has no network interface.");
				return true;
			}
		}

		switch (cmd)
		{
			case "net_info":
			{
				string ip = n.GetAddress();
				if (ip.IsEmpty())
					ip = "not connected";

				string ssid = "-";
				if (n.GetParent())
					ssid = n.GetParent().GetSsid();

				string ssh = "off";
				if (n.IsSshEnabled())
					ssh = "on";

				array<string> info = {ip, n.GetGateway(), ssid, ssh};
				ROOT_AE3_GuiRouter.Reply(s, rid, true, cmd, info);
				return true;
			}

			case "net_scan":
			{
				array<ROOT_AE3_NetworkComponent> found = {};
				sys.Scan(ROOT_AE3_Util.WorldPos(c.GetOwner()), found);
				array<string> res = {};
				foreach (ROOT_AE3_NetworkComponent r : found)
				{
					string sec = "open";
					if (!r.GetPassword().IsEmpty())
						sec = "password";

					array<string> e = {r.GetSsid(), r.GetGateway(), Math.Round(vector.Distance(ROOT_AE3_Util.WorldPos(c.GetOwner()), ROOT_AE3_Util.WorldPos(r.GetOwner()))).ToString() + " m", sec};
					res.Insert(ROOT_AE3_Codec.JoinSub(e));
				}

				ROOT_AE3_GuiRouter.Reply(s, rid, true, cmd, res);
				return true;
			}

			case "net_connect":
			{
				ROOT_AE3_NetworkComponent r = sys.FindRouterBySubnet(ROOT_AE3_Util.Subnet(A(f, 2)));
				if (r && !r.GetPassword().IsEmpty() && !s.CheckAuth(r.GetPassword() == A(f, 3)))
				{
					Msg(s, rid, false, cmd, "Wrong password.");
					return true;
				}

				if (!sys.Connect(n, r, A(f, 3), reason))
					Msg(s, rid, false, cmd, reason);
				else
					Msg(s, rid, true, cmd, "Connected as " + n.GetAddress());

				return true;
			}

			case "net_ssh":
			{
				if (!c.IsSudoer(user))
				{
					Msg(s, rid, false, cmd, "Changing remote access needs root or a sudoer.");
					return true;
				}

				n.SetSshEnabled(A(f, 2) == "1");
				if (n.IsSshEnabled())
					Msg(s, rid, true, cmd, "Remote access on.");
				else
					Msg(s, rid, true, cmd, "Remote access off.");

				return true;
			}

			case "net_disconnect":
			{
				sys.Disconnect(n);
				Msg(s, rid, true, cmd, "Disconnected.");
				return true;
			}

			case "ping_host":
			{
				ROOT_AE3_NetworkComponent target = sys.Resolve(A(f, 2));
				int hops;
				if (n.GetAddress().IsEmpty())
					Msg(s, rid, true, cmd, "Network is unreachable.");
				else if (!target || !sys.CanReach(n, target, hops))
					Msg(s, rid, true, cmd, string.Format("PING %1\nFrom %2: Destination Host Unreachable", A(f, 2), n.GetAddress()));
				else
					Msg(s, rid, true, cmd, string.Format("PING %1 from %2\nReply: hops=%3 time=%4 ms\nReply: hops=%3 time=%5 ms\nReply: hops=%3 time=%6 ms\n3 packets transmitted, 3 received", A(f, 2), n.GetAddress(), hops, 2 + hops * 3, 3 + hops * 3, 2 + hops * 3));

				return true;
			}

			case "ssh_connect":
			{
				string target = A(f, 2);
				int at = target.IndexOf("@");
				if (at < 0)
				{
					Msg(s, rid, false, cmd, "Use user@host (IP address or hostname).");
					return true;
				}

				string sshUser = target.Substring(0, at);
				string ip = target.Substring(at + 1, target.Length() - at - 1);
				ROOT_AE3_NetworkComponent tn = sys.Resolve(ip);
				ROOT_AE3_ComputerComponent tc;
				if (tn)
					tc = ROOT_AE3_ComputerComponent.Of(tn.GetOwner());

				if (!tc || tc == c || !sys.SshAllowed(n, tn, reason) || !tc.IsRunning())
				{
					if (reason.IsEmpty())
						reason = "No route to host.";

					Msg(s, rid, false, cmd, reason);
					return true;
				}

				if ((sshUser == "root" && !tc.AllowsRootLogin()) || !s.CheckAuth(tc.HasUser(sshUser) && tc.CheckPassword(sshUser, A(f, 3))))
				{
					ROOT_AE3_Shell.WriteLog(tc, "/var/log/auth.log", string.Format("ssh: failed login for '%1' from %2", sshUser, n.GetAddress()));
					Msg(s, rid, false, cmd, "Permission denied.");
					return true;
				}

				s.m_GuiSshTarget = tc;
				s.m_sGuiSshUser = sshUser;
				ROOT_AE3_Shell.WriteLog(tc, "/var/log/auth.log", string.Format("ssh: user '%1' logged in from %2", sshUser, n.GetAddress()));
				Msg(s, rid, true, cmd, "Connected to " + tc.GetHostname());
				return true;
			}

			case "ssh_close":
			{
				s.m_GuiSshTarget = null;
				s.m_sGuiSshUser = "";
				Msg(s, rid, true, cmd, "Connection closed.");
				return true;
			}

			case "ssh_ls":
			case "ssh_read":
			case "ssh_pull":
			case "ssh_push":
				return HandleSsh(s, rid, cmd, f, user);

			case "map_data":
			{
				MapData(s, rid, c, A(f, 2).ToFloat());
				return true;
			}

			case "cctv_list":
			{
				array<string> res = {};
				ROOT_AE3_Cctv.List(res);
				ROOT_AE3_GuiRouter.Reply(s, rid, true, cmd, res);
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool HandleSsh(ROOT_AE3_Session s, string rid, string cmd, array<string> f, string user)
	{
		ROOT_AE3_ComputerComponent c = s.m_Computer;
		ROOT_AE3_ComputerComponent tc = s.m_GuiSshTarget;
		int hops;
		string sshReason;
		if (!tc || !tc.IsRunning() || !ROOT_AE3_NetworkSystem.Get().CanReach(c.GetNetwork(), tc.GetNetwork(), hops) || !ROOT_AE3_NetworkSystem.Get().SshAllowed(c.GetNetwork(), tc.GetNetwork(), sshReason))
		{
			s.m_GuiSshTarget = null;
			Msg(s, rid, false, cmd, "Not connected.");
			return true;
		}

		string ru = tc.FsUser(s.m_sGuiSshUser);
		string lu = c.FsUser(user);
		ROOT_AE3_Filesystem rfs = tc.GetFilesystem();
		ROOT_AE3_Filesystem lfs = c.GetFilesystem();
		string err;
		string remote = ROOT_AE3_Filesystem.Normalize(ROOT_AE3_Filesystem.HomeOf(s.m_sGuiSshUser), A(f, 2), s.m_sGuiSshUser);
		switch (cmd)
		{
			case "ssh_ls":
			{
				ROOT_AE3_FsNode dir = rfs.Resolve(remote, ru, err);
				if (!dir || !dir.m_bDir || !dir.HasPermission(ru, 0))
				{
					if (err.IsEmpty())
						err = ROOT_AE3_Filesystem.ERR_PERMISSION;

					Msg(s, rid, false, cmd, err);
					return true;
				}

				array<string> res = {remote};
				foreach (string name : dir.GetSortedNames())
				{
					ROOT_AE3_FsNode node = dir.GetChild(name);
					string full = remote + "/" + name;
					if (remote == "/")
						full = "/" + name;

					string type = "file";
					if (node.m_bDir)
						type = "dir";

					array<string> e = {full, type, ROOT_AE3_Util.FormatSize(node.GetSize())};
					res.Insert(ROOT_AE3_Codec.JoinSub(e));
				}

				ROOT_AE3_GuiRouter.Reply(s, rid, true, cmd, res);
				return true;
			}

			case "ssh_read":
			{
				string content;
				if (!rfs.ReadFile(remote, ru, content, err))
					Msg(s, rid, false, cmd, err);
				else
					Msg(s, rid, true, cmd, Display(content));

				return true;
			}

			case "ssh_pull":
			{
				string local = ROOT_AE3_Filesystem.Normalize(ROOT_AE3_Filesystem.HomeOf(user), A(f, 3), user);
				ROOT_AE3_FsNode node = rfs.Resolve(remote, ru, err);
				ROOT_AE3_FsNode target = lfs.Resolve(local, lu, err);
				ROOT_AE3_Filesystem targetFs = lfs.GetLastFs();
				if (!node || !target || !target.m_bDir)
				{
					Msg(s, rid, false, cmd, "Choose a remote file and an existing local folder.");
					return true;
				}

				if (!node.HasPermission(ru, 0))
				{
					Msg(s, rid, false, cmd, ROOT_AE3_Filesystem.ERR_PERMISSION);
					return true;
				}

				ROOT_AE3_FsNode copy = node.CopyReadable(ru);
				if (!target.HasPermission(lu, 1))
				{
					Msg(s, rid, false, cmd, ROOT_AE3_Filesystem.ERR_PERMISSION);
					return true;
				}

				ROOT_AE3_SshTransferJob job = new ROOT_AE3_SshTransferJob();
				job.m_Session = s;
				job.m_sRid = rid;
				job.m_sCmd = cmd;
				job.m_Node = copy;
				job.m_Target = target;
				job.m_Fs = targetFs;
				job.m_Remote = s.m_GuiSshTarget;
				job.m_sWriteUser = lu;
				job.m_TargetFs = lfs;
				job.m_sTargetPath = local;
				job.m_sOwner = user;
				job.Schedule(ROOT_AE3_Transfer.Duration(node.GetSize(), ROOT_AE3_Transfer.NETWORK, Speed(c, user)) * 1000);
				return true;
			}

			case "ssh_push":
			{
				string local = ROOT_AE3_Filesystem.Normalize(ROOT_AE3_Filesystem.HomeOf(user), A(f, 2), user);
				string rdir = ROOT_AE3_Filesystem.Normalize(ROOT_AE3_Filesystem.HomeOf(s.m_sGuiSshUser), A(f, 3), s.m_sGuiSshUser);
				ROOT_AE3_FsNode node = lfs.Resolve(local, lu, err);
				ROOT_AE3_FsNode target = rfs.Resolve(rdir, ru, err);
				ROOT_AE3_Filesystem remoteFs = rfs.GetLastFs();
				if (!node || !target || !target.m_bDir || !target.HasPermission(ru, 1) || !node.HasPermission(lu, 0))
				{
					Msg(s, rid, false, cmd, "Choose a readable local file and a writable remote folder.");
					return true;
				}

				ROOT_AE3_SshTransferJob job = new ROOT_AE3_SshTransferJob();
				job.m_Session = s;
				job.m_sRid = rid;
				job.m_sCmd = cmd;
				job.m_Node = node.CopyReadable(lu);
				job.m_Target = target;
				job.m_Fs = remoteFs;
				job.m_Remote = s.m_GuiSshTarget;
				job.m_sWriteUser = ru;
				job.m_TargetFs = rfs;
				job.m_sTargetPath = rdir;
				job.m_sOwner = s.m_sGuiSshUser;
				job.Schedule(ROOT_AE3_Transfer.Duration(node.GetSize(), ROOT_AE3_Transfer.NETWORK, Speed(c, user)) * 1000);
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Content shown to the client: locked files never leave the server.
	static string Display(string content)
	{
		if (ROOT_AE3_Intel.IsLocked(content))
			return "This file is password protected.";

		return content;
	}

	//------------------------------------------------------------------------------------------------
	//! Mail file directly in /var/mail or /var/sent, no symbolic links.
	protected static ROOT_AE3_FsNode MailNode(ROOT_AE3_Filesystem fs, string path)
	{
		string dir = ROOT_AE3_Filesystem.Parent(path);
		if ((dir != "/var/mail" && dir != "/var/sent") || !path.EndsWith(".mail"))
			return null;

		return PlainNode(fs, dir, ROOT_AE3_Filesystem.BaseName(path));
	}

	//------------------------------------------------------------------------------------------------
	//! Regular file name directly in dir; links and other names are rejected.
	protected static ROOT_AE3_FsNode PlainNode(ROOT_AE3_Filesystem fs, string dir, string name)
	{
		if (!ROOT_AE3_Filesystem.ValidName(name) || name.Contains("/") || name == "." || name == "..")
			return null;

		string err;
		ROOT_AE3_FsNode d = fs.Resolve(dir, "root", err, false);
		if (!d || !d.m_bDir || d.IsLink())
			return null;

		ROOT_AE3_FsNode n = d.GetChild(name);
		if (!n || n.m_bDir || n.IsLink())
			return null;

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Transfer speed in KB/s: user > device > router chain > setting.
	protected static float Speed(ROOT_AE3_ComputerComponent c, string user)
	{
		float userSpeed = c.GetUserSpeed(user);
		if (userSpeed > 0)
			return userSpeed;

		return ROOT_AE3_NetworkSystem.Get().GetTransferSpeed(c.GetNetwork());
	}

	//------------------------------------------------------------------------------------------------
	//! Range, then "name|kind|dx|dz|address" per device; kinds self, router, computer, power.
	//! Devices within range m (east, north offsets) of the computer; range 100-5000, default 500.
	//! Setting m_bDesktopMapSameNetworkOnly: only the computer, its router, the routers linked to it and the
	//! devices connected to it; no power devices.
	protected static void MapData(ROOT_AE3_Session s, string rid, ROOT_AE3_ComputerComponent c, float range)
	{
		if (range <= 0)
			range = 500;

		range = Math.Clamp(range, 100, 5000);
		vector origin = ROOT_AE3_Util.WorldPos(c.GetOwner());
		bool sameNet = ROOT_AE3_SettingsManager.Get().m_bDesktopMapSameNetworkOnly;
		ROOT_AE3_NetworkComponent own = c.GetNetwork();
		ROOT_AE3_NetworkComponent ownRouter;
		if (own && own.IsRouter())
			ownRouter = own;
		else if (own && own.IsConnected())
			ownRouter = own.GetParent();

		array<string> res = {range.ToString()};
		foreach (ROOT_AE3_DeviceComponent d : ROOT_AE3_Manager.Get().GetDevices())
		{
			if (!d)
				continue;

			vector pos = d.GetOwner().GetOrigin();
			vector delta = pos - origin;
			if (d.IsHidden() || Math.AbsFloat(delta[0]) > range || Math.AbsFloat(delta[2]) > range || ROOT_AE3_Util.IsCarried(d.GetOwner()))
				continue;

			string kind = "computer";
			ROOT_AE3_EDeviceType type = d.GetDeviceType();
			if (d.GetOwner() == c.GetOwner())
				kind = "self";
			else if (type == ROOT_AE3_EDeviceType.ROUTER)
				kind = "router";
			else if (type == ROOT_AE3_EDeviceType.GENERATOR || type == ROOT_AE3_EDeviceType.BATTERY || type == ROOT_AE3_EDeviceType.SOLAR)
				kind = "power";
			else if (!d.IsComputer())
				continue;

			ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(d.GetOwner());
			string ip;
			if (n)
				ip = n.GetAddress();

			if (sameNet && kind != "self")
			{
				if (!n || !ownRouter)
					continue;

				bool linked = n == ownRouter || n.GetParent() == ownRouter || (n.IsRouter() && ownRouter.GetParent() == n);
				if (!linked)
					continue;
			}

			array<string> e = {d.GetDeviceName(), kind, delta[0].ToString(), delta[2].ToString(), ip};
			res.Insert(ROOT_AE3_Codec.JoinSub(e));
		}

		ROOT_AE3_GuiRouter.Reply(s, rid, true, "map_data", res);
	}
}
