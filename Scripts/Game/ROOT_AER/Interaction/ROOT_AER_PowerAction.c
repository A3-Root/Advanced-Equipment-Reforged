//! Switches a device on or off (computers boot or shut down, generators start or stop, lamps light).
class ROOT_AER_PowerAction : ROOT_AER_UserActionBase
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		ROOT_AER_PowerComponent p = ROOT_AER_PowerComponent.Of(GetOwner());
		return p && IsPlaced(GetOwner()) && !ROOT_AER_Util.IsDestroyed(GetOwner()) && CanBePerformedScript(user);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		ROOT_AER_PowerComponent p = ROOT_AER_PowerComponent.Of(GetOwner());
		if (p.GetState() != ROOT_AER_EPowerState.OFF)
			return true;

		ROOT_AER_LidComponent lid = ROOT_AER_LidComponent.Of(GetOwner());
		if (lid && !lid.IsOpen())
		{
			SetCannotPerformReason("Lid closed");
			return false;
		}

		if (p.GetRole() == ROOT_AER_EPowerRole.GENERATOR && p.GetFuel() <= 0)
		{
			SetCannotPerformReason("No fuel");
			return false;
		}

		if (p.GetRole() == ROOT_AER_EPowerRole.CONSUMER && !p.HasEnergy())
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

		ROOT_AER_PowerComponent p = ROOT_AER_PowerComponent.Of(pOwnerEntity);
		ROOT_AER_ComputerComponent c = ROOT_AER_ComputerComponent.Of(pOwnerEntity);
		ROOT_AER_LidComponent lid = ROOT_AER_LidComponent.Of(pOwnerEntity);
		if (p.GetState() == ROOT_AER_EPowerState.OFF && lid && !lid.IsOpen())
		{
			Report(pUserEntity, "Open the lid first.");
			return;
		}

		if (p.GetState() == ROOT_AER_EPowerState.OFF)
		{
			string reason;
			if (!p.TurnOn(reason))
				Report(pUserEntity, reason);

			return;
		}

		if (c && p.GetState() == ROOT_AER_EPowerState.ON && c.IsRunning())
			c.Shutdown();
		else
			p.TurnOff();
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		ROOT_AER_PowerComponent p = ROOT_AER_PowerComponent.Of(GetOwner());
		bool generator = p.GetRole() == ROOT_AER_EPowerRole.GENERATOR;
		if (p.GetState() == ROOT_AER_EPowerState.OFF)
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
			else if (ROOT_AER_ComputerComponent.Of(GetOwner()))
				outName = "Shut down";
			else
				outName = "Power off";
		}

		return true;
	}
}
