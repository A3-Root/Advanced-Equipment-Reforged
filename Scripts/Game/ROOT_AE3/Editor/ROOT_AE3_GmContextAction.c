//! Game Master context action on Advanced Equipment devices. Opens the ROOT_AE3_GmDialog action panel for the
//! hovered and selected devices; with m_sAction set, runs that ROOT_AE3_GmActions key instead.
//! Runs on the Game Master's machine; the request is sent to the server, which checks the rights.
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class ROOT_AE3_GmContextAction : SCR_BaseContextAction
{
	[Attribute("", desc: "Action key (see ROOT_AE3_GmActions), empty = action panel")]
	protected string m_sAction;

	//------------------------------------------------------------------------------------------------
	protected void Collect(SCR_EditableEntityComponent hovered, set<SCR_EditableEntityComponent> selected, notnull array<IEntity> outEntities)
	{
		if (selected)
		{
			foreach (SCR_EditableEntityComponent e : selected)
			{
				if (e && ROOT_AE3_DeviceComponent.Of(e.GetOwner()) && !outEntities.Contains(e.GetOwner()))
					outEntities.Insert(e.GetOwner());
			}
		}

		if (hovered && ROOT_AE3_DeviceComponent.Of(hovered.GetOwner()) && !outEntities.Contains(hovered.GetOwner()))
			outEntities.Insert(hovered.GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		array<IEntity> ents = {};
		Collect(hoveredEntity, selectedEntities, ents);
		if (m_sAction.IsEmpty())
			return !ents.IsEmpty();

		return ROOT_AE3_GmActions.AppliesToAny(m_sAction, ents);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return CanBeShown(hoveredEntity, selectedEntities, cursorWorldPosition, flags);
	}

	//------------------------------------------------------------------------------------------------
	override void PerformOwner(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{
		array<IEntity> ents = {};
		Collect(hoveredEntity, selectedEntities, ents);
		if (ents.IsEmpty())
			return;

		if (m_sAction.IsEmpty())
		{
			ROOT_AE3_GmDialog.OpenPanel(ents);
			return;
		}

		array<RplId> ids = {};
		foreach (IEntity e : ents)
		{
			ids.Insert(ROOT_AE3_DeviceComponent.Of(e).GetRplId());
		}

		string title;
		array<string> labels = {};
		array<string> defaults = {};
		if (ROOT_AE3_GmActions.Fields(m_sAction, title, labels, defaults))
		{
			ROOT_AE3_GmDialog.Request(m_sAction, title, labels, defaults, ids);
			return;
		}

		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (pc)
			pc.ROOT_AE3_GmRequest(m_sAction, ids, "");
	}
}
