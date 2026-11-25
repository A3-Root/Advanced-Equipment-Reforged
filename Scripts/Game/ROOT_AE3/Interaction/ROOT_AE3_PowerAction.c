//! Switches a device on or off (computers boot or shut down, generators start or stop, lamps light).
class ROOT_AE3_PowerAction : ROOT_AE3_UserActionBase
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(GetOwner());
		return p && IsPlaced(GetOwner()) && !ROOT_AE3_Util.IsDestroyed(GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(GetOwner());
		if (p.GetState() != ROOT_AE3_EPowerState.OFF)
			return true;

		ROOT_AE3_LidComponent lid = ROOT_AE3_LidComponent.Of(GetOwner());
		if (lid && !lid.IsOpen())
		{
			SetCannotPerformReason("Lid closed");
			return false;
		}

		if (p.GetRole() == ROOT_AE3_EPowerRole.GENERATOR && p.GetFuel() <= 0)
		{
			SetCannotPerformReason("No fuel");
			return false;
		}

		if (p.GetRole() == ROOT_AE3_EPowerRole.CONSUMER && !p.HasEnergy())
		{
			SetCannotPerformReason("No power");
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!IsAuthority(pOwnerEntity) || !InReach(pOwnerEntity, pUserEntity))
			return;

		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(pOwnerEntity);
		ROOT_AE3_ComputerComponent c = ROOT_AE3_ComputerComponent.Of(pOwnerEntity);
		ROOT_AE3_LidComponent lid = ROOT_AE3_LidComponent.Of(pOwnerEntity);
		if (p.GetState() == ROOT_AE3_EPowerState.OFF && lid && !lid.IsOpen())
		{
			Report(pUserEntity, "Open the lid first.");
			return;
		}

		if (p.GetState() == ROOT_AE3_EPowerState.OFF)
		{
			string reason;
			if (!p.TurnOn(reason))
				Report(pUserEntity, reason);

			return;
		}

		if (c && p.GetState() == ROOT_AE3_EPowerState.ON && c.IsRunning())
			c.Shutdown();
		else
			p.TurnOff();
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(GetOwner());
		bool generator = p.GetRole() == ROOT_AE3_EPowerRole.GENERATOR;
		if (p.GetState() == ROOT_AE3_EPowerState.OFF)
		{
			if (generator)
				outName = "Start generator";
			else
				outName = "Power on";
		}
		else
		{
			if (generator)
				outName = "Stop generator";
			else if (ROOT_AE3_ComputerComponent.Of(GetOwner()))
				outName = "Shut down";
			else
				outName = "Power off";
		}

		return true;
	}
}
