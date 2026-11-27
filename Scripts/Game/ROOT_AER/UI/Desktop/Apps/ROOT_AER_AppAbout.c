//! About armaOS: product card with the BIOS version and copyright of the game setting.
class ROOT_AER_AppAbout : ROOT_AER_App
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
		ROOT_AER_Settings s = ROOT_AER_SettingsManager.Get();
		ROOT_AER_Client c = ROOT_AER_Client.Get();
		float w = Width();
		float h = Height();
		ROOT_AER_UI.Rect(GetContent(), 0, 0, w, h, ROOT_AER_Theme.Panel());
		ROOT_AER_UI.Rect(GetContent(), 0, 0, w, 108, ROOT_AER_Theme.Sidebar());
		ROOT_AER_UI.Rect(GetContent(), 0, 108, w, 1, ROOT_AER_Theme.Border());
		ROOT_AER_UI.Icon(GetContent(), 24, 22, 64, "logo");
		ROOT_AER_UI.Text(GetContent(), 104, 26, w - 120, 32, ROOT_AER_Theme.Caption("armaOS"), 26, ROOT_AER_Theme.Text());
		ROOT_AER_UI.Text(GetContent(), 104, 64, w - 120, 18, "Advanced Equipment Reforged for Arma Reforger - by Root", 13, ROOT_AER_Theme.Muted());
		float y = Heading(24, 124, w - 48, "This computer");
		y = KeyValue(24, y, w - 48, "Host", c.m_sHostname, 110);
		y = KeyValue(24, y, w - 48, "Style", ROOT_AER_Theme.StyleName(c.m_iStyle) + ", " + ROOT_AER_Theme.Name(c.m_sTheme.ToInt()), 110);
		ROOT_AER_UI.Text(GetContent(), 24, y + 6, w - 48, h - y - 12, s.m_sTerminalBiosVersion + "\n" + s.m_sTerminalCopyright, 12, ROOT_AER_Theme.Muted(), false, true);
	}
}
