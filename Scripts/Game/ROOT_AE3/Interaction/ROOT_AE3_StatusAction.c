//! Shows power, battery, fuel and network state of a device as a hint.
class ROOT_AE3_StatusAction : ROOT_AE3_UserActionBase
{
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBroadcastScript()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (pUserEntity != SCR_PlayerController.GetLocalControlledEntity())
			return;

		ROOT_AE3_DeviceComponent d = ROOT_AE3_DeviceComponent.Of(pOwnerEntity);
		string text;
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(pOwnerEntity);
		if (p)
		{
			array<string> states = {"off", "on", "standby", "crashed"};
			text += "State: " + states[Math.ClampInt(p.GetState(), 0, 3)];
			if (p.GetCapacityKWh() > 0)
				text += string.Format("\nBattery: %1%%", Math.Round(p.GetLevel() * 100));

			if (p.GetFuelCapacity() > 0)
				text += string.Format("\nFuel: %1 / %2 L (%3 L/h)", ROOT_AE3_Util.FormatFloat(p.GetFuel(), 1), Math.Round(p.GetFuelCapacity()), ROOT_AE3_Util.FormatFloat(p.GetBurnRate(), 1));

			if (p.IsProvider())
				text += string.Format("\nLoad: %1 / %2 W", Math.Round(p.GetLoadW()), Math.Round(p.GetOutputW()));
			else if (p.HasExternalPower())
				text += "\nExternal power: yes";
			else
				text += "\nExternal power: no";
		}

		ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(pOwnerEntity);
		if (n)
		{
			string ip = n.GetAddress();
			if (ip.IsEmpty())
				ip = "not connected";

			text += "\nNetwork: " + ip;
			if (n.IsRouter())
				text += "\nSSID: " + n.GetSsid();
		}

		string name = "Device";
		if (d)
			name = d.GetDeviceName();

		SCR_HintManagerComponent.ShowCustomHint(text, name, 6);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Check status";
		return true;
	}
}
