//! Text field form for Game Master actions (string input, which editor attributes do not offer).
class ROOT_AE3_GmDialog : ChimeraMenuBase
{
	protected static string s_sKey;
	protected static string s_sTitle;
	protected static ref array<string> s_aLabels = {};
	protected static ref array<string> s_aDefaults = {};
	protected static ref array<RplId> s_aTargets = {};

	protected ref ROOT_AE3_GmDialogForm m_Form;

	//------------------------------------------------------------------------------------------------
	static void Request(string key, string title, notnull array<string> labels, notnull array<string> defaults, notnull array<RplId> targets)
	{
		s_sKey = key;
		s_sTitle = title;
		s_aLabels.Copy(labels);
		s_aDefaults.Copy(defaults);
		s_aTargets.Copy(targets);
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ROOT_AE3_GmDialog);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		m_Form = new ROOT_AE3_GmDialogForm(this, GetRootWidget(), s_sTitle, s_aLabels, s_aDefaults);
		GetGame().GetInputManager().AddActionListener("MenuBack", EActionTrigger.DOWN, OnBack);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnBack);
		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBack(float value = 0, EActionTrigger reason = 0)
	{
		Close();
	}

	//------------------------------------------------------------------------------------------------
	void Submit(notnull array<string> values)
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		foreach (int i, string v : values)
		{
			values[i] = ROOT_AE3_Codec.Clean(v);
		}

		if (pc)
			pc.ROOT_AE3_GmRequest(s_sKey, s_aTargets, ROOT_AE3_Codec.Join(values));

		Close();
	}
}
