//! Server: intel content and communication. Mail, chat, instant messages, web pages, browser history,
//! media markers ("AE3_MEDIA|image|<texture>") and locked files ("AE3_LOCKED|<pw length>|<pw><content>").
class ROOT_AE3_Intel
{
	static const string LOCK_PREFIX = "AE3_LOCKED|";
	static const string MEDIA_PREFIX = "AE3_MEDIA|";

	protected static ref map<string, ROOT_AE3_ComputerComponent> s_mMail = new map<string, ROOT_AE3_ComputerComponent>();
	protected static ref map<string, ROOT_AE3_ComputerComponent> s_mHandles = new map<string, ROOT_AE3_ComputerComponent>();
	protected static ref map<ROOT_AE3_ComputerComponent, float> s_mLastMail = new map<ROOT_AE3_ComputerComponent, float>();
	protected static ref map<ROOT_AE3_ComputerComponent, float> s_mLastChat = new map<ROOT_AE3_ComputerComponent, float>();
	protected static ref map<ROOT_AE3_ComputerComponent, float> s_mLastNotify = new map<ROOT_AE3_ComputerComponent, float>();
	protected static ref array<string> s_aAutoMail = {};		//!< Addresses given by ProvisionIdentity
	protected static ref array<string> s_aAutoHandles = {};
	static const int MAX_MAILS = 100;		//!< Mail files kept per folder
	static const int MAX_BODY = 8192;		//!< Characters per mail body
	static const float MAIL_INTERVAL = 2;	//!< Seconds between mails from one computer
	protected static ref map<string, ref ROOT_AE3_Webpage> s_mPages = new map<string, ref ROOT_AE3_Webpage>();
	protected static int s_iMailCounter;

	//------------------------------------------------------------------------------------------------
	//! Drops all state (game end).
	static void Reset()
	{
		s_mMail.Clear();
		s_mHandles.Clear();
		s_mLastMail.Clear();
		s_mLastChat.Clear();
		s_mLastNotify.Clear();
		s_aAutoMail.Clear();
		s_aAutoHandles.Clear();
		s_mPages.Clear();
		s_iMailCounter = 0;
	}

	// Locked and media files --------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	static bool IsLocked(string content)
	{
		return content.StartsWith(LOCK_PREFIX);
	}

	//------------------------------------------------------------------------------------------------
	static string Lock(string content, string password)
	{
		return LOCK_PREFIX + password.Length().ToString() + "|" + password + content;
	}

	//------------------------------------------------------------------------------------------------
	static bool Unlock(string content, string password, out string payload)
	{
		if (!IsLocked(content))
			return false;

		string rest = content.Substring(LOCK_PREFIX.Length(), content.Length() - LOCK_PREFIX.Length());
		int bar = rest.IndexOf("|");
		if (bar < 0)
			return false;

		int len = rest.Substring(0, bar).ToInt();
		if (len < 0 || bar + 1 + len > rest.Length())
			return false;

		string pw = rest.Substring(bar + 1, len);
		if (pw != password)
			return false;

		int start = bar + 1 + len;
		payload = rest.Substring(start, rest.Length() - start);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsMedia(string content)
	{
		return content.StartsWith(MEDIA_PREFIX);
	}

	//------------------------------------------------------------------------------------------------
	static string Media(string type, string resource, string caption = "")
	{
		return MEDIA_PREFIX + type + "|" + resource + "|" + caption;
	}

	//------------------------------------------------------------------------------------------------
	static string MediaType(string content)
	{
		array<string> p = {};
		content.Split("|", p, false);
		if (p.Count() < 2)
			return "unknown";

		return p[1];
	}

	//------------------------------------------------------------------------------------------------
	static string MediaResource(string content)
	{
		array<string> p = {};
		content.Split("|", p, false);
		if (p.Count() < 3)
			return "";

		return p[2];
	}

	//------------------------------------------------------------------------------------------------
	static string MediaCaption(string content)
	{
		array<string> p = {};
		content.Split("|", p, false);
		if (p.Count() < 4)
			return "";

		return p[3];
	}

	// Mail ------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Same computer, or a powered network path between both.
	protected static bool Reachable(ROOT_AE3_ComputerComponent a, ROOT_AE3_ComputerComponent b)
	{
		if (a == b)
			return true;

		int hops;
		return ROOT_AE3_NetworkSystem.Get().CanReach(a.GetNetwork(), b.GetNetwork(), hops);
	}

	//------------------------------------------------------------------------------------------------
	//! Sortable id unique across restarts: mission date and time plus a counter.
	protected static string NextMailId()
	{
		s_iMailCounter++;
		string now = ROOT_AE3_Util.Now();
		now.Replace("-", "");
		now.Replace(":", "");
		now.Replace(" ", "_");
		return now + "_" + ROOT_AE3_Util.PadLeft(s_iMailCounter.ToString(), 4, "0") + "_" + Math.RandomInt(0, 1000).ToString();
	}

	//------------------------------------------------------------------------------------------------
	static bool RegisterMailAddress(ROOT_AE3_ComputerComponent computer, string address)
	{
		address = ROOT_AE3_Util.Lower(address.Trim());
		if (address.IsEmpty() || address.Length() > 64 || !ROOT_AE3_Filesystem.ValidName(address))
			return false;

		ROOT_AE3_ComputerComponent owner = s_mMail.Get(address);
		if (owner && owner != computer)
			return false;

		if (!owner && computer.GetMailAddresses().Count() >= 16)
			return false;

		s_mMail.Set(address, computer);
		if (!computer.GetMailAddresses().Contains(address))
			computer.GetMailAddresses().Insert(address);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool RemoveMailAddress(ROOT_AE3_ComputerComponent computer, string address)
	{
		address = ROOT_AE3_Util.Lower(address.Trim());
		if (!computer.GetMailAddresses().Contains(address))
			return false;

		computer.GetMailAddresses().RemoveItem(address);
		if (s_mMail.Get(address) == computer)
			s_mMail.Remove(address);

		s_aAutoMail.RemoveItem(address);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: default mail address <host>@lan and chat handle @<host> when the computer has none.
	static void ProvisionIdentity(ROOT_AE3_ComputerComponent computer)
	{
		string base = ROOT_AE3_Util.Lower(computer.GetHostname());
		base.Replace(" ", "");
		base.Replace("@", "");
		if (base.IsEmpty())
			base = "laptop";

		if (computer.GetMailAddresses().IsEmpty())
		{
			string addr = base + "@lan";
			int n = 1;
			while (s_mMail.Get(addr) && s_mMail.Get(addr) != computer)
			{
				n++;
				addr = base + n.ToString() + "@lan";
			}

			if (RegisterMailAddress(computer, addr) && !s_aAutoMail.Contains(addr))
				s_aAutoMail.Insert(addr);
		}

		if (HandleOf(computer).IsEmpty())
		{
			string handle = "@" + base;
			int k = 1;
			while (s_mHandles.Get(handle) && s_mHandles.Get(handle) != computer)
			{
				k++;
				handle = "@" + base + k.ToString();
			}

			if (RegisterHandle(computer, handle) && !s_aAutoHandles.Contains(handle))
				s_aAutoHandles.Insert(handle);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Restore: an address held as an automatic identity by another computer moves to this one.
	static bool ClaimMailAddress(ROOT_AE3_ComputerComponent computer, string address)
	{
		address = ROOT_AE3_Util.Lower(address.Trim());
		ROOT_AE3_ComputerComponent owner = s_mMail.Get(address);
		if (owner && owner != computer && s_aAutoMail.Contains(address))
		{
			owner.GetMailAddresses().RemoveItem(address);
			s_mMail.Remove(address);
			s_aAutoMail.RemoveItem(address);
			ProvisionIdentity(owner);
		}

		return RegisterMailAddress(computer, address);
	}

	//------------------------------------------------------------------------------------------------
	//! Restore: a handle held as an automatic identity by another computer moves to this one.
	static bool ClaimHandle(ROOT_AE3_ComputerComponent computer, string handle)
	{
		ROOT_AE3_ComputerComponent owner = s_mHandles.Get(handle);
		if (owner && owner != computer && s_aAutoHandles.Contains(handle))
		{
			s_mHandles.Remove(handle);
			s_aAutoHandles.RemoveItem(handle);
			array<string> left = HandlesOf(owner);
			left.RemoveItem(handle);
			SetHandles(owner, left);
			ProvisionIdentity(owner);
		}

		return RegisterHandle(computer, handle);
	}

	//------------------------------------------------------------------------------------------------
	//! Frees mail addresses and chat handle of a computer (restore, removal).
	static void ReleaseAccounts(ROOT_AE3_ComputerComponent computer)
	{
		foreach (string a : computer.GetMailAddresses())
		{
			if (s_mMail.Get(a) == computer)
				s_mMail.Remove(a);
		}

		foreach (string h : HandlesOf(computer))
		{
			if (s_mHandles.Get(h) == computer)
				s_mHandles.Remove(h);
		}
	}

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_ComputerComponent MailOwner(string address)
	{
		return s_mMail.Get(ROOT_AE3_Util.Lower(address));
	}

	//------------------------------------------------------------------------------------------------
	static string MailText(string from, string to, string subject, string body, string date)
	{
		return string.Format("From: %1\nTo: %2\nSubject: %3\nDate: %4\n\n%5", from, to, subject, date, body);
	}

	//------------------------------------------------------------------------------------------------
	//! Stores a mail in /var/mail of a computer (no delivery checks). Used by editors and the API.
	static void StoreMail(ROOT_AE3_ComputerComponent computer, string from, string to, string subject, string body, string date = "")
	{
		if (date.IsEmpty())
			date = ROOT_AE3_Util.Now();

		string e;
		Prune(computer, "/var/mail");
		computer.GetFilesystem().WriteFile(string.Format("/var/mail/%1.mail", NextMailId()), MailText(from, to, ROOT_AE3_Util.Clip(ROOT_AE3_Codec.Clean(subject), 128), ROOT_AE3_Util.Clip(ROOT_AE3_Codec.Clean(body), MAX_BODY), date), "root", false, e, 63, "root", true);
	}

	//------------------------------------------------------------------------------------------------
	//! Keeps at most MAX_MAILS mail files in a folder; the oldest go first.
	protected static void Prune(ROOT_AE3_ComputerComponent computer, string folder)
	{
		string e;
		ROOT_AE3_FsNode dir = computer.GetFilesystem().Resolve(folder, "root", e);
		if (!dir || !dir.m_bDir)
			return;

		array<string> names = {};
		foreach (string n : dir.GetSortedNames())
		{
			if (n.EndsWith(".mail"))
				names.Insert(n);
		}

		for (int i = 0; i <= names.Count() - MAX_MAILS; i++)
		{
			dir.m_mChildren.Remove(names[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Sends a mail between registered addresses. Both computers must be running.
	static bool SendMail(ROOT_AE3_ComputerComponent sender, string from, string to, string subject, string body, out string reason)
	{
		ROOT_AE3_ComputerComponent target = MailOwner(to);
		if (!target)
		{
			reason = "Unknown recipient.";
			return false;
		}

		if (!target.IsRunning() || !sender.IsRunning() || !Reachable(sender, target))
		{
			reason = "The recipient is not reachable.";
			return false;
		}

		float now = GetGame().GetWorld().GetWorldTime() / 1000.0;
		if (now - s_mLastMail.Get(sender) < MAIL_INTERVAL)
		{
			reason = "Wait a moment before sending the next mail.";
			return false;
		}

		s_mLastMail.Set(sender, now);
		string date = ROOT_AE3_Util.Now();
		StoreMail(target, from, to, subject, body, date);
		string e;
		Prune(sender, "/var/sent");
		sender.GetFilesystem().WriteFile(string.Format("/var/sent/%1.mail", NextMailId()), MailText(from, to, ROOT_AE3_Util.Clip(subject, 128), ROOT_AE3_Util.Clip(body, MAX_BODY), date), "root", false, e, 63, "root", true);
		NotifyOperator(target, "New mail", "From " + from + ": " + subject);
		return true;
	}

	// Chat and instant messages ----------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Own chat handles of a computer, the first is the default sender.
	static array<string> HandlesOf(ROOT_AE3_ComputerComponent computer)
	{
		array<string> res = {};
		string list = computer.GetPref("", "chat_handles", "");
		if (!list.IsEmpty())
			res = ROOT_AE3_Codec.SplitSub(list);

		string legacy = computer.GetPref("", "chat_handle", "");
		if (!legacy.IsEmpty() && !res.Contains(legacy))
			res.InsertAt(legacy, 0);

		return res;
	}

	//------------------------------------------------------------------------------------------------
	protected static void SetHandles(ROOT_AE3_ComputerComponent computer, notnull array<string> handles)
	{
		computer.SetPref("", "chat_handles", ROOT_AE3_Codec.JoinSub(handles));
		string first;
		if (!handles.IsEmpty())
			first = handles[0];

		computer.SetPref("", "chat_handle", first);
	}

	//------------------------------------------------------------------------------------------------
	//! "name" becomes "@name".
	static string NormalizeHandle(string handle)
	{
		handle = handle.Trim();
		if (!handle.IsEmpty() && !handle.StartsWith("@"))
			handle = "@" + handle;

		return handle;
	}

	//------------------------------------------------------------------------------------------------
	//! Adds a chat handle to a computer (at most 8). False when taken by another computer or invalid.
	static bool RegisterHandle(ROOT_AE3_ComputerComponent computer, string handle)
	{
		handle = NormalizeHandle(handle);
		if (handle.Length() < 2 || handle.Length() > 32 || !ROOT_AE3_Filesystem.ValidName(handle))
			return false;

		ROOT_AE3_ComputerComponent owner = s_mHandles.Get(handle);
		if (owner && owner != computer)
			return false;

		array<string> handles = HandlesOf(computer);
		if (!handles.Contains(handle))
		{
			if (handles.Count() >= 8)
				return false;

			handles.Insert(handle);
		}

		s_mHandles.Set(handle, computer);
		SetHandles(computer, handles);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool RemoveHandle(ROOT_AE3_ComputerComponent computer, string handle)
	{
		array<string> handles = HandlesOf(computer);
		if (!handles.Contains(handle))
			return false;

		handles.RemoveItem(handle);
		if (s_mHandles.Get(handle) == computer)
			s_mHandles.Remove(handle);

		s_aAutoHandles.RemoveItem(handle);
		SetHandles(computer, handles);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Default handle of a computer, empty when it has none.
	static string HandleOf(ROOT_AE3_ComputerComponent computer)
	{
		array<string> handles = HandlesOf(computer);
		if (handles.IsEmpty())
			return "";

		return handles[0];
	}

	//------------------------------------------------------------------------------------------------
	static void GetHandles(notnull array<string> outHandles)
	{
		for (int i = 0, n = s_mHandles.Count(); i < n; i++)
		{
			if (s_mHandles.GetElement(i))
				outHandles.Insert(s_mHandles.GetKey(i));
		}

		outHandles.Sort();
	}

	//------------------------------------------------------------------------------------------------
	//! Log file of the conversation between an own handle and a peer.
	static string ChatLog(string self, string peer)
	{
		return "/var/chat/" + self + "/" + peer + ".log";
	}

	//------------------------------------------------------------------------------------------------
	//! Sends a chat message from an own handle (empty = default handle) to a handle on a reachable computer.
	static bool SendChat(ROOT_AE3_ComputerComponent sender, string from, string to, string text, out string reason)
	{
		if (from.IsEmpty())
			from = HandleOf(sender);

		to = NormalizeHandle(to);
		if (from.IsEmpty() || !HandlesOf(sender).Contains(from))
		{
			reason = "Create a handle first.";
			return false;
		}

		float now = GetGame().GetWorld().GetWorldTime() / 1000.0;
		if (now - s_mLastChat.Get(sender) < 0.5)
		{
			reason = "Wait a moment before sending the next message.";
			return false;
		}

		s_mLastChat.Set(sender, now);

		ROOT_AE3_ComputerComponent target = s_mHandles.Get(to);
		if (!target || !target.IsRunning() || !Reachable(sender, target))
		{
			reason = to + " is not reachable.";
			return false;
		}

		text = OneLine(text);
		if (text.Length() > 1024)
			text = text.Substring(0, 1024);

		string line = string.Format("[%1] %2: %3\n", ROOT_AE3_Util.Now(), from, text);
		string e;
		sender.GetFilesystem().CreateDir("/var/chat/" + from, "root", "root", 63, e, true);
		target.GetFilesystem().CreateDir("/var/chat/" + to, "root", "root", 63, e, true);
		sender.GetFilesystem().AppendLog(ChatLog(from, to), line, 63);
		if (target != sender || from != to)
			target.GetFilesystem().AppendLog(ChatLog(to, from), line, 63);

		NotifyOperator(target, "Message from " + from, text, ROOT_AE3_Sound.MESSAGE);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static void DeliverInstantMessage(ROOT_AE3_ComputerComponent target, string fromIp, string fromUser, string text)
	{
		text = ROOT_AE3_Util.Clip(OneLine(text), 1024);
		string line = string.Format("[%1] %2@%3: %4\n", ROOT_AE3_Util.Now(), fromUser, fromIp, text);
		target.GetFilesystem().AppendLog("/var/mail/inbox", line, 63);
		NotifyOperator(target, "Message from " + fromIp, text, ROOT_AE3_Sound.MESSAGE);
	}

	//------------------------------------------------------------------------------------------------
	//! Message text as one line (log files hold one message per line).
	protected static string OneLine(string text)
	{
		text = ROOT_AE3_Codec.Clean(text);
		text.Replace("\r", " ");
		text.Replace("\n", " ");
		return text;
	}

	//------------------------------------------------------------------------------------------------
	static void NotifyOperator(ROOT_AE3_ComputerComponent computer, string title, string text, int sound = ROOT_AE3_Sound.MAIL)
	{
		float now = GetGame().GetWorld().GetWorldTime() / 1000.0;
		if (now - s_mLastNotify.Get(computer) < 1)
			return;

		s_mLastNotify.Set(computer, now);
		ROOT_AE3_Sound.Play(computer.GetOwner(), sound);
		ROOT_AE3_Session s = ROOT_AE3_Manager.Get().GetSessionOf(computer);
		if (s)
			s.Notify(title, text);
	}

	// Web ---------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Page visible on every computer (computer = null) or only on one.
	static void RegisterWebpage(string url, string title, string markdown, ROOT_AE3_ComputerComponent computer = null)
	{
		ROOT_AE3_Webpage page = new ROOT_AE3_Webpage();
		page.m_sUrl = NormalizeUrl(url);
		page.m_sTitle = ROOT_AE3_Codec.Clean(title);
		page.m_sContent = ROOT_AE3_Codec.Clean(markdown);
		if (computer)
			computer.GetWebpages().Set(page.m_sUrl, page);
		else
			s_mPages.Set(page.m_sUrl, page);
	}

	//------------------------------------------------------------------------------------------------
	//! Website: pages of one domain. Paths are relative ("" = index).
	static void RegisterSite(string domain, notnull array<string> paths, notnull array<string> titles, notnull array<string> contents)
	{
		domain = NormalizeUrl(domain);
		for (int i = 0; i < paths.Count(); i++)
		{
			string p = paths[i];
			string url = domain;
			if (!p.IsEmpty() && p != "/" && p != "index")
				url = domain + "/" + p;

			string title, content;
			if (i < titles.Count())
				title = titles[i];

			if (i < contents.Count())
				content = contents[i];

			RegisterWebpage(url, title, content);
		}
	}

	//------------------------------------------------------------------------------------------------
	static string NormalizeUrl(string url)
	{
		url = url.Trim();
		url.ToLower();
		if (url.StartsWith("http://"))
			url = url.Substring(7, url.Length() - 7);

		if (url.StartsWith("https://"))
			url = url.Substring(8, url.Length() - 8);

		while (url.EndsWith("/"))
		{
			url = url.Substring(0, url.Length() - 1);
		}

		return url;
	}

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_Webpage FindPage(ROOT_AE3_ComputerComponent computer, string url)
	{
		url = NormalizeUrl(url);
		ROOT_AE3_Webpage page;
		if (computer)
			page = computer.GetWebpages().Get(url);

		if (!page)
			page = s_mPages.Get(url);

		return page;
	}

	//------------------------------------------------------------------------------------------------
	//! URLs visible to a computer.
	static void ListPages(ROOT_AE3_ComputerComponent computer, notnull array<string> outUrls)
	{
		for (int i = 0, n = s_mPages.Count(); i < n; i++)
		{
			outUrls.Insert(s_mPages.GetKey(i));
		}

		if (computer)
		{
			map<string, ref ROOT_AE3_Webpage> own = computer.GetWebpages();
			for (int i = 0, n = own.Count(); i < n; i++)
			{
				if (!outUrls.Contains(own.GetKey(i)))
					outUrls.Insert(own.GetKey(i));
			}
		}

		outUrls.Sort();
	}

	//------------------------------------------------------------------------------------------------
	static void AddBrowserHistory(ROOT_AE3_ComputerComponent computer, string url, string date = "")
	{
		if (date.IsEmpty())
			date = ROOT_AE3_Util.Now();

		computer.GetFilesystem().AppendLog("/var/log/browser_history", date + "|" + NormalizeUrl(url) + "\n", 15);
	}

	//------------------------------------------------------------------------------------------------
	//! Seeds the RootNet home page once.
	static void EnsureHomePage()
	{
		if (s_mPages.Contains("rootnet.home"))
			return;

		string md = "# RootNet\n\nWelcome to the RootNet, the network of this area of operations.\n\n## Pages\n\nOpen the address bar and enter an address, or use the links on the pages registered by the mission.\n\n[About armaOS](rootnet.home/about)";
		RegisterWebpage("rootnet.home", "RootNet", md);
		RegisterWebpage("rootnet.home/about", "About armaOS", "# armaOS\n\nDesktop and terminal operating system of Advanced Equipment.\n\n[Back](rootnet.home)");
	}
}
