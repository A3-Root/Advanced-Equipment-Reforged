//! Opens the name form of a placed device for the local player; the server applies the name (ROOT_AER_EMsg.RENAME).
class ROOT_AER_RenameAction : ROOT_AER_UserActionBase
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return ROOT_AER_DeviceComponent.Of(GetOwner()) && IsPlaced(GetOwner()) && !ROOT_AER_Util.IsDestroyed(GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (pUserEntity != SCR_PlayerController.GetLocalControlledEntity())
			return;

		ROOT_AER_GmDialog.OpenRename(pOwnerEntity);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Rename";
		return true;
	}
}
