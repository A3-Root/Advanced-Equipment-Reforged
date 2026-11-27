//! Game Master attributes (Configs/Editor/AttributeLists/Edit.conf). m_sKey selects the value:
//! device keys (attributes window of a device) or, with m_bGlobal, ROOT_AER_Settings keys (game settings
//! window and attributes window of the AER System module).
//! Read and written on the server.
class ROOT_AER_EditorAttr
{
	//------------------------------------------------------------------------------------------------
	static IEntity Device(Managed item)
	{
		SCR_EditableEntityComponent editable = SCR_EditableEntityComponent.Cast(item);
		if (!editable)
			return null;

		IEntity owner = editable.GetOwner();
		if (!owner || !ROOT_AER_DeviceComponent.Of(owner))
			return null;

		return owner;
	}

	//------------------------------------------------------------------------------------------------
	//! Game mode (game settings window) or the active AER System module (its attributes window).
	static bool IsSettingsHolder(Managed item)
	{
		if (BaseGameMode.Cast(item))
			return true;

		SCR_EditableEntityComponent editable = SCR_EditableEntityComponent.Cast(item);
		return editable && ROOT_AER_SystemModuleComponent.IsActiveModule(editable.GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	//! False when the key does not apply to the item.
	static bool Read(Managed item, string key, bool global, out float value)
	{
		if (global)
		{
			if (!IsSettingsHolder(item))
				return false;

			return ROOT_AER_SettingsManager.Get().GetNumber(key, value);
		}

		IEntity e = Device(item);
		if (!e)
			return false;

		ROOT_AER_PowerComponent p = ROOT_AER_PowerComponent.Of(e);
		ROOT_AER_NetworkComponent n = ROOT_AER_NetworkComponent.Of(e);
		ROOT_AER_ComputerComponent c = ROOT_AER_ComputerComponent.Of(e);
		switch (key)
		{
			case "power":
			{
				if (!p)
					return false;

				value = ROOT_AER_Util.BoolNum(p.GetState() != ROOT_AER_EPowerState.OFF);
				return true;
			}

			case "battery":
			{
				if (!p || p.GetCapacityKWh() <= 0)
					return false;

				value = Math.Round(p.GetLevel() * 100);
				return true;
			}

			case "fuel":
			{
				if (!p || p.GetFuelCapacity() <= 0)
					return false;

				value = Math.Round(p.GetFuel() / p.GetFuelCapacity() * 100);
				return true;
			}

			case "fuel_use":
			{
				if (!p || p.GetFuelCapacity() <= 0)
					return false;

				value = p.GetBurnRate();
				return true;
			}

			case "range":
			{
				if (!n || !n.IsRouter())
					return false;

				value = n.GetBaseRange();
				return true;
			}

			case "network_speed":
			{
				if (!n)
					return false;

				value = n.GetDeviceSpeed();
				if (n.IsRouter())
					value = n.GetRouterSpeed();

				return true;
			}

			case "accept":
			{
				if (!n || !n.IsRouter())
					return false;

				value = ROOT_AER_Util.BoolNum(n.AcceptsConnections());
				return true;
			}

			case "external_ssh":
			{
				if (!n || !n.IsRouter())
					return false;

				value = ROOT_AER_Util.BoolNum(n.AllowsExternalSsh());
				return true;
			}

			case "ssh":
			{
				if (!n || n.IsRouter() || !c)
					return false;

				value = ROOT_AER_Util.BoolNum(n.IsSshEnabled());
				return true;
			}

			case "root_login":
			{
				if (!c)
					return false;

				value = c.GetRootLoginPolicy();
				return true;
			}

			case "interface":
			{
				if (!c)
					return false;

				value = c.GetInterfaceModeRaw();
				return true;
			}

			case "ui_style":
			{
				if (!c)
					return false;

				value = c.GetUiStyleRaw();
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	static void Write(Managed item, string key, bool global, float value)
	{
		if (global)
		{
			if (IsSettingsHolder(item) && ROOT_AER_SettingsManager.Get().SetNumber(key, value))
				ROOT_AER_SettingsManager.Broadcast();

			return;
		}

		IEntity e = Device(item);
		if (!e)
			return;

		ROOT_AER_PowerComponent p = ROOT_AER_PowerComponent.Of(e);
		ROOT_AER_NetworkComponent n = ROOT_AER_NetworkComponent.Of(e);
		ROOT_AER_ComputerComponent c = ROOT_AER_ComputerComponent.Of(e);
		switch (key)
		{
			case "power":
			{
				bool on = value != 0;
				if (p && on != (p.GetState() != ROOT_AER_EPowerState.OFF))
				{
					int state = ROOT_AER_EPowerState.OFF;
					if (on)
						state = ROOT_AER_EPowerState.ON;

					ROOT_AER_API.SetPowerState(e, state);
				}

				break;
			}

			case "battery": ROOT_AER_API.SetBatteryLevel(e, value / 100); break;
			case "fuel_use": ROOT_AER_API.SetFuelUse(e, value); break;
			case "fuel":
			{
				if (p)
					ROOT_AER_API.SetFuel(e, p.GetFuelCapacity() * value / 100);

				break;
			}

			case "range":
			{
				if (n)
					n.ConfigureRouter("", n.GetPassword(), value, -1, n.AcceptsConnections(), n.AllowsExternalSsh(), -1);

				break;
			}

			case "network_speed": ROOT_AER_API.SetTransferSpeed(e, value); break;
			case "accept":
			{
				if (n)
					n.ConfigureRouter("", n.GetPassword(), 0, -1, value != 0, n.AllowsExternalSsh(), -1);

				break;
			}

			case "external_ssh":
			{
				if (n)
					n.ConfigureRouter("", n.GetPassword(), 0, -1, n.AcceptsConnections(), value != 0, -1);

				break;
			}

			case "ssh":
			{
				if (n)
					n.SetSshEnabled(value != 0);

				break;
			}

			case "root_login": ROOT_AER_API.SetRootLogin(e, value); break;
			case "interface": ROOT_AER_API.SetInterfaceMode(e, value); break;
			case "ui_style": ROOT_AER_API.SetUiStyle(e, value); break;
		}
	}
}

//------------------------------------------------------------------------------------------------
//! Checkbox (AttributePrefab_Checkbox.layout).
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class ROOT_AER_CheckboxEditorAttribute : SCR_BaseEditorAttribute
{
	[Attribute(desc: "Value key (see ROOT_AER_EditorAttr)")]
	protected string m_sKey;

	[Attribute("0", desc: "Game setting (ROOT_AER_Settings) instead of a device value")]
	protected bool m_bGlobal;

	//------------------------------------------------------------------------------------------------
	override bool BoolAllowDuplicate()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		float v;
		if (!ROOT_AER_EditorAttr.Read(item, m_sKey, m_bGlobal, v))
			return null;

		return SCR_BaseEditorAttributeVar.CreateBool(v != 0);
	}

	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;

		float v;
		if (var.GetBool())
			v = 1;

		ROOT_AER_EditorAttr.Write(item, m_sKey, m_bGlobal, v);
	}
}

//------------------------------------------------------------------------------------------------
//! Slider (AttributePrefab_Slider.layout), range from m_baseValues.
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class ROOT_AER_SliderEditorAttribute : SCR_BaseValueListEditorAttribute
{
	[Attribute(desc: "Value key (see ROOT_AER_EditorAttr)")]
	protected string m_sKey;

	[Attribute("0", desc: "Game setting (ROOT_AER_Settings) instead of a device value")]
	protected bool m_bGlobal;

	//------------------------------------------------------------------------------------------------
	override bool BoolAllowDuplicate()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		float v;
		if (!ROOT_AER_EditorAttr.Read(item, m_sKey, m_bGlobal, v))
			return null;

		return SCR_BaseEditorAttributeVar.CreateFloat(v);
	}

	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (var)
			ROOT_AER_EditorAttr.Write(item, m_sKey, m_bGlobal, var.GetFloat());
	}
}

//------------------------------------------------------------------------------------------------
//! Choice (AttributePrefab_Spinbox.layout); m_aValues hold the values.
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class ROOT_AER_ChoiceEditorAttribute : SCR_BaseFloatValueHolderEditorAttribute
{
	[Attribute(desc: "Value key (see ROOT_AER_EditorAttr)")]
	protected string m_sKey;

	[Attribute("0", desc: "Game setting (ROOT_AER_Settings) instead of a device value")]
	protected bool m_bGlobal;

	//------------------------------------------------------------------------------------------------
	override bool BoolAllowDuplicate()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		float v;
		if (!ROOT_AER_EditorAttr.Read(item, m_sKey, m_bGlobal, v))
			return null;

		return SCR_BaseEditorAttributeVar.CreateInt(Math.Max(0, ConvertValueToIndex(v)));
	}

	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		float v;
		if (var && ConvertIndexToValue(var.GetInt(), v))
			ROOT_AER_EditorAttr.Write(item, m_sKey, m_bGlobal, v);
	}
}
