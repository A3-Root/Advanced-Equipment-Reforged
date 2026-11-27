//! Network cable of a device, in two steps (ROOT_AER_CableHand): "Plug in network cable" on the first device (needs a
//! carried network cable; with several lengths a form asks which one, ROOT_AER_GmDialog.OpenCable), then "Plug network cable into ..." on the other end (computer to router,
//! router to router). The second step fails while the devices are farther apart than the cable.
//! Without a held cable: "Unplug network cable" on a wired device gives the cable item back.
class ROOT_AER_ConnectNetworkAction : ROOT_AER_UserActionBase
{
	//------------------------------------------------------------------------------------------------
	protected bool Wired(IEntity owner)
	{
		ROOT_AER_ConnectionComponent link = ROOT_AER_ConnectionComponent.Of(owner);
		return link && link.GetNetPeer() != RplId.Invalid();
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsRouter(IEntity owner)
	{
		ROOT_AER_NetworkComponent n = ROOT_AER_NetworkComponent.Of(owner);
		return n && n.IsRouter();
	}

	//------------------------------------------------------------------------------------------------
	//! Client: the local player holds a network cable from another device.
	protected bool Finishing(IEntity owner)
	{
		return ROOT_AER_CableHand.LocalKind() == ROOT_AER_CableHand.NET && ROOT_AER_CableHand.LocalDevice() != owner;
	}

	//------------------------------------------------------------------------------------------------
	protected bool Holding(IEntity owner)
	{
		return ROOT_AER_CableHand.LocalKind() == ROOT_AER_CableHand.NET && ROOT_AER_CableHand.LocalDevice() == owner;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (ROOT_AER_Util.IsDestroyed(GetOwner()))
			return false;

		IEntity owner = GetOwner();
		if (!ROOT_AER_NetworkComponent.Of(owner) || !IsPlaced(owner))
			return false;

		if (Holding(owner) || Finishing(owner) || Wired(owner))
			return true;

		return CanBePerformedScript(user);
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
			string reason = ROOT_AER_CableHand.Check(ROOT_AER_CableHand.NET, ROOT_AER_CableHand.LocalDevice(), owner, ROOT_AER_CableHand.LocalLength(), child, parent);
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

		if (!CachedScan(true, ROOT_AER_CableComponent.FindCarried(user, 0) != null))
		{
			SetCannotPerformReason("No network cable");
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The step follows the held cable of the player on the server, not the client's view. Several carried cable
	//! lengths: the server waits for the form of the local player (ROOT_AER_EMsg.CABLE_TAKE).
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		bool choose = ROOT_AER_CableHand.LocalKind() != ROOT_AER_CableHand.NET && !Wired(pOwnerEntity);
		array<float> lengths = {};
		if (choose)
			ROOT_AER_CableComponent.CarriedLengths(pUserEntity, lengths);

		if (choose && lengths.Count() > 1 && pUserEntity == SCR_PlayerController.GetLocalControlledEntity())
			ROOT_AER_GmDialog.OpenCable(pOwnerEntity, lengths);

		if (!IsAuthority(pOwnerEntity) || !InReach(pOwnerEntity, pUserEntity))
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		ROOT_AER_HeldCable held = ROOT_AER_CableHand.GetHeld(playerId);
		string reason;
		if (held && held.m_iKind == ROOT_AER_CableHand.NET)
		{
			if (held.m_Device == pOwnerEntity)
			{
				ROOT_AER_CableHand.Drop(playerId, "Network cable unplugged from " + ROOT_AER_CableHand.Name(pOwnerEntity) + ".");
				return;
			}

			if (!ROOT_AER_CableHand.Plug(pUserEntity, pOwnerEntity, reason))
				Report(pUserEntity, reason);

			return;
		}

		if (Wired(pOwnerEntity))
		{
			Unplug(pOwnerEntity, pUserEntity);
			return;
		}

		array<float> carried = {};
		ROOT_AER_CableComponent.CarriedLengths(pUserEntity, carried);
		if (carried.Count() > 1)
			return;

		if (!ROOT_AER_CableHand.Take(pUserEntity, ROOT_AER_CableHand.NET, pOwnerEntity, reason))
			Report(pUserEntity, reason);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: removes the cable and gives the cable item back.
	protected void Unplug(IEntity owner, IEntity user)
	{
		ROOT_AER_NetworkComponent n = ROOT_AER_NetworkComponent.Of(owner);
		ROOT_AER_ConnectionComponent link = ROOT_AER_ConnectionComponent.Of(owner);
		if (!n || !link || !n.GetParent())
			return;

		IEntity router = n.GetParent().GetOwner();
		ResourceName item = link.TakeCable(router);
		ROOT_AER_API.DisconnectNetwork(owner);
		ROOT_AER_CableComponent.Give(item, owner, user);
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
			IEntity first = ROOT_AER_CableHand.LocalDevice();
			float dist = vector.Distance(first.GetOrigin(), owner.GetOrigin());
			outName = string.Format("Plug network cable into %1 (%2 / %3 m)", ROOT_AER_CableHand.Name(owner), Math.Ceil(dist), ROOT_AER_CableHand.LocalLength());
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
