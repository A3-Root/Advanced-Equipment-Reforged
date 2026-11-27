//! Settings with a navigation pane: Personalize (theme, wallpaper), Terminal (design), Style (desktop style of
//! this computer, set by the Game Master, World Editor or game setting). Stored per user on this computer.
class ROOT_AER_AppSettings : ROOT_AER_App
{
	protected static const int ID_NAV = 1;
	protected static const int ID_THEME = 10;
	protected static const int ID_WALL = 20;
	protected static const int ID_PICTURE = 26;
	protected static const int ID_WEB = 27;
	protected static const int ID_DESIGN = 30;
	protected static const float NAV_W = 170;

	protected int m_iPage;

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
		return Vector(720, 460, 0);
	}

	//------------------------------------------------------------------------------------------------
	override void Build()
	{
		float w = Width();
		float h = Height();
		ROOT_AER_UI.Rect(GetContent(), 0, 0, NAV_W, h, ROOT_AER_Theme.Sidebar());
		ROOT_AER_UI.Rect(GetContent(), NAV_W, 0, 1, h, ROOT_AER_Theme.Border());
		ROOT_AER_UI.Rect(GetContent(), NAV_W + 1, 0, w - NAV_W - 1, h, ROOT_AER_Theme.Panel());
		array<string> pages = {"Personalize", "Terminal", "Style"};
		array<string> icons = {"photos", "terminal", "settings"};
		foreach (int i, string p : pages)
		{
			Color bg = ROOT_AER_UI.Rgb(0, 0, 0, 0);
			if (i == m_iPage)
				bg = ROOT_AER_Theme.Select();

			ROOT_AER_UI.ItemButton(GetContent(), 6, 10 + i * 38, NAV_W - 12, 34, icons[i], p, ID_NAV + i, this, 14, null, bg);
		}

		float x = NAV_W + 20;
		float cw = w - x - 20;
		switch (m_iPage)
		{
			case 0: BuildPersonalize(x, cw); break;
			case 1: BuildTerminal(x, cw); break;
			case 2: BuildStyle(x, cw); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildPersonalize(float x, float cw)
	{
		ROOT_AER_Client c = ROOT_AER_Client.Get();
		int theme = c.m_sTheme.ToInt();
		float y = Heading(x, 14, cw, "Theme");
		float tw = Math.Min(150, (cw - 16) / 3);
		for (int i = 0; i < 3; i++)
		{
			Color bg = ROOT_AER_Theme.Button();
			Color fg = ROOT_AER_Theme.Text();
			if (i == theme)
			{
				bg = ROOT_AER_Theme.Accent();
				fg = ROOT_AER_Theme.OnAccent();
			}

			ROOT_AER_UI.Button(GetContent(), x + i * (tw + 8), y, tw, 32, ROOT_AER_Theme.Name(i), ID_THEME + i, this, bg, fg, 14);
		}

		y = Heading(x, y + 50, cw, "Wallpaper");
		float pw = Math.Min(96, (cw - 40) / 6);
		float ph = pw * 0.6;
		string wall = c.m_sWallpaper;
		for (int i = 0; i <= 5; i++)
		{
			float px = x + i * (pw + 8);
			Color edge = ROOT_AER_Theme.Border();
			if (wall == i.ToString() || (i == 0 && wall.IsEmpty()))
				edge = ROOT_AER_Theme.Accent();

			ROOT_AER_UI.Rect(GetContent(), px - 2, y - 2, pw + 4, ph + 4, edge);
			ROOT_AER_UI.Rect(GetContent(), px, y, pw, ph, ROOT_AER_UI.Rgb(0, 0, 0));
			ResourceName res = ROOT_AER_AppRegistry.Wallpaper(i);
			if (!res.IsEmpty())
				ROOT_AER_UI.Picture(GetContent(), px, y, pw, ph, res);
			else
				ROOT_AER_UI.Text(GetContent(), px, y + ph * 0.5 - 9, pw, 18, "Black", 12, ROOT_AER_UI.Rgb(0.8, 0.8, 0.8), false, false, 1);

			ROOT_AER_UI.Hit(GetContent(), px, y, pw, ph, ID_WALL + i, this, ROOT_AER_UI.Rgb(0, 0, 0, 0));
		}

		y += ph + 14;
		float bx = x;
		if (ROOT_AER_SettingsManager.Get().m_bDesktopEnableFileBrowsing)
		{
			Btn(bx, y, 150, 30, "Picture...", ID_PICTURE);
			bx += 158;
		}

		if (ROOT_AER_SettingsManager.Get().m_bDesktopWebImages)
			Btn(bx, y, 150, 30, "Web address...", ID_WEB);

		string shown = wall;
		if (wall == "0" || wall.IsEmpty())
			shown = "black";

		ROOT_AER_UI.Text(GetContent(), x, y + 44, cw, 40, "Current: " + shown + ". Picture uses an image file (Photos saves web and base64 images as files).", 13, ROOT_AER_Theme.Muted(), false, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildTerminal(float x, float cw)
	{
		ROOT_AER_Client c = ROOT_AER_Client.Get();
		float y = Heading(x, 14, cw, "Terminal design");
		y = KeyValue(x, y, cw, "Current", ROOT_AER_TerminalDesigns.Name(c.m_iDesign), 100);
		MainBtn(x, y + 8, 160, 30, "Next design", ID_DESIGN);
		ROOT_AER_UI.Text(GetContent(), x, y + 52, cw, 40, "Colours and font of the terminal of this computer. The terminal also changes it with its Design button.", 13, ROOT_AER_Theme.Muted(), false, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildStyle(float x, float cw)
	{
		ROOT_AER_Client c = ROOT_AER_Client.Get();
		float y = Heading(x, 14, cw, "Desktop style");
		y = KeyValue(x, y, cw, "Style", ROOT_AER_Theme.StyleName(c.m_iStyle), 100);
		y = KeyValue(x, y, cw, "Theme", ROOT_AER_Theme.Name(c.m_sTheme.ToInt()), 100);
		ROOT_AER_UI.Text(GetContent(), x, y + 8, cw, 60, "Windows: icons, rounded tool bars, start menu grid. Tactical: monospace green, amber or night display, outlined controls, program list. The Game Master, World Editor or game setting choose the style of this computer.", 13, ROOT_AER_Theme.Muted(), false, true);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		ROOT_AER_Client c = ROOT_AER_Client.Get();
		if (id >= ID_NAV && id < ID_NAV + 3)
		{
			m_iPage = id - ID_NAV;
			Redraw();
			return true;
		}

		if (id >= ID_THEME && id <= ID_THEME + 2)
		{
			Request2("sys_set", "theme", (id - ID_THEME).ToString());
			m_Desktop.ApplyPrefs((id - ID_THEME).ToString(), c.m_sWallpaper);
			return true;
		}

		if (id >= ID_WALL && id <= ID_WALL + 5)
		{
			Request2("sys_set", "wallpaper", (id - ID_WALL).ToString());
			m_Desktop.ApplyPrefs(c.m_sTheme, (id - ID_WALL).ToString());
			return true;
		}

		if (id == ID_PICTURE)
		{
			string folder = ROOT_AER_Filesystem.HomeOf(c.m_sUser);
			if (c.m_sWallpaper.StartsWith("/"))
				folder = ROOT_AER_Filesystem.Parent(c.m_sWallpaper);

			m_Desktop.ShowFilePicker(this, "wallpaper", "Choose a background picture", folder, false, "", "Use");
			return true;
		}

		if (id == ID_WEB)
		{
			string cur = "https://";
			if (ROOT_AER_WebMedia.KindOf(c.m_sWallpaper) == "url")
				cur = c.m_sWallpaper;

			m_Desktop.ShowDialog(this, "wallpaper_url", "Background from the web", "Address of a PNG, JPEG or GIF image:", true, cur, "Use");
			return true;
		}

		if (id == ID_DESIGN)
		{
			c.m_iDesign = (c.m_iDesign + 1) % ROOT_AER_TerminalDesigns.COUNT;
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

		if (tag == "wallpaper_url" && ROOT_AER_WebMedia.KindOf(value) != "url")
		{
			Status("Enter an address starting with http:// or https://.");
			return;
		}

		ROOT_AER_Client c = ROOT_AER_Client.Get();
		Request2("sys_set", "wallpaper", value);
		m_Desktop.ApplyPrefs(c.m_sTheme, value);
	}
}
