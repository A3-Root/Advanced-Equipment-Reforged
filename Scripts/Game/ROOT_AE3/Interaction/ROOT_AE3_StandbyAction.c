//! Puts a running computer into standby; a standby computer wakes up.
class ROOT_AE3_StandbyAction : ROOT_AE3_UserActionBase
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (ROOT_AE3_Util.IsDestroyed(GetOwner()))
			return false;

		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(GetOwner());
		if (!p || !ROOT_AE3_ComputerComponent.Of(GetOwner()) || !IsPlaced(GetOwner()))
			return false;

		return (p.GetState() == ROOT_AE3_EPowerState.ON || p.GetState() == ROOT_AE3_EPowerState.STANDBY) && CanBePerformedScript(user);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(GetOwner());
		ROOT_AE3_LidComponent lid = ROOT_AE3_LidComponent.Of(GetOwner());
		if (p && p.GetState() == ROOT_AE3_EPowerState.STANDBY && lid && !lid.IsOpen())
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

		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(pOwnerEntity);
		string reason = "Open the lid first.";
		bool ok;
		ROOT_AE3_LidComponent lid = ROOT_AE3_LidComponent.Of(pOwnerEntity);
		if (p.GetState() == ROOT_AE3_EPowerState.STANDBY && lid && !lid.IsOpen())
			ok = false;
		else if (p.GetState() == ROOT_AE3_EPowerState.STANDBY)
			ok = p.TurnOn(reason);
		else
			ok = p.Standby(reason);

		if (!ok)
			Report(pUserEntity, reason);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(GetOwner());
		if (p.GetState() == ROOT_AE3_EPowerState.STANDBY)
			outName = "Wake up";
		else
			outName = "Standby";

		return true;
	}
}
