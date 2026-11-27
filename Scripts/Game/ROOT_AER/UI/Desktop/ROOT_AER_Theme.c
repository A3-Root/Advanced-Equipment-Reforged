//! Desktop look. Style (per computer): 0 Windows, 1 Tactical. Theme (per user) inside the style:
//! Windows 0 Dark, 1 Light, 2 Olive; Tactical 0 Green, 1 Amber, 2 Night.
//! Tactical: monospace text, square outlined controls, bracketed titles, no shadows.
class ROOT_AER_Theme
{
	static int s_iTheme;
	static int s_iStyle;

	//------------------------------------------------------------------------------------------------
	static bool IsTactical()
	{
		return s_iStyle == 1;
	}

	//------------------------------------------------------------------------------------------------
	static string Name(int theme)
	{
		if (IsTactical())
		{
			if (theme == 1)
				return "Amber";

			if (theme == 2)
				return "Night";

			return "Green";
		}

		if (theme == 1)
			return "Light";

		if (theme == 2)
			return "Olive";

		return "Dark";
	}

	//------------------------------------------------------------------------------------------------
	static string StyleName(int style)
	{
		if (style == 1)
			return "Tactical";

		return "Windows";
	}

	//------------------------------------------------------------------------------------------------
	//! Title text: Tactical "[ TITLE ]".
	static string Caption(string title)
	{
		if (!IsTactical())
			return title;

		string t = title;
		t.ToUpper();
		return "[ " + t + " ]";
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
	//! Tactical phosphor colour of the theme at brightness k (0-1), alpha a.
	protected static Color Ink(float k, float a = 1)
	{
		if (s_iTheme == 1)
			return ROOT_AER_UI.Rgb(1.0 * k, 0.72 * k, 0.2 * k, a);

		if (s_iTheme == 2)
			return ROOT_AER_UI.Rgb(0.95 * k, 0.3 * k, 0.25 * k, a);

		return ROOT_AER_UI.Rgb(0.4 * k, 1.0 * k, 0.5 * k, a);
	}

	//------------------------------------------------------------------------------------------------
	//! Tactical background tone at brightness k.
	protected static Color Ground(float k, float a = 1)
	{
		if (s_iTheme == 1)
			return ROOT_AER_UI.Rgb(0.06 * k, 0.045 * k, 0.02 * k, a);

		if (s_iTheme == 2)
			return ROOT_AER_UI.Rgb(0.05 * k, 0.02 * k, 0.02 * k, a);

		return ROOT_AER_UI.Rgb(0.02 * k, 0.055 * k, 0.03 * k, a);
	}

	//------------------------------------------------------------------------------------------------
	//! Window background.
	static Color Panel()
	{
		if (IsTactical())
			return Ground(1);

		return Pick(ROOT_AER_UI.Rgb(0.125, 0.125, 0.13), ROOT_AER_UI.Rgb(0.953, 0.953, 0.953), ROOT_AER_UI.Rgb(0.19, 0.21, 0.15));
	}

	//------------------------------------------------------------------------------------------------
	//! Window title bar.
	static Color Title()
	{
		if (IsTactical())
			return Ground(1.8);

		return Pick(ROOT_AER_UI.Rgb(0.165, 0.165, 0.17), ROOT_AER_UI.Rgb(0.91, 0.92, 0.93), ROOT_AER_UI.Rgb(0.26, 0.29, 0.19));
	}

	//------------------------------------------------------------------------------------------------
	//! List headers, status bar, start menu footer.
	static Color Bar()
	{
		if (IsTactical())
			return Ground(1.5);

		return Pick(ROOT_AER_UI.Rgb(0.16, 0.16, 0.17), ROOT_AER_UI.Rgb(0.9, 0.9, 0.91), ROOT_AER_UI.Rgb(0.24, 0.27, 0.18));
	}

	//------------------------------------------------------------------------------------------------
	static Color Taskbar()
	{
		if (IsTactical())
			return Ground(0.8, 0.96);

		return Pick(ROOT_AER_UI.Rgb(0.1, 0.1, 0.11, 0.9), ROOT_AER_UI.Rgb(0.93, 0.94, 0.96, 0.9), ROOT_AER_UI.Rgb(0.16, 0.18, 0.11, 0.92));
	}

	//------------------------------------------------------------------------------------------------
	//! Navigation panes.
	static Color Sidebar()
	{
		if (IsTactical())
			return Ground(1.3);

		return Pick(ROOT_AER_UI.Rgb(0.15, 0.15, 0.155), ROOT_AER_UI.Rgb(0.93, 0.93, 0.94), ROOT_AER_UI.Rgb(0.22, 0.24, 0.17));
	}

	//------------------------------------------------------------------------------------------------
	static Color Accent()
	{
		if (IsTactical())
			return Ink(0.9);

		if (s_iTheme == 2)
			return ROOT_AER_UI.Rgb(0.5, 0.58, 0.26);

		return ROOT_AER_UI.Rgb(0.0, 0.47, 0.83);
	}

	//------------------------------------------------------------------------------------------------
	//! Text on Accent backgrounds.
	static Color OnAccent()
	{
		if (IsTactical())
			return Ground(0.5);

		return ROOT_AER_UI.Rgb(1, 1, 1);
	}

	//------------------------------------------------------------------------------------------------
	static Color Text()
	{
		if (IsTactical())
			return Ink(0.95);

		if (s_iTheme == 1)
			return ROOT_AER_UI.Rgb(0.1, 0.1, 0.1);

		return ROOT_AER_UI.Rgb(0.95, 0.95, 0.95);
	}

	//------------------------------------------------------------------------------------------------
	static Color Muted()
	{
		if (IsTactical())
			return Ink(0.55);

		if (s_iTheme == 1)
			return ROOT_AER_UI.Rgb(0.38, 0.38, 0.4);

		return ROOT_AER_UI.Rgb(0.65, 0.66, 0.68);
	}

	//------------------------------------------------------------------------------------------------
	//! Edit boxes and list backgrounds.
	static Color Field()
	{
		if (IsTactical())
			return Ground(0.6);

		return Pick(ROOT_AER_UI.Rgb(0.17, 0.17, 0.175), ROOT_AER_UI.Rgb(1, 1, 1), ROOT_AER_UI.Rgb(0.14, 0.16, 0.11));
	}

	//------------------------------------------------------------------------------------------------
	static Color Button()
	{
		if (IsTactical())
			return Ground(1.6);

		return Pick(ROOT_AER_UI.Rgb(0.22, 0.22, 0.23), ROOT_AER_UI.Rgb(0.99, 0.99, 0.99), ROOT_AER_UI.Rgb(0.3, 0.33, 0.22));
	}

	//------------------------------------------------------------------------------------------------
	static Color Border()
	{
		if (IsTactical())
			return Ink(1, 0.35);

		if (s_iTheme == 1)
			return ROOT_AER_UI.Rgb(0, 0, 0, 0.14);

		return ROOT_AER_UI.Rgb(1, 1, 1, 0.1);
	}

	//------------------------------------------------------------------------------------------------
	//! Overlay of transparent buttons under the cursor.
	static Color Hover()
	{
		if (IsTactical())
			return Ink(1, 0.14);

		if (s_iTheme == 1)
			return ROOT_AER_UI.Rgb(0, 0, 0, 0.06);

		return ROOT_AER_UI.Rgb(1, 1, 1, 0.08);
	}

	//------------------------------------------------------------------------------------------------
	//! Selected list row or icon.
	static Color Select()
	{
		if (IsTactical())
			return Ink(1, 0.25);

		if (s_iTheme == 1)
			return ROOT_AER_UI.Rgb(0.0, 0.47, 0.83, 0.16);

		if (s_iTheme == 2)
			return ROOT_AER_UI.Rgb(0.6, 0.7, 0.3, 0.3);

		return ROOT_AER_UI.Rgb(0.3, 0.6, 0.95, 0.28);
	}

	//------------------------------------------------------------------------------------------------
	static Color Danger()
	{
		return ROOT_AER_UI.Rgb(0.77, 0.17, 0.11);
	}

	//------------------------------------------------------------------------------------------------
	//! Positive state (connected, on).
	static Color Good()
	{
		if (IsTactical())
			return Ink(1);

		return ROOT_AER_UI.Rgb(0.2, 0.7, 0.35);
	}

	//------------------------------------------------------------------------------------------------
	//! Desktop background behind the wallpaper.
	static Color Desktop()
	{
		if (IsTactical())
			return Ground(0.4);

		return ROOT_AER_UI.Rgb(0.05, 0.08, 0.12);
	}
}
