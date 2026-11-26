//! Opens the name form of a placed device for the local player; the server applies the name (ROOT_AE3_EMsg.RENAME).
class ROOT_AE3_RenameAction : ROOT_AE3_UserActionBase
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return ROOT_AE3_DeviceComponent.Of(GetOwner()) && IsPlaced(GetOwner()) && !ROOT_AE3_Util.IsDestroyed(GetOwner());
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

		ROOT_AE3_GmDialog.OpenRename(pOwnerEntity);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Rename";
		return true;
	}
}
