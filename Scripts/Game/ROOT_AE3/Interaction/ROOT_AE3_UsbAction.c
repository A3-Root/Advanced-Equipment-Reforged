//! Plugs a flash drive from the inventory into a free port, or ejects the drive of the last used port.
class ROOT_AE3_UsbAction : ROOT_AE3_UserActionBase
{
	[Attribute("0", desc: "Eject instead of insert")]
	protected bool m_bEject;

	//------------------------------------------------------------------------------------------------
	protected int UsedPort(ROOT_AE3_ComputerComponent c)
	{
		for (int i = c.GetUsbPortCount() - 1; i >= 0; i--)
		{
			if (c.GetUsbDrive(i))
				return i;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (ROOT_AE3_Util.IsDestroyed(GetOwner()))
			return false;

		ROOT_AE3_ComputerComponent c = ROOT_AE3_ComputerComponent.Of(GetOwner());
		if (!c || c.GetUsbPortCount() == 0 || !IsPlaced(GetOwner()))
			return false;

		if (m_bEject)
			return c.HasUsbDrive();

		return FreePort(c) && CanBePerformedScript(user);
	}

	//------------------------------------------------------------------------------------------------
	protected bool FreePort(ROOT_AE3_ComputerComponent c)
	{
		for (int i = 0; i < c.GetUsbPortCount(); i++)
		{
			if (!c.GetUsbDrive(i))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (m_bEject)
			return true;

		bool due = ScanDue(user);
		if (!CachedScan(due, due && ROOT_AE3_UsbManager.FindDrive(user) != null))
		{
			SetCannotPerformReason("No flash drive");
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Several carried drives: the local player chooses one (ROOT_AE3_GmDialog.OpenUsb), the server waits for
	//! ROOT_AE3_EMsg.USB_INSERT.
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_bEject && pUserEntity == SCR_PlayerController.GetLocalControlledEntity())
		{
			array<IEntity> carried = {};
			ROOT_AE3_UsbManager.Drives(pUserEntity, carried);
			if (carried.Count() > 1)
				ROOT_AE3_GmDialog.OpenUsb(pOwnerEntity, carried);
		}

		if (!IsAuthority(pOwnerEntity) || !InReach(pOwnerEntity, pUserEntity))
			return;

		ROOT_AE3_ComputerComponent c = ROOT_AE3_ComputerComponent.Of(pOwnerEntity);
		string reason;
		if (m_bEject)
		{
			int port = UsedPort(c);
			if (port >= 0 && !ROOT_AE3_UsbManager.Eject(c, port, pUserEntity, reason))
				Report(pUserEntity, reason);

			return;
		}

		array<IEntity> drives = {};
		ROOT_AE3_UsbManager.Drives(pUserEntity, drives);
		if (drives.Count() > 1)
			return;

		if (!ROOT_AE3_UsbManager.Insert(c, pUserEntity, reason))
			Report(pUserEntity, reason);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (m_bEject)
			outName = "Eject flash drive";
		else
			outName = "Insert flash drive";

		return true;
	}
}
