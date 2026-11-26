//! Cable link to the nearest router within reach (routers cascade), or removes the link.
class ROOT_AE3_ConnectNetworkAction : ROOT_AE3_UserActionBase
{
	[Attribute("10", desc: "Cable length in m")]
	protected float m_fReach;

	//------------------------------------------------------------------------------------------------
	protected float m_fLastScan = -10;
	protected bool m_bLastFound;

	//------------------------------------------------------------------------------------------------
	//! Cached once per second for the action menu.
	protected bool HasNearest(IEntity owner)
	{
		float now = GetGame().GetWorld().GetWorldTime() / 1000.0;
		if (now - m_fLastScan > 1)
		{
			m_fLastScan = now;
			m_bLastFound = Nearest(owner) != null;
		}

		return m_bLastFound;
	}

	//------------------------------------------------------------------------------------------------
	protected ROOT_AE3_NetworkComponent Nearest(IEntity owner)
	{
		ROOT_AE3_NetworkComponent best;
		float bestDist = m_fReach;
		foreach (ROOT_AE3_DeviceComponent d : ROOT_AE3_Manager.Get().GetDevices())
		{
			if (!d || d.IsHidden() || d.GetOwner() == owner || ROOT_AE3_Util.IsCarried(d.GetOwner()))
				continue;

			ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(d.GetOwner());
			if (!n || !n.IsRouter())
				continue;

			float dist = vector.Distance(owner.GetOrigin(), d.GetOwner().GetOrigin());
			if (dist <= bestDist)
			{
				best = n;
				bestDist = dist;
			}
		}

		return best;
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
		ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(GetOwner());
		if (n.IsConnected())
			return true;

		if (!HasNearest(GetOwner()))
		{
			SetCannotPerformReason("No router in reach");
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!IsAuthority(pOwnerEntity) || !InReach(pOwnerEntity, pUserEntity))
			return;

		ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(pOwnerEntity);
		if (n.IsConnected())
		{
			ROOT_AE3_API.DisconnectNetwork(pOwnerEntity);
			return;
		}

		ROOT_AE3_NetworkComponent r = Nearest(pOwnerEntity);
		string reason = "No router in reach.";
		if (r && !r.AcceptsConnections())
		{
			Report(pUserEntity, "The router does not accept new connections.");
			return;
		}

		if (!r || !ROOT_AE3_ConnectionComponent.ConnectNetwork(pOwnerEntity, r.GetOwner(), reason))
			Report(pUserEntity, reason);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(GetOwner());
		if (n.IsConnected())
			outName = "Unplug network cable";
		else
			outName = "Plug network cable into router";

		return true;
	}
}
