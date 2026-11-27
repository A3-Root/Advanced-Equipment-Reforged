//! About armaOS: product card with the BIOS version and copyright of the game setting.
class ROOT_AE3_AppAbout : ROOT_AE3_App
{
	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "About";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "logo";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(500, 320, 0);
	}

	//------------------------------------------------------------------------------------------------
	override void Build()
	{
		ROOT_AE3_Settings s = ROOT_AE3_SettingsManager.Get();
		ROOT_AE3_Client c = ROOT_AE3_Client.Get();
		float w = Width();
		float h = Height();
		ROOT_AE3_UI.Rect(GetContent(), 0, 0, w, h, ROOT_AE3_Theme.Panel());
		ROOT_AE3_UI.Rect(GetContent(), 0, 0, w, 108, ROOT_AE3_Theme.Sidebar());
		ROOT_AE3_UI.Rect(GetContent(), 0, 108, w, 1, ROOT_AE3_Theme.Border());
		ROOT_AE3_UI.Icon(GetContent(), 24, 22, 64, "logo");
		ROOT_AE3_UI.Text(GetContent(), 104, 26, w - 120, 32, ROOT_AE3_Theme.Caption("armaOS"), 26, ROOT_AE3_Theme.Text());
		ROOT_AE3_UI.Text(GetContent(), 104, 64, w - 120, 18, "Advanced Equipment for Arma Reforger - by Root", 13, ROOT_AE3_Theme.Muted());
		float y = Heading(24, 124, w - 48, "This computer");
		y = KeyValue(24, y, w - 48, "Host", c.m_sHostname, 110);
		y = KeyValue(24, y, w - 48, "Style", ROOT_AE3_Theme.StyleName(c.m_iStyle) + ", " + ROOT_AE3_Theme.Name(c.m_sTheme.ToInt()), 110);
		ROOT_AE3_UI.Text(GetContent(), 24, y + 6, w - 48, h - y - 12, s.m_sTerminalBiosVersion + "\n" + s.m_sTerminalCopyright, 12, ROOT_AE3_Theme.Muted(), false, true);
	}
}
