//! Theme, wallpaper (black, built in, image file, web address) and terminal design.
class ROOT_AE3_AppSettings : ROOT_AE3_App
{
	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Settings";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "settings";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(560, 400, 0);
	}

	//------------------------------------------------------------------------------------------------
	override void Build()
	{
		ROOT_AE3_Client c = ROOT_AE3_Client.Get();
		Label(10, 10, 200, 22, "Theme");
		for (int i = 0; i < 3; i++)
		{
			Btn(10 + i * 110, 36, 100, 28, ROOT_AE3_Theme.Name(i), 10 + i);
		}

		Label(10, 80, 200, 22, "Wallpaper");
		for (int i = 0; i <= 5; i++)
		{
			string l = "Black";
			if (i > 0)
				l = i.ToString();

			Btn(10 + i * 76, 106, 70, 28, l, 20 + i);
		}

		bool files = ROOT_AE3_SettingsManager.Get().m_bDesktopEnableFileBrowsing;
		if (files)
			Btn(10, 142, 150, 28, "Picture...", 26);

		if (ROOT_AE3_SettingsManager.Get().m_bDesktopWebImages)
			Btn(168, 142, 150, 28, "Web address...", 27);

		Label(10, 190, 400, 22, "Terminal design: " + ROOT_AE3_TerminalDesigns.Name(c.m_iDesign));
		Btn(10, 216, 120, 28, "Next design", 30);
		string wall = c.m_sWallpaper;
		if (wall == "0" || wall.IsEmpty())
			wall = "black";

		Label(10, 262, Width() - 20, 80, string.Format("Theme %1, wallpaper %2. Picture uses an image file (Photos can save web and base64 images as files). Settings are stored per user on this computer.", ROOT_AE3_Theme.Name(c.m_sTheme.ToInt()), wall), 13, false, true);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		ROOT_AE3_Client c = ROOT_AE3_Client.Get();
		if (id >= 10 && id <= 12)
		{
			Request2("sys_set", "theme", (id - 10).ToString());
			m_Desktop.ApplyPrefs((id - 10).ToString(), c.m_sWallpaper);
			return true;
		}

		if (id >= 20 && id <= 25)
		{
			Request2("sys_set", "wallpaper", (id - 20).ToString());
			m_Desktop.ApplyPrefs(c.m_sTheme, (id - 20).ToString());
			return true;
		}

		if (id == 26)
		{
			string folder = ROOT_AE3_Filesystem.HomeOf(c.m_sUser);
			if (c.m_sWallpaper.StartsWith("/"))
				folder = ROOT_AE3_Filesystem.Parent(c.m_sWallpaper);

			m_Desktop.ShowFilePicker(this, "wallpaper", "Choose a background picture", folder, false, "", "Use");
			return true;
		}

		if (id == 27)
		{
			string cur = "https://";
			if (ROOT_AE3_WebMedia.KindOf(c.m_sWallpaper) == "url")
				cur = c.m_sWallpaper;

			m_Desktop.ShowDialog(this, "wallpaper_url", "Background from the web", "Address of a PNG, JPEG or GIF image:", true, cur, "Use");
			return true;
		}

		if (id == 30)
		{
			c.m_iDesign = (c.m_iDesign + 1) % ROOT_AE3_TerminalDesigns.COUNT;
			Request2("sys_set", "design", c.m_iDesign.ToString());
			Redraw();
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnDialog(string tag, bool ok, string value)
	{
		value = value.Trim();
		if (!ok || value.IsEmpty() || (tag != "wallpaper" && tag != "wallpaper_url"))
			return;

		if (tag == "wallpaper_url" && ROOT_AE3_WebMedia.KindOf(value) != "url")
		{
			Status("Enter an address starting with http:// or https://.");
			return;
		}

		ROOT_AE3_Client c = ROOT_AE3_Client.Get();
		Request2("sys_set", "wallpaper", value);
		m_Desktop.ApplyPrefs(c.m_sTheme, value);
	}
}
