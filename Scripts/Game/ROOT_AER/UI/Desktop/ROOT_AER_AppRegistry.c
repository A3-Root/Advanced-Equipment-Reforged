//! Desktop apps in start menu order. Other mods add their own with ROOT_AER_API.RegisterExtApp.
class ROOT_AER_AppRegistry
{
	protected static ref array<typename> s_aExternal = {};

	//------------------------------------------------------------------------------------------------
	static void RegisterExternal(typename appType)
	{
		if (appType.IsInherited(ROOT_AER_App) && !s_aExternal.Contains(appType))
			s_aExternal.Insert(appType);
	}

	//------------------------------------------------------------------------------------------------
	static void GetApps(notnull array<typename> outApps)
	{
		outApps.Insert(ROOT_AER_AppMyComputer);
		outApps.Insert(ROOT_AER_AppFiles);
		outApps.Insert(ROOT_AER_AppNotepad);
		outApps.Insert(ROOT_AER_AppMedia);
		outApps.Insert(ROOT_AER_AppTerminal);
		outApps.Insert(ROOT_AER_AppBrowser);
		outApps.Insert(ROOT_AER_AppMail);
		outApps.Insert(ROOT_AER_AppMessenger);
		outApps.Insert(ROOT_AER_AppCalendar);
		outApps.Insert(ROOT_AER_AppNetwork);
		outApps.Insert(ROOT_AER_AppSsh);
		outApps.Insert(ROOT_AER_AppPing);
		outApps.Insert(ROOT_AER_AppMap);
		outApps.Insert(ROOT_AER_AppCctv);
		outApps.Insert(ROOT_AER_AppGames);
		outApps.Insert(ROOT_AER_AppSnake);
		outApps.Insert(ROOT_AER_AppCalculator);
		outApps.Insert(ROOT_AER_AppRecycleBin);
		outApps.Insert(ROOT_AER_AppSettings);
		outApps.Insert(ROOT_AER_AppSysInfo);
		outApps.Insert(ROOT_AER_AppAbout);
		foreach (typename t : s_aExternal)
		{
			outApps.Insert(t);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Wallpaper texture 1-5; empty for invalid index.
	static ResourceName Wallpaper(int index)
	{
		switch (index)
		{
			case 1: return "{8037D7105C1FA693}UI/Textures/ROOT_AER/Wallpapers/wallpaper_1.edds";
			case 2: return "{DEDFFE53E392289C}UI/Textures/ROOT_AER/Wallpapers/wallpaper_2.edds";
			case 3: return "{2AD7B9CBEE4F40E8}UI/Textures/ROOT_AER/Wallpapers/wallpaper_3.edds";
			case 4: return "{630FACD49C893482}UI/Textures/ROOT_AER/Wallpapers/wallpaper_4.edds";
			case 5: return "{9707EB4C91545CF6}UI/Textures/ROOT_AER/Wallpapers/wallpaper_5.edds";
		}

		return "";
	}
}
