//! 3x2 inventory slot (portable server).
modded enum ESlotSize
{
	SLOT_3x2
}

//------------------------------------------------------------------------------------------------
modded class SCR_ItemAttributeCollection
{
	//------------------------------------------------------------------------------------------------
	override int GetSlotSum()
	{
		if (m_Size == ESlotSize.SLOT_3x2)
			return 6;

		return super.GetSlotSum();
	}
}

//------------------------------------------------------------------------------------------------
modded class SCR_InventorySlotUI
{
	//------------------------------------------------------------------------------------------------
	override protected string SetSlotSize()
	{
		string layout = super.SetSlotSize();
		ROOT_AE3_ApplySlot3x2();
		return layout;
	}

	//------------------------------------------------------------------------------------------------
	protected void ROOT_AE3_ApplySlot3x2()
	{
		if (m_Attributes && m_Attributes.GetItemSize() == ESlotSize.SLOT_3x2)
		{
			m_iSizeX = 3;
			m_iSizeY = 2;
		}
	}
}

//------------------------------------------------------------------------------------------------
modded class SCR_ArsenalInventorySlotUI
{
	//------------------------------------------------------------------------------------------------
	override protected string SetSlotSize()
	{
		string layout = super.SetSlotSize();
		ROOT_AE3_ApplySlot3x2();
		return layout;
	}
}

//------------------------------------------------------------------------------------------------
modded class SCR_SupplyInventorySlotUI
{
	//------------------------------------------------------------------------------------------------
	override protected string SetSlotSize()
	{
		string layout = super.SetSlotSize();
		ROOT_AE3_ApplySlot3x2();
		return layout;
	}
}
