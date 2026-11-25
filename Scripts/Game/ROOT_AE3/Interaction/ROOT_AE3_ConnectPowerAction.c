//! Plugs the device into the nearest power source within reach, or unplugs it.
class ROOT_AE3_ConnectPowerAction : ROOT_AE3_UserActionBase
{
	[Attribute("6", desc: "Cable length in m")]
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
	protected ROOT_AE3_PowerComponent Nearest(IEntity owner)
	{
		ROOT_AE3_PowerComponent self = ROOT_AE3_PowerComponent.Of(owner);
		ROOT_AE3_PowerComponent best;
		float bestDist = m_fReach;
		foreach (ROOT_AE3_DeviceComponent d : ROOT_AE3_Manager.Get().GetDevices())
		{
			if (!d || d.GetOwner() == owner || ROOT_AE3_Util.IsCarried(d.GetOwner()))
				continue;

			ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(d.GetOwner());
			if (!p || !p.IsProvider() || p.GetProvider() == self)
				continue;

			float dist = vector.Distance(owner.GetOrigin(), d.GetOwner().GetOrigin());
			if (dist <= bestDist)
			{
				best = p;
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

		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(GetOwner());
		return p && IsPlaced(GetOwner()) && !p.IsMains() && (p.GetRole() == ROOT_AE3_EPowerRole.CONSUMER || p.GetRole() == ROOT_AE3_EPowerRole.BATTERY);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(GetOwner());
		if (p.HasProvider())
			return true;

		if (!HasNearest(GetOwner()))
		{
			SetCannotPerformReason("No power source in reach");
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!IsAuthority(pOwnerEntity) || !InReach(pOwnerEntity, pUserEntity))
			return;

		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(pOwnerEntity);
		if (p.GetProvider())
		{
			p.Disconnect();
			return;
		}

		ROOT_AE3_PowerComponent src = Nearest(pOwnerEntity);
		string reason = "No power source in reach.";
		if (!src || !p.ConnectTo(src, reason))
			Report(pUserEntity, reason);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(GetOwner());
		if (p.HasProvider())
			outName = "Unplug power";
		else
			outName = "Plug into power source";

		return true;
	}
}
