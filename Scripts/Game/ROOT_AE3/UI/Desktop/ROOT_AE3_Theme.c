//! Desktop themes: 0 Dark, 1 Light, 2 Olive.
class ROOT_AE3_Theme
{
	static int s_iTheme;

	//------------------------------------------------------------------------------------------------
	static string Name(int theme)
	{
		if (theme == 1)
			return "Light";

		if (theme == 2)
			return "Olive";

		return "Dark";
	}

	//------------------------------------------------------------------------------------------------
	protected static Color Pick(Color dark, Color light, Color olive)
	{
		if (s_iTheme == 1)
			return light;

		if (s_iTheme == 2)
			return olive;

		return dark;
	}

	//------------------------------------------------------------------------------------------------
	//! Desktop background without wallpaper.
	static Color Desktop()
	{
		return Pick(ROOT_AE3_UI.Rgb(0.04, 0.16, 0.3), ROOT_AE3_UI.Rgb(0.0, 0.45, 0.78), ROOT_AE3_UI.Rgb(0.25, 0.3, 0.18));
	}

	//------------------------------------------------------------------------------------------------
	//! Window background.
	static Color Panel()
	{
		return Pick(ROOT_AE3_UI.Rgb(0.125, 0.125, 0.13), ROOT_AE3_UI.Rgb(0.953, 0.953, 0.953), ROOT_AE3_UI.Rgb(0.19, 0.21, 0.15));
	}

	//------------------------------------------------------------------------------------------------
	//! Window title bar.
	static Color Title()
	{
		return Pick(ROOT_AE3_UI.Rgb(0.165, 0.165, 0.17), ROOT_AE3_UI.Rgb(0.91, 0.92, 0.93), ROOT_AE3_UI.Rgb(0.26, 0.29, 0.19));
	}

	//------------------------------------------------------------------------------------------------
	//! List headers, status bar, start menu footer.
	static Color Bar()
	{
		return Pick(ROOT_AE3_UI.Rgb(0.16, 0.16, 0.17), ROOT_AE3_UI.Rgb(0.9, 0.9, 0.91), ROOT_AE3_UI.Rgb(0.24, 0.27, 0.18));
	}

	//------------------------------------------------------------------------------------------------
	static Color Taskbar()
	{
		return Pick(ROOT_AE3_UI.Rgb(0.1, 0.1, 0.11, 0.9), ROOT_AE3_UI.Rgb(0.93, 0.94, 0.96, 0.9), ROOT_AE3_UI.Rgb(0.16, 0.18, 0.11, 0.92));
	}

	//------------------------------------------------------------------------------------------------
	//! File manager navigation pane.
	static Color Sidebar()
	{
		return Pick(ROOT_AE3_UI.Rgb(0.15, 0.15, 0.155), ROOT_AE3_UI.Rgb(0.93, 0.93, 0.94), ROOT_AE3_UI.Rgb(0.22, 0.24, 0.17));
	}

	//------------------------------------------------------------------------------------------------
	static Color Accent()
	{
		if (s_iTheme == 2)
			return ROOT_AE3_UI.Rgb(0.5, 0.58, 0.26);

		return ROOT_AE3_UI.Rgb(0.0, 0.47, 0.83);
	}

	//------------------------------------------------------------------------------------------------
	static Color Text()
	{
		if (s_iTheme == 1)
			return ROOT_AE3_UI.Rgb(0.1, 0.1, 0.1);

		return ROOT_AE3_UI.Rgb(0.95, 0.95, 0.95);
	}

	//------------------------------------------------------------------------------------------------
	static Color Muted()
	{
		if (s_iTheme == 1)
			return ROOT_AE3_UI.Rgb(0.38, 0.38, 0.4);

		return ROOT_AE3_UI.Rgb(0.65, 0.66, 0.68);
	}

	//------------------------------------------------------------------------------------------------
	//! Edit boxes and list backgrounds.
	static Color Field()
	{
		return Pick(ROOT_AE3_UI.Rgb(0.17, 0.17, 0.175), ROOT_AE3_UI.Rgb(1, 1, 1), ROOT_AE3_UI.Rgb(0.14, 0.16, 0.11));
	}

	//------------------------------------------------------------------------------------------------
	static Color Button()
	{
		return Pick(ROOT_AE3_UI.Rgb(0.22, 0.22, 0.23), ROOT_AE3_UI.Rgb(0.99, 0.99, 0.99), ROOT_AE3_UI.Rgb(0.3, 0.33, 0.22));
	}

	//------------------------------------------------------------------------------------------------
	static Color Border()
	{
		if (s_iTheme == 1)
			return ROOT_AE3_UI.Rgb(0, 0, 0, 0.14);

		return ROOT_AE3_UI.Rgb(1, 1, 1, 0.1);
	}

	//------------------------------------------------------------------------------------------------
	//! Overlay of transparent buttons under the cursor.
	static Color Hover()
	{
		if (s_iTheme == 1)
			return ROOT_AE3_UI.Rgb(0, 0, 0, 0.06);

		return ROOT_AE3_UI.Rgb(1, 1, 1, 0.08);
	}

	//------------------------------------------------------------------------------------------------
	//! Selected list row or icon.
	static Color Select()
	{
		if (s_iTheme == 1)
			return ROOT_AE3_UI.Rgb(0.0, 0.47, 0.83, 0.16);

		if (s_iTheme == 2)
			return ROOT_AE3_UI.Rgb(0.6, 0.7, 0.3, 0.3);

		return ROOT_AE3_UI.Rgb(0.3, 0.6, 0.95, 0.28);
	}

	//------------------------------------------------------------------------------------------------
	static Color Danger()
	{
		return ROOT_AE3_UI.Rgb(0.77, 0.17, 0.11);
	}
}
