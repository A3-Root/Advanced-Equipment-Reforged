//! Game Master dialog: action panel for the selected devices, then the field form of the chosen action.
//! Also the player rename form (key RENAME_KEY, sent as ROOT_AE3_EMsg.RENAME instead of a Game Master request)
//! and the inventory form of a carried computer (key DEVICE_KEY: Open, or Rename as RENAME_KEY).
class ROOT_AE3_GmDialog : ChimeraMenuBase
{
	protected static string s_sKey;
	protected static string s_sTitle;
	protected static ref array<string> s_aLabels = {};
	protected static ref array<string> s_aDefaults = {};
	protected static ref array<RplId> s_aTargets = {};
	protected static ref array<string> s_aKeys = {};		//!< Panel: actions that apply to the selection
	protected static string s_sDevices;

	protected static IEntity s_Device;	//!< DEVICE_KEY: the carried computer

	static const string RENAME_KEY = "_rename";
	static const string DEVICE_KEY = "_device";

	protected ref ROOT_AE3_GmDialogForm m_Form;
	protected ref ROOT_AE3_GmPanel m_Panel;

	//------------------------------------------------------------------------------------------------
	//! Opens the action panel for the given devices.
	static void OpenPanel(notnull array<IEntity> devices)
	{
		s_aTargets.Clear();
		s_aKeys.Clear();
		s_sKey = "";
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
	//! Opens the field form of one action directly.
	static void Request(string key, string title, notnull array<string> labels, notnull array<string> defaults, notnull array<RplId> targets)
	{
		s_sKey = key;
		s_sTitle = title;
		s_aLabels.Copy(labels);
		s_aDefaults.Copy(defaults);
		s_aTargets.Copy(targets);
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

		array<string> labels = {"Name (empty = default)"};
		array<string> defaults = {d.GetCustomName()};
		array<RplId> targets = {d.GetRplId()};
		Request(RENAME_KEY, "Rename " + d.GetDeviceName(), labels, defaults, targets);
	}

	//------------------------------------------------------------------------------------------------
	//! Player: form of a computer in the own inventory: Open, or Rename with the name field.
	static void OpenDevice(IEntity device)
	{
		ROOT_AE3_DeviceComponent d = ROOT_AE3_DeviceComponent.Of(device);
		if (!d)
			return;

		s_Device = device;
		array<string> labels = {"Name (empty = default)"};
		array<string> defaults = {d.GetCustomName()};
		array<RplId> targets = {d.GetRplId()};
		Request(DEVICE_KEY, d.GetDeviceName(), labels, defaults, targets);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		if (s_sKey.IsEmpty())
			m_Panel = new ROOT_AE3_GmPanel(this, GetRootWidget(), s_sDevices, s_aKeys);
		else if (s_sKey == DEVICE_KEY)
			m_Form = new ROOT_AE3_GmDialogForm(this, GetRootWidget(), s_sTitle, s_aLabels, s_aDefaults, "Rename", "Open");
		else
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
	//! Panel choice: shows the form, or sends actions without fields at once.
	void Choose(string key)
	{
		s_sKey = key;
		s_aLabels.Clear();
		s_aDefaults.Clear();
		if (!ROOT_AE3_GmActions.Fields(key, s_sTitle, s_aLabels, s_aDefaults))
		{
			Send("");
			Close();
			return;
		}

		Widget root = GetRootWidget();
		ROOT_AE3_UI.Clear(root);
		if (m_Panel)
			m_Panel.ReleaseHandlers();

		m_Panel = null;
		m_Form = new ROOT_AE3_GmDialogForm(this, root, s_sTitle, s_aLabels, s_aDefaults);
	}

	//------------------------------------------------------------------------------------------------
	protected void Send(string values)
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (pc)
			pc.ROOT_AE3_GmRequest(s_sKey, s_aTargets, values);
	}

	//------------------------------------------------------------------------------------------------
	//! Third form button. DEVICE_KEY: opens the carried computer.
	void Extra()
	{
		IEntity device = s_Device;
		Close();
		if (s_sKey == DEVICE_KEY && device)
			ROOT_AE3_Client.Get().OpenCarried(device);
	}

	//------------------------------------------------------------------------------------------------
	void Submit(notnull array<string> values)
	{
		if (s_sKey == RENAME_KEY || s_sKey == DEVICE_KEY)
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
