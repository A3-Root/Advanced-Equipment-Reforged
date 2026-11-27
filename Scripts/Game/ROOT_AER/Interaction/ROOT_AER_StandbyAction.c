//! Puts a running computer into standby; a standby computer wakes up.
class ROOT_AER_StandbyAction : ROOT_AER_UserActionBase
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (ROOT_AER_Util.IsDestroyed(GetOwner()))
			return false;

		ROOT_AER_PowerComponent p = ROOT_AER_PowerComponent.Of(GetOwner());
		if (!p || !ROOT_AER_ComputerComponent.Of(GetOwner()) || !IsPlaced(GetOwner()))
			return false;

		return (p.GetState() == ROOT_AER_EPowerState.ON || p.GetState() == ROOT_AER_EPowerState.STANDBY) && CanBePerformedScript(user);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		ROOT_AER_PowerComponent p = ROOT_AER_PowerComponent.Of(GetOwner());
		ROOT_AER_LidComponent lid = ROOT_AER_LidComponent.Of(GetOwner());
		if (p && p.GetState() == ROOT_AER_EPowerState.STANDBY && lid && !lid.IsOpen())
		{
			SetCannotPerformReason("Lid closed");
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
		string reason = "Open the lid first.";
		bool ok;
		ROOT_AER_LidComponent lid = ROOT_AER_LidComponent.Of(pOwnerEntity);
		if (p.GetState() == ROOT_AER_EPowerState.STANDBY && lid && !lid.IsOpen())
			ok = false;
		else if (p.GetState() == ROOT_AER_EPowerState.STANDBY)
			ok = p.TurnOn(reason);
		else
			ok = p.Standby(reason);

		if (!ok)
			Report(pUserEntity, reason);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		ROOT_AER_PowerComponent p = ROOT_AER_PowerComponent.Of(GetOwner());
		if (p.GetState() == ROOT_AER_EPowerState.STANDBY)
			outName = "Wake up";
		else
			outName = "Standby";

		return true;
	}
}
