//! Inventory "Use" on Advanced Equipment items: computers in the own inventory open (wake or boot first),
//! EWO packs in the own inventory or the vicinity open the EWO panel.
class ROOT_AE3_InventoryUse
{
	//------------------------------------------------------------------------------------------------
	static bool Handles(IEntity item)
	{
		if (!item)
			return false;

		if (ROOT_AE3_ComputerComponent.Of(item))
			return true;

		return ROOT_AE3_EwoSystem.IsEwoPack(item);
	}

	//------------------------------------------------------------------------------------------------
	//! Client.
	static void Use(IEntity item)
	{
		if (!item)
			return;

		if (ROOT_AE3_ComputerComponent.Of(item))
		{
			ROOT_AE3_Client.Get().OpenCarried(item);
			return;
		}

		if (ROOT_AE3_EwoSystem.IsEwoPack(item))
			ROOT_AE3_EwoPanel.Open(item);
	}
}

//------------------------------------------------------------------------------------------------
modded class SCR_CharacterInventoryStorageComponent
{
	//------------------------------------------------------------------------------------------------
	override bool CanUseItem_Inventory(notnull IEntity item, ESlotFunction slotFunction = ESlotFunction.TYPE_GENERIC)
	{
		if (ROOT_AE3_InventoryUse.Handles(item))
			return true;

		return super.CanUseItem_Inventory(item, slotFunction);
	}
}

//------------------------------------------------------------------------------------------------
modded class SCR_InventoryMenuUI
{
	//------------------------------------------------------------------------------------------------
	protected IEntity ROOT_AE3_FocusedItem()
	{
		if (!m_pFocusedSlotUI || !m_pFocusedSlotUI.GetInventoryItemComponent())
			return null;

		IEntity item = m_pFocusedSlotUI.GetInventoryItemComponent().GetOwner();
		if (!item || !ROOT_AE3_InventoryUse.Handles(item))
			return null;

		if (ItemBelongsToThePlayer(item))
			return item;

		if (ROOT_AE3_EwoSystem.IsEwoPack(item) && !ROOT_AE3_Util.IsCarried(item))
			return item;

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Use button for Advanced Equipment items with mouse, keyboard and gamepad.
	override void NavigationBarUpdate()
	{
		super.NavigationBarUpdate();
		if (m_pNavigationBar && !m_DraggedSlot && (!m_bIsUsingGamepad || !m_pSelectedSlotUI) && ROOT_AE3_FocusedItem())
			m_pNavigationBar.SetButtonEnabled(BUTTON_USE, true);
	}

	//------------------------------------------------------------------------------------------------
	override protected void Action_UseItem()
	{
		IEntity item = ROOT_AE3_FocusedItem();
		if (!item)
		{
			super.Action_UseItem();
			return;
		}

		Action_CloseInventory();
		GetGame().GetCallqueue().CallLater(ROOT_AE3_InventoryUse.Use, 100, false, item);
	}
}
