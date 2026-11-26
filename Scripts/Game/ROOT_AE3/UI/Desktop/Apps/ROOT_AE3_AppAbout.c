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
		string text = "armaOS\n\nAdvanced Equipment for Arma Reforger\nRoot, y0014984, Wasserstoff, JulesVerner\n\n" + s.m_sTerminalBiosVersion + "\n" + s.m_sTerminalCopyright;
		Label(14, 12, Width() - 28, Height() - 24, text, 15, false, true);
	}
}
