//! Inventory name and description of an Advanced Equipment Reforged device: custom name, hostname, battery, flash drives.
[BaseContainerProps()]
class ROOT_AER_DeviceInventoryUIInfo : SCR_InventoryUIInfo
{
	//------------------------------------------------------------------------------------------------
	override string GetInventoryItemName(InventoryItemComponent item)
	{
		string base = super.GetInventoryItemName(item);
		ROOT_AER_DeviceComponent dev = Device(item);
		if (!dev || dev.GetCustomName().IsEmpty())
			return base;

		return dev.GetCustomName() + " (" + WidgetManager.Translate(base) + ")";
	}

	//------------------------------------------------------------------------------------------------
	override string GetInventoryItemDescription(InventoryItemComponent item)
	{
		string text = super.GetInventoryItemDescription(item);
		ROOT_AER_DeviceComponent dev = Device(item);
		if (!dev)
			return text;

		array<string> lines = {};
		if (!text.IsEmpty())
			lines.Insert(WidgetManager.Translate(text));

		ROOT_AER_ComputerComponent c = ROOT_AER_ComputerComponent.Of(dev.GetOwner());
		if (c && !c.GetHostname().IsEmpty())
			lines.Insert(WidgetManager.Translate("#ROOT_AER-Inv_Hostname", c.GetHostname()));

		ROOT_AER_PowerComponent p = ROOT_AER_PowerComponent.Of(dev.GetOwner());
		if (p && p.GetCapacityKWh() > 0)
			lines.Insert(WidgetManager.Translate("#ROOT_AER-Inv_Battery", Math.Round(p.GetLevel() * 100).ToString()));

		if (p)
			lines.Insert(WidgetManager.Translate("#ROOT_AER-Inv_State", WidgetManager.Translate(StateName(p.GetState()))));

		if (c && c.GetUsbPortCount() > 0)
		{
			int mask = c.GetUsbMask();
			string usb;
			int count;
			for (int i = 0; i < c.GetUsbPortCount(); i++)
			{
				if (mask & (1 << i))
				{
					if (count > 0)
						usb += ", ";

					usb += "USB" + i.ToString();
					count++;
				}
			}

			if (count == 0)
				usb = WidgetManager.Translate("#ROOT_AER-Inv_None");

			lines.Insert(WidgetManager.Translate("#ROOT_AER-Inv_Usb", usb));
		}

		if (c)
			lines.Insert(WidgetManager.Translate("#ROOT_AER-Inv_HintComputer"));
		string result;
		foreach (int i, string line : lines)
		{
			if (i > 0)
				result += "\n";

			result += line;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected static string StateName(int state)
	{
		switch (state)
		{
			case ROOT_AER_EPowerState.ON: return "#ROOT_AER-Inv_On";
			case ROOT_AER_EPowerState.STANDBY: return "#ROOT_AER-Inv_Standby";
			case ROOT_AER_EPowerState.CRASHED: return "#ROOT_AER-Inv_Crashed";
		}

		return "#ROOT_AER-Inv_Off";
	}

	//------------------------------------------------------------------------------------------------
	protected static ROOT_AER_DeviceComponent Device(InventoryItemComponent item)
	{
		if (!item)
			return null;

		return ROOT_AER_DeviceComponent.Of(item.GetOwner());
	}
}
