//! Theme, wallpaper and terminal design.
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
		return Vector(520, 330, 0);
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
			string l = "None";
			if (i > 0)
				l = i.ToString();

			Btn(10 + i * 76, 106, 70, 28, l, 20 + i);
		}

		Label(10, 150, 400, 22, "Terminal design: " + ROOT_AE3_TerminalDesigns.Name(c.m_iDesign));
		Btn(10, 176, 120, 28, "Next design", 30);
		Label(10, 220, Width() - 20, 60, string.Format("Theme %1, wallpaper %2. Settings are stored per user on this computer.", ROOT_AE3_Theme.Name(c.m_sTheme.ToInt()), c.m_sWallpaper), 13, false, true);
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

		if (id == 30)
		{
			c.m_iDesign = (c.m_iDesign + 1) % ROOT_AE3_TerminalDesigns.COUNT;
			Request2("sys_set", "design", c.m_iDesign.ToString());
			Redraw();
			return true;
		}

		return false;
	}
}
