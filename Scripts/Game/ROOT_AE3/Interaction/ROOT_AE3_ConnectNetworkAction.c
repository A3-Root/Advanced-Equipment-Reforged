//! Network cable of a device, in two steps (ROOT_AE3_CableHand): "Plug in network cable" on the first device (needs a
//! carried network cable; the longest is used), then "Plug network cable into ..." on the other end (computer to router,
//! router to router). The second step fails while the devices are farther apart than the cable.
//! Without a held cable: "Unplug network cable" on a wired device gives the cable item back.
class ROOT_AE3_ConnectNetworkAction : ROOT_AE3_UserActionBase
{
	//------------------------------------------------------------------------------------------------
	protected bool Wired(IEntity owner)
	{
		ROOT_AE3_ConnectionComponent link = ROOT_AE3_ConnectionComponent.Of(owner);
		return link && link.GetNetPeer() != RplId.Invalid();
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsRouter(IEntity owner)
	{
		ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(owner);
		return n && n.IsRouter();
	}

	//------------------------------------------------------------------------------------------------
	//! Client: the local player holds a network cable from another device.
	protected bool Finishing(IEntity owner)
	{
		return ROOT_AE3_CableHand.LocalKind() == ROOT_AE3_CableHand.NET && ROOT_AE3_CableHand.LocalDevice() != owner;
	}

	//------------------------------------------------------------------------------------------------
	protected bool Holding(IEntity owner)
	{
		return ROOT_AE3_CableHand.LocalKind() == ROOT_AE3_CableHand.NET && ROOT_AE3_CableHand.LocalDevice() == owner;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (ROOT_AE3_Util.IsDestroyed(GetOwner()))
			return false;

		return ROOT_AE3_NetworkComponent.Of(GetOwner()) && IsPlaced(GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		IEntity owner = GetOwner();
		if (Holding(owner))
			return true;

		if (Finishing(owner))
		{
			IEntity child, parent;
			string reason = ROOT_AE3_CableHand.Check(ROOT_AE3_CableHand.NET, ROOT_AE3_CableHand.LocalDevice(), owner, ROOT_AE3_CableHand.LocalLength(), child, parent);
			if (!reason.IsEmpty())
			{
				SetCannotPerformReason(reason);
				return false;
			}

			return true;
		}

		if (Wired(owner))
			return true;

		if (!ScanDue(user))
		{
			if (!m_bScanResult)
				SetCannotPerformReason("No network cable");

			return m_bScanResult;
		}

		if (!CachedScan(true, ROOT_AE3_CableComponent.FindCarried(user, 0) != null))
		{
			SetCannotPerformReason("No network cable");
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority. The step follows the held cable of the player on the server, not the client's view.
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!IsAuthority(pOwnerEntity) || !InReach(pOwnerEntity, pUserEntity))
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		ROOT_AE3_HeldCable held = ROOT_AE3_CableHand.GetHeld(playerId);
		string reason;
		if (held && held.m_iKind == ROOT_AE3_CableHand.NET)
		{
			if (held.m_Device == pOwnerEntity)
			{
				ROOT_AE3_CableHand.Drop(playerId, "Network cable unplugged from " + ROOT_AE3_CableHand.Name(pOwnerEntity) + ".");
				return;
			}

			if (!ROOT_AE3_CableHand.Plug(pUserEntity, pOwnerEntity, reason))
				Report(pUserEntity, reason);

			return;
		}

		if (Wired(pOwnerEntity))
		{
			Unplug(pOwnerEntity, pUserEntity);
			return;
		}

		if (!ROOT_AE3_CableHand.Take(pUserEntity, ROOT_AE3_CableHand.NET, pOwnerEntity, reason))
			Report(pUserEntity, reason);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: removes the cable and gives the cable item back.
	protected void Unplug(IEntity owner, IEntity user)
	{
		ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(owner);
		ROOT_AE3_ConnectionComponent link = ROOT_AE3_ConnectionComponent.Of(owner);
		if (!n || !link || !n.GetParent())
			return;

		IEntity router = n.GetParent().GetOwner();
		ResourceName item = link.TakeCable(router);
		ROOT_AE3_API.DisconnectNetwork(owner);
		ROOT_AE3_CableComponent.Give(item, owner, user);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		IEntity owner = GetOwner();
		if (Holding(owner))
		{
			outName = "Unplug held network cable";
			return true;
		}

		if (Finishing(owner))
		{
			IEntity first = ROOT_AE3_CableHand.LocalDevice();
			float dist = vector.Distance(first.GetOrigin(), owner.GetOrigin());
			outName = string.Format("Plug network cable into %1 (%2 / %3 m)", ROOT_AE3_CableHand.Name(owner), Math.Ceil(dist), ROOT_AE3_CableHand.LocalLength());
			return true;
		}

		if (Wired(owner))
			outName = "Unplug network cable";
		else if (IsRouter(owner))
			outName = "Plug in network cable (router port)";
		else
			outName = "Plug in network cable";

		return true;
	}
}
