//! Server: computer state as one string, and named saves in $profile:ROOT_AE3/saves.
//! Covers filesystem, users, root password and login policy, hostname, history, preferences, calendar,
//! mail addresses, own web pages, interface mode, design, power level, flash drives in the ports, lid, access
//! lists and installed programs.
class ROOT_AE3_SaveLoad
{
	static const string DIR = "$profile:ROOT_AE3/saves";
	static const string VERSION = "AE3SAVE7";
	static const string VERSION_6 = "AE3SAVE6";		//!< Calendar events without location

	//------------------------------------------------------------------------------------------------
	static string Pack(ROOT_AE3_ComputerComponent c)
	{
		array<string> users = {};
		map<string, string> um = c.GetUserMap();
		for (int i = 0, n = um.Count(); i < n; i++)
		{
			users.Insert(um.GetKey(i));
			users.Insert(um.GetElement(i));
		}

		array<string> hist = {};
		map<string, ref array<string>> hm = c.GetHistoryMap();
		for (int i = 0, n = hm.Count(); i < n; i++)
		{
			foreach (string l : hm.GetElement(i))
			{
				hist.Insert(hm.GetKey(i));
				hist.Insert(l);
			}
		}

		array<string> prefs = {};
		map<string, string> pm = c.GetPrefMap();
		for (int i = 0, n = pm.Count(); i < n; i++)
		{
			prefs.Insert(pm.GetKey(i));
			prefs.Insert(pm.GetElement(i));
		}

		array<string> cal = {};
		foreach (ROOT_AE3_CalendarEvent ev : c.GetCalendar())
		{
			cal.Insert(ev.m_sDate);
			cal.Insert(ev.m_sTime);
			cal.Insert(ev.m_sTitle);
			cal.Insert(ev.m_sText);
			cal.Insert(ev.m_sLocation);
		}

		array<string> pages = {};
		map<string, ref ROOT_AE3_Webpage> wm = c.GetWebpages();
		for (int i = 0, n = wm.Count(); i < n; i++)
		{
			ROOT_AE3_Webpage w = wm.GetElement(i);
			pages.Insert(w.m_sUrl);
			pages.Insert(w.m_sTitle);
			pages.Insert(w.m_sContent);
		}

		string level = "-1";
		ROOT_AE3_PowerComponent p = c.GetPower();
		if (p)
			level = p.GetLevel().ToString();

		array<string> head = {
			VERSION, c.GetHostname(), c.GetRootPasswordRaw(), c.GetRootLoginPolicy().ToString(), c.GetInterfaceModeRaw().ToString(),
			c.GetDesignRaw().ToString(), level, ROOT_AE3_Codec.JoinSub(users), ROOT_AE3_Codec.JoinSub(hist), ROOT_AE3_Codec.JoinSub(prefs),
			ROOT_AE3_Codec.JoinSub(cal), ROOT_AE3_Codec.JoinSub(c.GetMailAddresses()), ROOT_AE3_Codec.JoinSub(pages)
		};

		// USB slots: port, prefab, escaped label, escaped filesystem, prop prefab, mounted, capacity; seven lines per slot.
		string usb;
		for (int i = 0; i < c.GetUsbPortCount(); i++)
		{
			ROOT_AE3_UsbSlot slot = c.GetUsbDrive(i);
			if (slot)
				usb += i.ToString() + "\n" + slot.m_sPrefab + "\n" + EscapeFs(slot.m_sLabel) + "\n" + EscapeFs(slot.m_Fs.Serialize()) + "\n" + slot.m_sPropPrefab + "\n" + ROOT_AE3_Util.BoolStr(c.IsMounted(i)) + "\n" + slot.m_iCapacity.ToString() + "\n";
		}

		head.Insert(usb);
		string lid = "1";
		ROOT_AE3_LidComponent lidComp = ROOT_AE3_LidComponent.Of(c.GetOwner());
		if (lidComp && !lidComp.IsOpen())
			lid = "0";

		head.Insert(lid);
		array<string> cli = {};
		array<string> gui = {};
		c.GetAccess(0, cli);
		c.GetAccess(1, gui);
		head.Insert(ROOT_AE3_Codec.JoinSub(cli));
		head.Insert(ROOT_AE3_Codec.JoinSub(gui));
		head.Insert(ROOT_AE3_Codec.JoinSub(c.GetInstalled()));
		// The filesystem uses both separators itself; it is appended after a fixed number of header fields.
		return ROOT_AE3_Codec.Join(head) + ROOT_AE3_Codec.Sep1() + c.GetFilesystem().Serialize();
	}

	//------------------------------------------------------------------------------------------------
	static bool Unpack(ROOT_AE3_ComputerComponent c, string data)
	{
		array<string> parts = ROOT_AE3_Codec.Split(data);
		if (parts.Count() < 19 || (parts[0] != VERSION && parts[0] != VERSION_6))
			return false;

		string fsData;
		for (int i = 18; i < parts.Count(); i++)
		{
			if (i > 18)
				fsData += ROOT_AE3_Codec.Sep1();

			fsData += parts[i];
		}

		ROOT_AE3_Filesystem restoredFs = ROOT_AE3_Filesystem.Deserialize(fsData);
		if (!restoredFs)
			return false;

		c.ClearAccounts();
		c.SetHostname(parts[1]);
		c.SetRootPassword(parts[2]);
		c.SetRootLogin(parts[3].ToInt());
		c.SetInterfaceMode(parts[4].ToInt());
		if (parts[5].ToInt() >= 0)
			c.SetDesign(parts[5].ToInt());

		float level = parts[6].ToFloat();
		if (level >= 0 && c.GetPower())
			c.GetPower().SetLevel(level);

		array<string> users = ROOT_AE3_Codec.SplitSub(parts[7]);
		for (int i = 0; i + 1 < users.Count(); i += 2)
		{
			c.GetUserMap().Set(users[i], users[i + 1]);
		}

		c.GetHistoryMap().Clear();
		array<string> hist = ROOT_AE3_Codec.SplitSub(parts[8]);
		for (int i = 0; i + 1 < hist.Count(); i += 2)
		{
			c.AddHistory(hist[i], hist[i + 1]);
		}

		array<string> prefs = ROOT_AE3_Codec.SplitSub(parts[9]);
		for (int i = 0; i + 1 < prefs.Count(); i += 2)
		{
			c.GetPrefMap().Set(prefs[i], prefs[i + 1]);
		}

		c.GetCalendar().Clear();
		array<string> cal = ROOT_AE3_Codec.SplitSub(parts[10]);
		int stride = 5;
		if (parts[0] == VERSION_6)
			stride = 4;

		for (int i = 0; i + stride - 1 < cal.Count(); i += stride)
		{
			string location;
			if (stride == 5)
				location = cal[i + 4];

			c.AddCalendarEvent(cal[i], cal[i + 1], cal[i + 2], cal[i + 3], location);
		}

		foreach (string a : ROOT_AE3_Codec.SplitSub(parts[11]))
		{
			ROOT_AE3_Intel.ClaimMailAddress(c, a);
		}

		array<string> pages = ROOT_AE3_Codec.SplitSub(parts[12]);
		for (int i = 0; i + 2 < pages.Count(); i += 3)
		{
			ROOT_AE3_Intel.RegisterWebpage(pages[i], pages[i + 1], pages[i + 2], c);
		}

		for (int p = 0; p < c.GetUsbPortCount(); p++)
		{
			ROOT_AE3_UsbSlot old = c.GetUsbDrive(p);
			if (old && old.m_Prop)
				SCR_EntityHelper.DeleteEntityAndChildren(old.m_Prop);

			c.SetUsbDrive(p, null);
		}

		array<string> usb = {};
		array<int> mounted = {};
		parts[13].Split("\n", usb, false);
		for (int i = 0; i + 6 < usb.Count(); i += 7)
		{
			if (usb[i + 5] == "1")
				mounted.Insert(usb[i].ToInt());

			ROOT_AE3_UsbSlot slot = new ROOT_AE3_UsbSlot();
			slot.m_sPrefab = usb[i + 1];
			slot.m_sLabel = UnescapeFs(usb[i + 2]);
			slot.m_iCapacity = usb[i + 6].ToInt();
			slot.m_Fs = ROOT_AE3_Filesystem.Deserialize(UnescapeFs(usb[i + 3]));
			slot.m_sPropPrefab = usb[i + 4];
			c.SetUsbDrive(usb[i].ToInt(), slot);
			slot.m_Prop = ROOT_AE3_UsbManager.SpawnProp(c, usb[i].ToInt(), slot.m_sPropPrefab);
		}

		ROOT_AE3_LidComponent lidComp = ROOT_AE3_LidComponent.Of(c.GetOwner());
		if (lidComp)
			lidComp.SetOpen(parts[14] != "0");

		array<string> cliAccess = ROOT_AE3_Codec.SplitSub(parts[15]);
		array<string> guiAccess = ROOT_AE3_Codec.SplitSub(parts[16]);
		c.SetAccess(0, cliAccess);
		c.SetAccess(1, guiAccess);
		c.GetInstalled().Clear();
		foreach (string program : ROOT_AE3_Codec.SplitSub(parts[17]))
		{
			if (!program.IsEmpty())
				c.MarkInstalled(program);
		}

		c.SetFilesystem(restoredFs);
		foreach (int port : mounted)
		{
			string mountReason;
			c.Mount(port, mountReason);
		}

		c.GetTerminal().Reset();
		foreach (string handle : ROOT_AE3_Intel.HandlesOf(c))
		{
			ROOT_AE3_Intel.ClaimHandle(c, handle);
		}

		ROOT_AE3_SetupComponent setup = ROOT_AE3_SetupComponent.Of(c.GetOwner());
		if (setup)
			setup.ApplyGlobal();
		if (c.IsReady())
			ROOT_AE3_Manager.Get().NotifyDeviceReady(ROOT_AE3_DeviceComponent.Of(c.GetOwner()), true);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Flash drive filesystems are nested in one header field: separators and newlines are escaped.
	protected static string EscapeFs(string s)
	{
		s.Replace("%", "%25");
		s.Replace("\n", "%0A");
		s.Replace(ROOT_AE3_Codec.Sep1(), "%1E");
		s.Replace(ROOT_AE3_Codec.Sep2(), "%1F");
		return s;
	}

	//------------------------------------------------------------------------------------------------
	protected static string UnescapeFs(string s)
	{
		s.Replace("%1F", ROOT_AE3_Codec.Sep2());
		s.Replace("%1E", ROOT_AE3_Codec.Sep1());
		s.Replace("%0A", "\n");
		s.Replace("%25", "%");
		return s;
	}

	//------------------------------------------------------------------------------------------------
	static string FileOf(string name)
	{
		string clean;
		for (int i = 0; i < name.Length(); i++)
		{
			int ch = name.ToAscii(i);
			if ((ch >= 48 && ch <= 57) || (ch >= 65 && ch <= 90) || (ch >= 97 && ch <= 122) || ch == 45 || ch == 95)
				clean += name.Get(i);
		}

		if (clean.IsEmpty())
			clean = "unnamed";

		return DIR + "/" + clean + ".txt";
	}

	//------------------------------------------------------------------------------------------------
	static bool Save(ROOT_AE3_ComputerComponent c, string name)
	{
		FileIO.MakeDirectory("$profile:ROOT_AE3");
		FileIO.MakeDirectory(DIR);
		FileHandle fh = FileIO.OpenFile(FileOf(name), FileMode.WRITE);
		if (!fh)
			return false;

		fh.Write(Pack(c));
		fh.Close();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool Restore(ROOT_AE3_ComputerComponent c, string name)
	{
		string path = FileOf(name);
		if (!FileIO.FileExists(path))
			return false;

		FileHandle fh = FileIO.OpenFile(path, FileMode.READ);
		if (!fh)
			return false;

		string data;
		fh.Read(data, fh.GetLength());
		fh.Close();
		ROOT_AE3_Manager.Get().EndSessionsOf(ROOT_AE3_DeviceComponent.Of(c.GetOwner()));
		return Unpack(c, data);
	}

	//------------------------------------------------------------------------------------------------
	static void ListSaves(notnull array<string> outNames)
	{
		ROOT_AE3_FileCollector collector = new ROOT_AE3_FileCollector();
		FileIO.FindFiles(collector.Add, DIR, ".txt");
		foreach (string f : collector.m_aFiles)
		{
			outNames.Insert(FilePath.StripExtension(FilePath.StripPath(f)));
		}
	}
}
