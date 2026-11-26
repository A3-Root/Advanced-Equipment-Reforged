//! About armaOS.
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
		return Vector(460, 260, 0);
	}

	//------------------------------------------------------------------------------------------------
	override void Build()
	{
		ROOT_AE3_Settings s = ROOT_AE3_SettingsManager.Get();
		ROOT_AE3_UI.Icon(GetContent(), 18, 18, 64, "logo");
		Label(98, 20, Width() - 110, 30, "armaOS", 24);
		ROOT_AE3_UI.Text(GetContent(), 98, 54, Width() - 110, 20, "Advanced Equipment for Arma Reforger", 14, ROOT_AE3_Theme.Muted());
		ROOT_AE3_UI.Text(GetContent(), 98, 74, Width() - 110, 20, "by Root", 14, ROOT_AE3_Theme.Muted());
		ROOT_AE3_UI.Rect(GetContent(), 18, 106, Width() - 36, 1, ROOT_AE3_Theme.Border());
		Label(18, 118, Width() - 36, Height() - 130, s.m_sTerminalBiosVersion + "\n" + s.m_sTerminalCopyright, 13, false, true);
	}
}
