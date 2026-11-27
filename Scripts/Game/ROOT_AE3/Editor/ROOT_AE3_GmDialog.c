//! Game Master dialog: action panel for the selected devices, then the field form of the chosen action.
//! The fields of an action, with the current values of the first selected device, come from the server
//! (SCR_PlayerController.ROOT_AE3_GmQuery, answered by OnFields).
//! Also the player rename form (key RENAME_KEY, sent as ROOT_AE3_EMsg.RENAME instead of a Game Master request).
class ROOT_AE3_GmDialog : ChimeraMenuBase
{
	protected static string s_sKey;
	protected static string s_sTitle;
	protected static ref array<ref ROOT_AE3_GmField> s_aFields = {};
	protected static ref array<RplId> s_aTargets = {};
	protected static ref array<string> s_aKeys = {};		//!< Panel: actions that apply to the selection
	protected static string s_sDevices;
	protected static bool s_bWaiting;						//!< Fields requested from the server
	protected static ROOT_AE3_GmDialog s_Open;

	static const string RENAME_KEY = "_rename";

	protected ref ROOT_AE3_GmDialogForm m_Form;
	protected ref ROOT_AE3_GmPanel m_Panel;

	//------------------------------------------------------------------------------------------------
	//! Opens the action panel for the given devices.
	static void OpenPanel(notnull array<IEntity> devices)
	{
		s_aTargets.Clear();
		s_aKeys.Clear();
		s_sKey = "";
		s_bWaiting = false;
		foreach (IEntity e : devices)
		{
			ROOT_AE3_DeviceComponent d = ROOT_AE3_DeviceComponent.Of(e);
			if (d)
				s_aTargets.Insert(d.GetRplId());
		}

		array<string> keys = {};
		array<string> labels = {};
		array<string> groups = {};
		ROOT_AE3_GmActions.List(keys, labels, groups);
		foreach (string key : keys)
		{
			if (ROOT_AE3_GmActions.AppliesToAny(key, devices))
				s_aKeys.Insert(key);
		}

		s_sDevices = devices.Count().ToString() + " devices";
		if (devices.Count() == 1)
		{
			ROOT_AE3_DeviceComponent d = ROOT_AE3_DeviceComponent.Of(devices[0]);
			if (d)
				s_sDevices = d.GetDeviceName();
		}

		if (!s_aTargets.IsEmpty() && !GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ROOT_AE3_GmDialog))
			Print("[AE3] Game Master dialog did not open", LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	//! Opens the form of one action directly (fields are requested from the server).
	static void OpenAction(string key, notnull array<RplId> targets)
	{
		s_sKey = key;
		s_sTitle = ROOT_AE3_GmActions.Label(key);
		s_aFields.Clear();
		s_aTargets.Copy(targets);
		s_bWaiting = true;
		if (!GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ROOT_AE3_GmDialog))
		{
			Print("[AE3] Game Master dialog did not open", LogLevel.WARNING);
			return;
		}

		Query();
	}

	//------------------------------------------------------------------------------------------------
	//! Opens a form with fields built locally (player forms).
	protected static void OpenLocal(string key, string title, notnull array<ref ROOT_AE3_GmField> fields, notnull array<RplId> targets)
	{
		s_sKey = key;
		s_sTitle = title;
		s_aFields.Clear();
		foreach (ROOT_AE3_GmField f : fields)
		{
			s_aFields.Insert(f);
		}

		s_aTargets.Copy(targets);
		s_bWaiting = false;
		if (!GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ROOT_AE3_GmDialog))
			Print("[AE3] Game Master dialog did not open", LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	//! Player: name form for one device.
	static void OpenRename(IEntity device)
	{
		ROOT_AE3_DeviceComponent d = ROOT_AE3_DeviceComponent.Of(device);
		if (!d)
			return;

		array<ref ROOT_AE3_GmField> fields = {ROOT_AE3_GmField.Make(ROOT_AE3_EGmField.TEXT, "Name (empty = default)", d.GetCustomName())};
		array<RplId> targets = {d.GetRplId()};
		OpenLocal(RENAME_KEY, "Rename " + d.GetDeviceName(), fields, targets);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Query()
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (pc)
			pc.ROOT_AE3_GmQuery(s_sKey, s_aTargets);
	}

	//------------------------------------------------------------------------------------------------
	//! Server answer to ROOT_AE3_GmQuery: title and packed fields; empty fields = no device applies.
	static void OnFields(string key, string title, string packed)
	{
		if (!s_bWaiting || key != s_sKey)
			return;

		s_bWaiting = false;
		s_sTitle = title;
		ROOT_AE3_GmField.Unpack(packed, s_aFields);
		if (s_Open)
			s_Open.ShowForm();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		s_Open = this;
		if (s_sKey.IsEmpty())
			m_Panel = new ROOT_AE3_GmPanel(this, GetRootWidget(), s_sDevices, s_aKeys);
		else
			ShowForm();

		GetGame().GetInputManager().AddActionListener("MenuBack", EActionTrigger.DOWN, OnBack);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		if (s_Open == this)
			s_Open = null;

		s_bWaiting = false;
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnBack);
		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBack(float value = 0, EActionTrigger reason = 0)
	{
		Close();
	}

	//------------------------------------------------------------------------------------------------
	//! Form of s_sKey, or a waiting note while the server builds the fields.
	protected void ShowForm()
	{
		Widget root = GetRootWidget();
		ROOT_AE3_UI.Clear(root);
		if (m_Panel)
			m_Panel.ReleaseHandlers();

		m_Panel = null;
		if (m_Form)
			m_Form.ReleaseHandlers();

		m_Form = null;
		if (s_bWaiting)
		{
			float sw, sh;
			ROOT_AE3_UI.ScreenSize(sw, sh);
			ROOT_AE3_UI.Rect(root, 0, 0, sw, sh, ROOT_AE3_UI.Rgb(0, 0, 0, 0.4));
			ROOT_AE3_UI.Rect(root, (sw - 320) * 0.5, (sh - 60) * 0.5, 320, 60, ROOT_AE3_UI.Rgb(0.12, 0.13, 0.15, 0.97));
			ROOT_AE3_UI.Text(root, (sw - 300) * 0.5, (sh - 24) * 0.5, 300, 24, s_sTitle + "...", 16, null, false, false, 1);
			return;
		}

		if (s_aFields.IsEmpty())
		{
			Close();
			return;
		}

		m_Form = new ROOT_AE3_GmDialogForm(this, root, s_sTitle, s_aFields);
	}

	//------------------------------------------------------------------------------------------------
	//! Panel choice: requests the form, or sends actions without fields at once.
	void Choose(string key)
	{
		s_sKey = key;
		s_aFields.Clear();
		if (!ROOT_AE3_GmActions.HasFields(key))
		{
			Send("");
			Close();
			return;
		}

		s_sTitle = ROOT_AE3_GmActions.Label(key);
		s_bWaiting = true;
		ShowForm();
		Query();
	}

	//------------------------------------------------------------------------------------------------
	protected void Send(string values)
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (pc)
			pc.ROOT_AE3_GmRequest(s_sKey, s_aTargets, values);
	}

	//------------------------------------------------------------------------------------------------
	void Submit(notnull array<string> values)
	{
		if (s_sKey == RENAME_KEY)
		{
			string name;
			if (!values.IsEmpty())
				name = values[0];

			foreach (RplId id : s_aTargets)
			{
				ROOT_AE3_Net.SendToServer(id, ROOT_AE3_EMsg.RENAME, name);
			}

			Close();
			return;
		}

		foreach (int i, string v : values)
		{
			values[i] = ROOT_AE3_Codec.Clean(v);
		}

		Send(ROOT_AE3_Codec.Join(values));
		Close();
	}
}
