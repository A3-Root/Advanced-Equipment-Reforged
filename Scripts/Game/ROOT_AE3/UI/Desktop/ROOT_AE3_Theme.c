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
	static Color Desktop()
	{
		if (s_iTheme == 1)
			return ROOT_AE3_UI.Rgb(0.55, 0.65, 0.75);

		if (s_iTheme == 2)
			return ROOT_AE3_UI.Rgb(0.25, 0.3, 0.18);

		return ROOT_AE3_UI.Rgb(0.11, 0.14, 0.2);
	}

	//------------------------------------------------------------------------------------------------
	static Color Panel()
	{
		if (s_iTheme == 1)
			return ROOT_AE3_UI.Rgb(0.93, 0.93, 0.93);

		if (s_iTheme == 2)
			return ROOT_AE3_UI.Rgb(0.2, 0.22, 0.15);

		return ROOT_AE3_UI.Rgb(0.15, 0.16, 0.18);
	}

	//------------------------------------------------------------------------------------------------
	static Color Bar()
	{
		if (s_iTheme == 1)
			return ROOT_AE3_UI.Rgb(0.78, 0.8, 0.83);

		if (s_iTheme == 2)
			return ROOT_AE3_UI.Rgb(0.32, 0.36, 0.22);

		return ROOT_AE3_UI.Rgb(0.08, 0.08, 0.1);
	}

	//------------------------------------------------------------------------------------------------
	static Color Accent()
	{
		if (s_iTheme == 2)
			return ROOT_AE3_UI.Rgb(0.55, 0.6, 0.3);

		return ROOT_AE3_UI.Rgb(0.91, 0.33, 0.13);
	}

	//------------------------------------------------------------------------------------------------
	static Color Text()
	{
		if (s_iTheme == 1)
			return ROOT_AE3_UI.Rgb(0.08, 0.08, 0.08);

		return ROOT_AE3_UI.Rgb(0.92, 0.92, 0.92);
	}

	//------------------------------------------------------------------------------------------------
	static Color Muted()
	{
		if (s_iTheme == 1)
			return ROOT_AE3_UI.Rgb(0.35, 0.35, 0.35);

		return ROOT_AE3_UI.Rgb(0.6, 0.62, 0.65);
	}

	//------------------------------------------------------------------------------------------------
	static Color Field()
	{
		if (s_iTheme == 1)
			return ROOT_AE3_UI.Rgb(1, 1, 1);

		return ROOT_AE3_UI.Rgb(0.07, 0.07, 0.08);
	}

	//------------------------------------------------------------------------------------------------
	static Color Button()
	{
		if (s_iTheme == 1)
			return ROOT_AE3_UI.Rgb(0.82, 0.83, 0.86);

		return ROOT_AE3_UI.Rgb(0.26, 0.28, 0.31);
	}

	//------------------------------------------------------------------------------------------------
	static Color Select()
	{
		return ROOT_AE3_UI.Rgb(0.91, 0.33, 0.13, 0.35);
	}
}
