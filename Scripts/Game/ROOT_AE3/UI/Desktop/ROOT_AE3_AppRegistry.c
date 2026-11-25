//! Desktop apps in start menu order. Other mods add their own with ROOT_AE3_API.RegisterExtApp.
class ROOT_AE3_AppRegistry
{
	protected static ref array<typename> s_aExternal = {};

	//------------------------------------------------------------------------------------------------
	static void RegisterExternal(typename appType)
	{
		if (appType.IsInherited(ROOT_AE3_App) && !s_aExternal.Contains(appType))
			s_aExternal.Insert(appType);
	}

	//------------------------------------------------------------------------------------------------
	static void GetApps(notnull array<typename> outApps)
	{
		outApps.Insert(ROOT_AE3_AppMyComputer);
		outApps.Insert(ROOT_AE3_AppFiles);
		outApps.Insert(ROOT_AE3_AppNotepad);
		outApps.Insert(ROOT_AE3_AppMedia);
		outApps.Insert(ROOT_AE3_AppTerminal);
		outApps.Insert(ROOT_AE3_AppBrowser);
		outApps.Insert(ROOT_AE3_AppMail);
		outApps.Insert(ROOT_AE3_AppMessenger);
		outApps.Insert(ROOT_AE3_AppCalendar);
		outApps.Insert(ROOT_AE3_AppNetwork);
		outApps.Insert(ROOT_AE3_AppSsh);
		outApps.Insert(ROOT_AE3_AppPing);
		outApps.Insert(ROOT_AE3_AppMap);
		outApps.Insert(ROOT_AE3_AppCctv);
		outApps.Insert(ROOT_AE3_AppGames);
		outApps.Insert(ROOT_AE3_AppSnake);
		outApps.Insert(ROOT_AE3_AppCalculator);
		outApps.Insert(ROOT_AE3_AppRecycleBin);
		outApps.Insert(ROOT_AE3_AppSettings);
		outApps.Insert(ROOT_AE3_AppSysInfo);
		outApps.Insert(ROOT_AE3_AppAbout);
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
			case 1: return "{8037D7105C1FA693}UI/Textures/ROOT_AE3/Wallpapers/wallpaper_1.edds";
			case 2: return "{DEDFFE53E392289C}UI/Textures/ROOT_AE3/Wallpapers/wallpaper_2.edds";
			case 3: return "{2AD7B9CBEE4F40E8}UI/Textures/ROOT_AE3/Wallpapers/wallpaper_3.edds";
			case 4: return "{630FACD49C893482}UI/Textures/ROOT_AE3/Wallpapers/wallpaper_4.edds";
			case 5: return "{9707EB4C91545CF6}UI/Textures/ROOT_AE3/Wallpapers/wallpaper_5.edds";
		}

		return "";
	}
}
