//! Terminal color designs: header background, console background, header text, console text.
class ROOT_AE3_TerminalDesigns
{
	static const int COUNT = 20;

	//------------------------------------------------------------------------------------------------
	static string Name(int i)
	{
		array<string> names = {
			"armaOS", "C64", "Apple II", "Amber", "Midnight Blue", "Light Mode", "Retro Red", "Teal Terminal",
			"Neon Violet", "Dark Gray", "Ice Blue", "Sepia Vintage", "High Contrast Dark", "High Contrast Light",
			"Yellow on Black", "Cyan on Black", "Orange on Black", "Cream", "Terminal Green", "Soft Blue"
		};
		return names[Math.ClampInt(i, 0, COUNT - 1)];
	}

	//------------------------------------------------------------------------------------------------
	//! part: 0 header background, 1 console background, 2 header text, 3 console text.
	protected static ref array<float> s_aValues;

	//------------------------------------------------------------------------------------------------
	static Color Get(int design, int part)
	{
		if (!s_aValues)
			s_aValues = {
			0.125,0.125,0.125, 0.2,0.2,0.2, 1,1,1, 1,1,1,
			0.502,0.459,0.835, 0.243,0.192,0.635, 0.243,0.192,0.635, 0.502,0.459,0.835,
			0.2,1,0.2, 0.157,0.157,0.157, 0.157,0.157,0.157, 0.2,1,0.2,
			1,0.69,0, 0.157,0.157,0.157, 0.157,0.157,0.157, 1,0.69,0,
			0.1,0.1,0.15, 0.05,0.05,0.1, 0.6,0.8,1, 0.4,0.7,1,
			0.9,0.9,0.9, 0.75,0.75,0.75, 0.1,0.1,0.1, 0.2,0.2,0.2,
			0.95,0.25,0.25, 0.25,0.05,0.05, 1,0.9,0.9, 1,0.8,0.8,
			0.25,0.75,0.75, 0.1,0.3,0.3, 0.05,0.15,0.15, 0.25,0.75,0.75,
			0.6,0.1,0.4, 0.1,0.05,0.15, 1,0.7,0.9, 1,0.5,0.8,
			0.3,0.3,0.3, 0.05,0.05,0.05, 0.8,0.8,0.8, 0.8,0.8,0.8,
			0.85,0.95,1, 0.6,0.75,0.9, 0.05,0.1,0.25, 0.1,0.2,0.4,
			0.9,0.8,0.6, 0.5,0.35,0.2, 0.2,0.1,0.05, 0.2,0.1,0.05,
			0,0,0, 0,0,0, 1,1,1, 1,1,1,
			1,1,1, 1,1,1, 0,0,0, 0,0,0,
			0,0,0, 0.1,0.1,0.1, 1,1,0, 1,1,0,
			0,0,0, 0.1,0.1,0.1, 0,1,1, 0,1,1,
			0,0,0, 0.1,0.1,0.1, 1,0.5,0, 1,0.5,0,
			1,1,0.8, 0.2,0.2,0.2, 0,0,0, 0,0,0,
			0.05,0.05,0.05, 0.2,0.2,0.2, 0,1,0, 0,1,0,
			0,0,0.15, 0.05,0.05,0.1, 1,1,0.5, 1,1,0.5
		};

		array<float> v = s_aValues;
		int base = (Math.ClampInt(design, 0, COUNT - 1) * 4 + Math.ClampInt(part, 0, 3)) * 3;
		return new Color(v[base], v[base + 1], v[base + 2], 1);
	}
}
