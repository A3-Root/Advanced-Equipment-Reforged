[ComponentEditorProps(category: "Advanced Equipment", description: "Shows the computer screen on the model ($rendertarget material) to nearby players")]
class ROOT_AE3_ScreenComponentClass : ScriptComponentClass
{
}

//! Client: renders the replicated screen mirror of ROOT_AE3_ComputerComponent into a render target texture
//! while the local player is within the UI-on-texture range. Not created on dedicated servers.
class ROOT_AE3_ScreenComponent : ScriptComponent
{
	[Attribute("{84CFC98A22EDCE1B}UI/layouts/ROOT_AE3/ROOT_AE3_Screen.layout", UIWidgets.ResourceNamePicker, desc: "Render target layout", params: "layout")]
	protected ResourceName m_sLayout;

	[Attribute("10", desc: "Frames per second of the screen texture", params: "1 60 1")]
	protected int m_iFps;

	protected Widget m_wRoot;
	protected RTTextureWidget m_wRT;
	protected FrameWidget m_wScreen;
	protected bool m_bDirty = true;
	protected int m_iRenderedScreen = -1;
	protected int m_iRenderedDesign = -1;
	protected TextWidget m_wText;
	protected static int s_iActive;
	static const int MAX_SCREENS = 4;	//!< Render-target screens one client draws at the same time

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (!GetGame().InPlayMode() || System.IsConsoleApp())
			return;

		GetGame().GetCallqueue().CallLater(CheckRange, 1000, true);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		GetGame().GetCallqueue().Remove(CheckRange);
		if (m_wRoot)
		{
			m_wRoot.RemoveFromHierarchy();
			m_wRoot = null;
			s_iActive = Math.Max(0, s_iActive - 1);
		}

		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_ScreenComponent Of(IEntity ent)
	{
		if (!ent)
			return null;

		return ROOT_AE3_ScreenComponent.Cast(ent.FindComponent(ROOT_AE3_ScreenComponent));
	}

	//------------------------------------------------------------------------------------------------
	void Rebind()
	{
		if (m_wRT)
			m_wRT.SetRenderTarget(GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	void OnComputerChanged()
	{
		m_bDirty = true;
		if (m_wRoot)
			Render();
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckRange()
	{
		ROOT_AE3_Client.Get().RequestSettings();
		ROOT_AE3_Settings s = ROOT_AE3_SettingsManager.Get();
		ROOT_AE3_ComputerComponent c = ROOT_AE3_ComputerComponent.Of(GetOwner());
		IEntity me = SCR_PlayerController.GetLocalControlledEntity();
		ROOT_AE3_LidComponent lid = ROOT_AE3_LidComponent.Of(GetOwner());
		bool want = s.m_bUiOnTexture && c && me && !ROOT_AE3_Util.IsCarried(GetOwner()) && c.GetScreen() != ROOT_AE3_EScreen.OFF && c.GetScreen() != ROOT_AE3_EScreen.STANDBY && (!lid || lid.IsOpen());
		if (want)
			want = vector.Distance(me.GetOrigin(), GetOwner().GetOrigin()) <= Math.Max(s.m_fUiPlayerRange, 1) + 1;

		if (want && !m_wRoot)
		{
			if (s_iActive >= MAX_SCREENS)
				return;

			Show();
		}
		else if (!want && m_wRoot)
		{
			Hide();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Show()
	{
		m_wRoot = GetGame().GetWorkspace().CreateWidgets(m_sLayout);
		if (!m_wRoot)
			return;

		s_iActive++;
		m_wRT = RTTextureWidget.Cast(m_wRoot.FindAnyWidget("RTTexture0"));
		m_wScreen = FrameWidget.Cast(m_wRoot.FindAnyWidget("Screen"));
		if (!m_wRT || !m_wScreen)
		{
			Hide();
			return;
		}

		m_wRoot.SetEnabled(false);
		m_wRT.SetRenderTarget(GetOwner());
		m_wRT.SetMaxFPS(m_iFps);
		Render();
	}

	//------------------------------------------------------------------------------------------------
	protected void Hide()
	{
		if (!m_wRoot)
			return;

		if (m_wRT)
			m_wRT.RemoveRenderTarget(GetOwner());

		m_wRoot.RemoveFromHierarchy();
		m_wRoot = null;
		m_wRT = null;
		m_wScreen = null;
		m_wText = null;
		m_iRenderedScreen = -1;
		s_iActive = Math.Max(0, s_iActive - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void Render()
	{
		ROOT_AE3_ComputerComponent c = ROOT_AE3_ComputerComponent.Of(GetOwner());
		if (!c || !m_wScreen)
			return;

		m_bDirty = false;
		int design = c.GetDesign();
		int screenMode = c.GetScreen();
		if (m_wText && screenMode == m_iRenderedScreen && design == m_iRenderedDesign)
		{
			UpdateText(c);
			return;
		}

		m_iRenderedScreen = screenMode;
		m_iRenderedDesign = design;
		m_wText = null;
		ROOT_AE3_UI.Clear(m_wScreen);
		float w = 1024;
		float h = 640;
		switch (c.GetScreen())
		{
			case ROOT_AE3_EScreen.BOOT:
			{
				ROOT_AE3_UI.Rect(m_wScreen, 0, 0, w, h, Color.FromInt(Color.BLACK));
				ROOT_AE3_Settings s = ROOT_AE3_SettingsManager.Get();
				ROOT_AE3_UI.Text(m_wScreen, 40, 40, w - 80, 400, s.m_sTerminalBiosVersion + "\n" + s.m_sTerminalCopyright + "\n\n" + s.m_sTerminalBootMessage, 28, ROOT_AE3_UI.Rgb(0.8, 0.8, 0.8), true, true);
				return;
			}

			case ROOT_AE3_EScreen.STANDBY:
			{
				ROOT_AE3_UI.Rect(m_wScreen, 0, 0, w, h, ROOT_AE3_UI.Rgb(0.02, 0.02, 0.03));
				ROOT_AE3_UI.Text(m_wScreen, w - 180, h - 50, 160, 30, "standby", 22, ROOT_AE3_UI.Rgb(0.3, 0.5, 0.3), true);
				return;
			}

			case ROOT_AE3_EScreen.CRASHED:
			{
				ROOT_AE3_UI.Rect(m_wScreen, 0, 0, w, h, ROOT_AE3_UI.Rgb(0.0, 0.2, 0.67));
				ROOT_AE3_UI.Text(m_wScreen, 60, 80, w - 120, 480, ":(\n\nYour computer ran into a problem and needs to restart.\n\nSTOP CODE: ARMAOS_KERNEL_PANIC\n\nPower the device off and on again.", 30, Color.FromInt(Color.WHITE), false, true);
				return;
			}

			case ROOT_AE3_EScreen.SHUTDOWN:
			{
				ROOT_AE3_UI.Rect(m_wScreen, 0, 0, w, h, Color.FromInt(Color.BLACK));
				ROOT_AE3_UI.Text(m_wScreen, 40, h * 0.45, w - 80, 40, "Shutting down...", 30, ROOT_AE3_UI.Rgb(0.8, 0.8, 0.8), true);
				return;
			}

			case ROOT_AE3_EScreen.DESKTOP:
			{
				ROOT_AE3_UI.Rect(m_wScreen, 0, 0, w, h, ROOT_AE3_UI.Rgb(0.11, 0.14, 0.2));
				ROOT_AE3_UI.Rect(m_wScreen, 0, h - 40, w, 40, ROOT_AE3_UI.Rgb(0.08, 0.08, 0.1));
				ROOT_AE3_UI.Rect(m_wScreen, 6, h - 34, 110, 28, ROOT_AE3_UI.Rgb(0.91, 0.33, 0.13));
				m_wText = ROOT_AE3_UI.Text(m_wScreen, 40, 40, w - 80, h - 120, "", 30, Color.FromInt(Color.WHITE), false, true);
				UpdateText(c);
				return;
			}
		}

		ROOT_AE3_UI.Rect(m_wScreen, 0, 0, w, h, ROOT_AE3_TerminalDesigns.Get(design, 1));
		m_wText = ROOT_AE3_UI.Text(m_wScreen, 16, 12, w - 32, h - 24, "", 26, ROOT_AE3_TerminalDesigns.Get(design, 3), true, false);
		UpdateText(c);
	}

	//------------------------------------------------------------------------------------------------
	//! Text of the terminal (last 22 lines plus the typed input) or of the desktop summary.
	protected void UpdateText(ROOT_AE3_ComputerComponent c)
	{
		if (!m_wText)
			return;

		if (c.GetScreen() == ROOT_AE3_EScreen.DESKTOP)
		{
			m_wText.SetText(c.GetScreenText());
			return;
		}

		string text = c.GetScreenText();
		array<string> lines = {};
		array<string> raw = {};
		text.Split("\n", raw, false);
		if (!raw.IsEmpty())
			raw[raw.Count() - 1] = raw[raw.Count() - 1] + c.GetScreenInput() + "_";

		foreach (string r : raw)
		{
			while (r.Length() > 64)
			{
				lines.Insert(r.Substring(0, 64));
				r = r.Substring(64, r.Length() - 64);
			}

			lines.Insert(r);
		}

		int start = Math.Max(0, lines.Count() - 22);
		string shown;
		for (int i = start; i < lines.Count(); i++)
		{
			shown += lines[i] + "\n";
		}

		m_wText.SetText(shown);
	}
}
