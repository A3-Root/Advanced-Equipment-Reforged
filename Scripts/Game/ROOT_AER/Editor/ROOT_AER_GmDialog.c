//! Game Master dialog: action panel for the selected devices, then the field form of the chosen action.
//! The fields of an action, with the current values of the first selected device, come from the server
//! (SCR_PlayerController.ROOT_AER_GmQuery, answered by OnFields).
//! Also the player rename form (key RENAME_KEY, sent as ROOT_AER_EMsg.RENAME) and the network cable choice (key
//! CABLE_KEY, sent as ROOT_AER_EMsg.CABLE_TAKE) instead of a Game Master request.
class ROOT_AER_GmDialog : ChimeraMenuBase
{
	protected static string s_sKey;
	protected static string s_sTitle;
	protected static ref array<ref ROOT_AER_GmField> s_aFields = {};
	protected static ref array<RplId> s_aTargets = {};
	protected static ref array<string> s_aKeys = {};		//!< Panel: actions that apply to the selection
	protected static string s_sDevices;
	protected static bool s_bWaiting;						//!< Fields requested from the server
	protected static ROOT_AER_GmDialog s_Open;

	static const string RENAME_KEY = "_rename";
	static const string CABLE_KEY = "_cable";
	static const string USB_KEY = "_usb";

	protected ref ROOT_AER_GmDialogForm m_Form;
	protected ref ROOT_AER_GmPanel m_Panel;

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
			ROOT_AER_DeviceComponent d = ROOT_AER_DeviceComponent.Of(e);
			if (d)
				s_aTargets.Insert(d.GetRplId());
		}

		array<string> keys = {};
		array<string> labels = {};
		array<string> groups = {};
		ROOT_AER_GmActions.List(keys, labels, groups);
		foreach (string key : keys)
		{
			if (ROOT_AER_GmActions.AppliesToAny(key, devices))
				s_aKeys.Insert(key);
		}

		s_sDevices = devices.Count().ToString() + " devices";
		if (devices.Count() == 1)
		{
			ROOT_AER_DeviceComponent d = ROOT_AER_DeviceComponent.Of(devices[0]);
			if (d)
				s_sDevices = d.GetDeviceName();
		}

		if (!s_aTargets.IsEmpty() && !GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ROOT_AER_GmDialog))
			Print("[AER] Game Master dialog did not open", LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	//! Opens the form of one action directly (fields are requested from the server).
	static void OpenAction(string key, notnull array<RplId> targets)
	{
		s_sKey = key;
		s_sTitle = ROOT_AER_GmActions.Label(key);
		s_aFields.Clear();
		s_aTargets.Copy(targets);
		s_bWaiting = true;
		if (!GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ROOT_AER_GmDialog))
		{
			Print("[AER] Game Master dialog did not open", LogLevel.WARNING);
			return;
		}

		Query();
	}

	//------------------------------------------------------------------------------------------------
	//! Opens a form with fields built locally (player forms).
	protected static void OpenLocal(string key, string title, notnull array<ref ROOT_AER_GmField> fields, notnull array<RplId> targets)
	{
		s_sKey = key;
		s_sTitle = title;
		s_aFields.Clear();
		foreach (ROOT_AER_GmField f : fields)
		{
			s_aFields.Insert(f);
		}

		s_aTargets.Copy(targets);
		s_bWaiting = false;
		if (!GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ROOT_AER_GmDialog))
			Print("[AER] Game Master dialog did not open", LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	//! Player: name form for one device.
	static void OpenRename(IEntity device)
	{
		ROOT_AER_DeviceComponent d = ROOT_AER_DeviceComponent.Of(device);
		if (!d)
			return;

		array<ref ROOT_AER_GmField> fields = {ROOT_AER_GmField.Make(ROOT_AER_EGmField.TEXT, "Name (empty = default)", d.GetCustomName())};
		array<RplId> targets = {d.GetRplId()};
		OpenLocal(RENAME_KEY, "Rename " + d.GetDeviceName(), fields, targets);
	}

	//------------------------------------------------------------------------------------------------
	//! Player: which carried network cable to plug into device first (lengths in m).
	static void OpenCable(IEntity device, notnull array<float> lengths)
	{
		ROOT_AER_DeviceComponent d = ROOT_AER_DeviceComponent.Of(device);
		if (!d || lengths.IsEmpty())
			return;

		array<string> values = {};
		array<string> labels = {};
		foreach (float l : lengths)
		{
			values.Insert(l.ToString());
			labels.Insert(l.ToString() + " m network cable");
		}

		array<ref ROOT_AER_GmField> fields = {ROOT_AER_GmField.Choice("Cable", values, labels, values[values.Count() - 1])};
		array<RplId> targets = {d.GetRplId()};
		OpenLocal(CABLE_KEY, "Plug in network cable: " + d.GetDeviceName(), fields, targets);
	}

	//------------------------------------------------------------------------------------------------
	//! Player: which carried flash drive goes into the computer (sent as ROOT_AER_EMsg.USB_INSERT).
	static void OpenUsb(IEntity device, notnull array<IEntity> drives)
	{
		ROOT_AER_DeviceComponent d = ROOT_AER_DeviceComponent.Of(device);
		if (!d || drives.IsEmpty())
			return;

		array<string> values = {};
		array<string> labels = {};
		foreach (int i, IEntity item : drives)
		{
			values.Insert(ROOT_AER_UsbManager.DriveId(item).ToString());
			labels.Insert((i + 1).ToString() + ". " + ROOT_AER_UsbManager.DriveName(item));
		}

		array<ref ROOT_AER_GmField> fields = {ROOT_AER_GmField.Choice("Flash drive", values, labels, values[0])};
		array<RplId> targets = {d.GetRplId()};
		OpenLocal(USB_KEY, "Insert flash drive: " + d.GetDeviceName(), fields, targets);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Query()
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (pc)
			pc.ROOT_AER_GmQuery(s_sKey, s_aTargets);
	}

	//------------------------------------------------------------------------------------------------
	//! Server answer to ROOT_AER_GmQuery: title and packed fields; empty fields = no device applies.
	static void OnFields(string key, string title, string packed)
	{
		if (!s_bWaiting || key != s_sKey)
			return;

		s_bWaiting = false;
		s_sTitle = title;
		ROOT_AER_GmField.Unpack(packed, s_aFields);
		if (s_Open)
			s_Open.ShowForm();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		s_Open = this;
		if (s_sKey.IsEmpty())
			m_Panel = new ROOT_AER_GmPanel(this, GetRootWidget(), s_sDevices, s_aKeys);
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
		ROOT_AER_UI.Clear(root);
		if (m_Panel)
			m_Panel.ReleaseHandlers();

		m_Panel = null;
		if (m_Form)
			m_Form.ReleaseHandlers();

		m_Form = null;
		if (s_bWaiting)
		{
			float sw, sh;
			ROOT_AER_UI.ScreenSize(sw, sh);
			ROOT_AER_UI.Rect(root, 0, 0, sw, sh, ROOT_AER_UI.Rgb(0, 0, 0, 0.4));
			ROOT_AER_UI.Rect(root, (sw - 320) * 0.5, (sh - 60) * 0.5, 320, 60, ROOT_AER_UI.Rgb(0.12, 0.13, 0.15, 0.97));
			ROOT_AER_UI.Text(root, (sw - 300) * 0.5, (sh - 24) * 0.5, 300, 24, s_sTitle + "...", 16, null, false, false, 1);
			return;
		}

		if (s_aFields.IsEmpty())
		{
			Close();
			return;
		}

		m_Form = new ROOT_AER_GmDialogForm(this, root, s_sTitle, s_aFields);
	}

	//------------------------------------------------------------------------------------------------
	//! Panel choice: requests the form, or sends actions without fields at once.
	void Choose(string key)
	{
		s_sKey = key;
		s_aFields.Clear();
		if (!ROOT_AER_GmActions.HasFields(key))
		{
			Send("");
			Close();
			return;
		}

		s_sTitle = ROOT_AER_GmActions.Label(key);
		s_bWaiting = true;
		ShowForm();
		Query();
	}

	//------------------------------------------------------------------------------------------------
	protected void Send(string values)
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (pc)
			pc.ROOT_AER_GmRequest(s_sKey, s_aTargets, values);
	}

	//------------------------------------------------------------------------------------------------
	void Submit(notnull array<string> values)
	{
		if (s_sKey == CABLE_KEY)
		{
			if (!values.IsEmpty())
			{
				foreach (RplId id : s_aTargets)
				{
					ROOT_AER_Net.SendToServer(id, ROOT_AER_EMsg.CABLE_TAKE, values[0]);
				}
			}

			Close();
			return;
		}

		if (s_sKey == USB_KEY)
		{
			if (!values.IsEmpty())
			{
				foreach (RplId usbTarget : s_aTargets)
				{
					ROOT_AER_Net.SendToServer(usbTarget, ROOT_AER_EMsg.USB_INSERT, values[0]);
				}
			}

			Close();
			return;
		}

		if (s_sKey == RENAME_KEY)
		{
			string name;
			if (!values.IsEmpty())
				name = values[0];

			foreach (RplId id : s_aTargets)
			{
				ROOT_AER_Net.SendToServer(id, ROOT_AER_EMsg.RENAME, name);
			}

			Close();
			return;
		}

		foreach (int i, string v : values)
		{
			values[i] = ROOT_AER_Codec.Clean(v);
		}

		Send(ROOT_AER_Codec.Join(values));
		Close();
	}
}
