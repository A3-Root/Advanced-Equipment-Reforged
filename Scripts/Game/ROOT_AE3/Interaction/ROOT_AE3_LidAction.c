//! Opens or closes the laptop lid.
class ROOT_AE3_LidAction : ROOT_AE3_UserActionBase
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (ROOT_AE3_Util.IsDestroyed(GetOwner()))
			return false;

		return ROOT_AE3_LidComponent.Of(GetOwner()) && IsPlaced(GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!IsAuthority(pOwnerEntity) || !InReach(pOwnerEntity, pUserEntity))
			return;

		ROOT_AE3_LidComponent lid = ROOT_AE3_LidComponent.Of(pOwnerEntity);
		lid.SetOpen(!lid.IsOpen());
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (ROOT_AE3_LidComponent.Of(GetOwner()).IsOpen())
			outName = "Close lid";
		else
			outName = "Open lid";

		return true;
	}
}
