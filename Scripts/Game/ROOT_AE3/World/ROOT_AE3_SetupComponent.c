[ComponentEditorProps(category: "Advanced Equipment", description: "Initial content of a computer: files, mail, web pages, calendar, history")]
class ROOT_AE3_SetupComponentClass : ScriptComponentClass
{
}

//! Applied once by ROOT_AE3_ComputerComponent on the authority after its filesystem exists.
class ROOT_AE3_SetupComponent : ScriptComponent
{
	[Attribute(desc: "Files and directories", category: "Content")]
	protected ref array<ref ROOT_AE3_FileEntry> m_aFiles;

	[Attribute(desc: "Mail addresses of this computer", category: "Content")]
	protected ref array<string> m_aMailAddresses;

	[Attribute(desc: "Mails in /var/mail", category: "Content")]
	protected ref array<ref ROOT_AE3_EmailEntry> m_aEmails;

	[Attribute("", desc: "Chat handle of this computer", category: "Content")]
	protected string m_sChatHandle;

	[Attribute(desc: "Web pages", category: "Content")]
	protected ref array<ref ROOT_AE3_WebpageEntry> m_aWebpages;

	[Attribute(desc: "Browser history (addresses)", category: "Content")]
	protected ref array<string> m_aBrowserHistory;

	[Attribute(desc: "Calendar events", category: "Content")]
	protected ref array<ref ROOT_AE3_CalendarEntry> m_aCalendar;

	[Attribute("0", desc: "Desktop wallpaper index 1-5, 0 for plain black", params: "0 5 1", category: "Content")]
	protected int m_iWallpaper;

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_SetupComponent Of(IEntity ent)
	{
		if (!ent)
			return null;

		return ROOT_AE3_SetupComponent.Cast(ent.FindComponent(ROOT_AE3_SetupComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! Web pages for every computer; registered again after a restore (they are not stored per computer).
	void ApplyGlobal()
	{
		if (!m_aWebpages)
			return;

		foreach (ROOT_AE3_WebpageEntry w : m_aWebpages)
		{
			if (w && w.m_bGlobal)
				ROOT_AE3_Intel.RegisterWebpage(w.m_sUrl, w.m_sTitle, w.m_sContent);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Apply(ROOT_AE3_ComputerComponent computer)
	{
		ROOT_AE3_Filesystem fs = computer.GetFilesystem();
		string err;
		if (m_aFiles)
		{
			foreach (ROOT_AE3_FileEntry f : m_aFiles)
			{
				if (f && !f.Apply(fs, err))
					ROOT_AE3_Log.Warn(string.Format("%1: file '%2' failed: %3", GetOwner(), f.m_sPath, err));
			}
		}

		if (m_aMailAddresses)
		{
			foreach (string a : m_aMailAddresses)
			{
				ROOT_AE3_Intel.RegisterMailAddress(computer, a);
			}
		}

		if (m_aEmails)
		{
			foreach (ROOT_AE3_EmailEntry m : m_aEmails)
			{
				if (!m)
					continue;

				string to = m.m_sTo;
				if (to.IsEmpty() && !computer.GetMailAddresses().IsEmpty())
					to = computer.GetMailAddresses()[0];

				ROOT_AE3_Intel.StoreMail(computer, m.m_sFrom, to, m.m_sSubject, m.m_sBody, m.m_sDate);
			}
		}

		if (!m_sChatHandle.IsEmpty())
			ROOT_AE3_Intel.RegisterHandle(computer, m_sChatHandle);

		if (m_aWebpages)
		{
			foreach (ROOT_AE3_WebpageEntry w : m_aWebpages)
			{
				if (!w)
					continue;

				if (w.m_bGlobal)
					ROOT_AE3_Intel.RegisterWebpage(w.m_sUrl, w.m_sTitle, w.m_sContent);
				else
					ROOT_AE3_Intel.RegisterWebpage(w.m_sUrl, w.m_sTitle, w.m_sContent, computer);
			}
		}

		if (m_aBrowserHistory)
		{
			foreach (string url : m_aBrowserHistory)
			{
				ROOT_AE3_Intel.AddBrowserHistory(computer, url);
			}
		}

		if (m_aCalendar)
		{
			foreach (ROOT_AE3_CalendarEntry c : m_aCalendar)
			{
				if (!c)
					continue;

				string date = c.m_sDate;
				if (date.IsEmpty())
					date = ROOT_AE3_Util.Today();

				computer.AddCalendarEvent(date, c.m_sTime, c.m_sTitle, c.m_sText, c.m_sLocation);
			}
		}

		computer.SetPref("", "wallpaper", m_iWallpaper.ToString());
	}
}
