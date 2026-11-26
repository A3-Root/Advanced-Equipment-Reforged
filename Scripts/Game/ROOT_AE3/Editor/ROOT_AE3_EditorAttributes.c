//! Game Master attributes (Configs/Editor/AttributeLists/Edit.conf). m_sKey selects the value:
//! device keys (attributes window of a device) or, with m_bGlobal, ROOT_AE3_Settings keys (game settings
//! window and attributes window of the AE3 System module).
//! Read and written on the server.
class ROOT_AE3_EditorAttr
{
	//------------------------------------------------------------------------------------------------
	static IEntity Device(Managed item)
	{
		SCR_EditableEntityComponent editable = SCR_EditableEntityComponent.Cast(item);
		if (!editable)
			return null;

		IEntity owner = editable.GetOwner();
		if (!owner || !ROOT_AE3_DeviceComponent.Of(owner))
			return null;

		return owner;
	}

	//------------------------------------------------------------------------------------------------
	//! Game mode (game settings window) or the active AE3 System module (its attributes window).
	static bool IsSettingsHolder(Managed item)
	{
		if (BaseGameMode.Cast(item))
			return true;

		SCR_EditableEntityComponent editable = SCR_EditableEntityComponent.Cast(item);
		return editable && ROOT_AE3_SystemModuleComponent.IsActiveModule(editable.GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	//! False when the key does not apply to the item.
	static bool Read(Managed item, string key, bool global, out float value)
	{
		if (global)
		{
			if (!IsSettingsHolder(item))
				return false;

			return ROOT_AE3_SettingsManager.Get().GetNumber(key, value);
		}

		IEntity e = Device(item);
		if (!e)
			return false;

		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(e);
		ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(e);
		ROOT_AE3_ComputerComponent c = ROOT_AE3_ComputerComponent.Of(e);
		switch (key)
		{
			case "power":
			{
				if (!p)
					return false;

				value = ROOT_AE3_Util.BoolNum(p.GetState() != ROOT_AE3_EPowerState.OFF);
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
				if (!p || p.GetRole() != ROOT_AE3_EPowerRole.GENERATOR || p.GetFuelCapacity() <= 0)
					return false;

				value = Math.Round(p.GetFuel() / p.GetFuelCapacity() * 100);
				return true;
			}

			case "range":
			{
				if (!n || !n.IsRouter())
					return false;

				value = n.GetBaseRange();
				return true;
			}

			case "accept":
			{
				if (!n || !n.IsRouter())
					return false;

				value = ROOT_AE3_Util.BoolNum(n.AcceptsConnections());
				return true;
			}

			case "external_ssh":
			{
				if (!n || !n.IsRouter())
					return false;

				value = ROOT_AE3_Util.BoolNum(n.AllowsExternalSsh());
				return true;
			}

			case "ssh":
			{
				if (!n || n.IsRouter() || !c)
					return false;

				value = ROOT_AE3_Util.BoolNum(n.IsSshEnabled());
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
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	static void Write(Managed item, string key, bool global, float value)
	{
		if (global)
		{
			if (IsSettingsHolder(item) && ROOT_AE3_SettingsManager.Get().SetNumber(key, value))
				ROOT_AE3_SettingsManager.Broadcast();

			return;
		}

		IEntity e = Device(item);
		if (!e)
			return;

		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(e);
		ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(e);
		ROOT_AE3_ComputerComponent c = ROOT_AE3_ComputerComponent.Of(e);
		switch (key)
		{
			case "power":
			{
				bool on = value != 0;
				if (p && on != (p.GetState() != ROOT_AE3_EPowerState.OFF))
				{
					int state = ROOT_AE3_EPowerState.OFF;
					if (on)
						state = ROOT_AE3_EPowerState.ON;

					ROOT_AE3_API.SetPowerState(e, state);
				}

				break;
			}

			case "battery": ROOT_AE3_API.SetBatteryLevel(e, value / 100); break;
			case "fuel":
			{
				if (p)
					ROOT_AE3_API.SetFuel(e, p.GetFuelCapacity() * value / 100);

				break;
			}

			case "range":
			{
				if (n)
					n.ConfigureRouter("", n.GetPassword(), value, -1, n.AcceptsConnections(), n.AllowsExternalSsh(), -1);

				break;
			}

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

			case "root_login": ROOT_AE3_API.SetRootLogin(e, value); break;
			case "interface": ROOT_AE3_API.SetInterfaceMode(e, value); break;
		}
	}
}

//------------------------------------------------------------------------------------------------
//! Checkbox (AttributePrefab_Checkbox.layout).
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class ROOT_AE3_CheckboxEditorAttribute : SCR_BaseEditorAttribute
{
	[Attribute(desc: "Value key (see ROOT_AE3_EditorAttr)")]
	protected string m_sKey;

	[Attribute("0", desc: "Game setting (ROOT_AE3_Settings) instead of a device value")]
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
		if (!ROOT_AE3_EditorAttr.Read(item, m_sKey, m_bGlobal, v))
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

		ROOT_AE3_EditorAttr.Write(item, m_sKey, m_bGlobal, v);
	}
}

//------------------------------------------------------------------------------------------------
//! Slider (AttributePrefab_Slider.layout), range from m_baseValues.
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class ROOT_AE3_SliderEditorAttribute : SCR_BaseValueListEditorAttribute
{
	[Attribute(desc: "Value key (see ROOT_AE3_EditorAttr)")]
	protected string m_sKey;

	[Attribute("0", desc: "Game setting (ROOT_AE3_Settings) instead of a device value")]
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
		if (!ROOT_AE3_EditorAttr.Read(item, m_sKey, m_bGlobal, v))
			return null;

		return SCR_BaseEditorAttributeVar.CreateFloat(v);
	}

	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (var)
			ROOT_AE3_EditorAttr.Write(item, m_sKey, m_bGlobal, var.GetFloat());
	}
}

//------------------------------------------------------------------------------------------------
//! Choice (AttributePrefab_Spinbox.layout); m_aValues hold the values.
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class ROOT_AE3_ChoiceEditorAttribute : SCR_BaseFloatValueHolderEditorAttribute
{
	[Attribute(desc: "Value key (see ROOT_AE3_EditorAttr)")]
	protected string m_sKey;

	[Attribute("0", desc: "Game setting (ROOT_AE3_Settings) instead of a device value")]
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
		if (!ROOT_AE3_EditorAttr.Read(item, m_sKey, m_bGlobal, v))
			return null;

		return SCR_BaseEditorAttributeVar.CreateInt(Math.Max(0, ConvertValueToIndex(v)));
	}

	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		float v;
		if (var && ConvertIndexToValue(var.GetInt(), v))
			ROOT_AE3_EditorAttr.Write(item, m_sKey, m_bGlobal, v);
	}
}
