//! Inventory "Use" on Advanced Equipment Reforged items: computers in the own inventory open (wake or boot first),
//! EWO packs in the own inventory or the vicinity open the EWO panel, other devices in the own inventory (flash
//! drive, router, battery pack) open the name form.
class ROOT_AER_InventoryUse
{
	//------------------------------------------------------------------------------------------------
	static bool Handles(IEntity item)
	{
		if (!item)
			return false;

		if (ROOT_AER_ComputerComponent.Of(item) || ROOT_AER_DeviceComponent.Of(item))
			return true;

		return ROOT_AER_EwoSystem.IsEwoPack(item);
	}

	//------------------------------------------------------------------------------------------------
	//! Client.
	static void Use(IEntity item)
	{
		if (!item)
			return;

		if (ROOT_AER_ComputerComponent.Of(item))
		{
			ROOT_AER_Client.Get().OpenCarried(item);
			return;
		}

		if (ROOT_AER_EwoSystem.IsEwoPack(item))
		{
			ROOT_AER_EwoPanel.Open(item);
			return;
		}

		if (ROOT_AER_DeviceComponent.Of(item))
			ROOT_AER_GmDialog.OpenRename(item);
	}
}

//------------------------------------------------------------------------------------------------
modded class SCR_CharacterInventoryStorageComponent
{
	//------------------------------------------------------------------------------------------------
	override bool CanUseItem_Inventory(notnull IEntity item, ESlotFunction slotFunction = ESlotFunction.TYPE_GENERIC)
	{
		if (ROOT_AER_InventoryUse.Handles(item))
			return true;

		return super.CanUseItem_Inventory(item, slotFunction);
	}
}

//------------------------------------------------------------------------------------------------
modded class SCR_InventoryMenuUI
{
	//------------------------------------------------------------------------------------------------
	protected IEntity ROOT_AER_FocusedItem()
	{
		if (!m_pFocusedSlotUI || !m_pFocusedSlotUI.GetInventoryItemComponent())
			return null;

		IEntity item = m_pFocusedSlotUI.GetInventoryItemComponent().GetOwner();
		if (!item || !ROOT_AER_InventoryUse.Handles(item))
			return null;

		if (ItemBelongsToThePlayer(item))
			return item;

		if (ROOT_AER_EwoSystem.IsEwoPack(item) && !ROOT_AER_Util.IsCarried(item))
			return item;

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Use button for Advanced Equipment Reforged items with mouse, keyboard and gamepad.
	override void NavigationBarUpdate()
	{
		super.NavigationBarUpdate();
		if (m_pNavigationBar && !m_DraggedSlot && (!m_bIsUsingGamepad || !m_pSelectedSlotUI) && ROOT_AER_FocusedItem())
			m_pNavigationBar.SetButtonEnabled(BUTTON_USE, true);
	}

	//------------------------------------------------------------------------------------------------
	override protected void Action_UseItem()
	{
		IEntity item = ROOT_AER_FocusedItem();
		if (!item)
		{
			super.Action_UseItem();
			return;
		}

		Action_CloseInventory();
		GetGame().GetCallqueue().CallLater(ROOT_AER_InventoryUse.Use, 100, false, item);
	}
}
