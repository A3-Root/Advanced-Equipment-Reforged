//! Power cable, in two steps (ROOT_AER_CableHand). Source side (m_bSource: generators, batteries, solar panels):
//! "Take power cable" (cable length ROOT_AER_PowerComponent.m_fCableLength). Device side: "Plug power cable into ..."
//! while a power cable is held (fails while the source is farther away than the cable), "Unplug power cable" when
//! plugged in.
class ROOT_AER_ConnectPowerAction : ROOT_AER_UserActionBase
{
	[Attribute("0", desc: "Source side: takes the power cable of this device (generator, battery, solar panel)")]
	protected bool m_bSource;

	//------------------------------------------------------------------------------------------------
	protected bool Holding(IEntity owner)
	{
		return ROOT_AER_CableHand.LocalKind() == ROOT_AER_CableHand.POWER && ROOT_AER_CableHand.LocalDevice() == owner;
	}

	//------------------------------------------------------------------------------------------------
	protected bool Finishing(IEntity owner)
	{
		return ROOT_AER_CableHand.LocalKind() == ROOT_AER_CableHand.POWER && ROOT_AER_CableHand.LocalDevice() != owner;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (ROOT_AER_Util.IsDestroyed(GetOwner()) || !IsPlaced(GetOwner()))
			return false;

		ROOT_AER_PowerComponent p = ROOT_AER_PowerComponent.Of(GetOwner());
		if (!p || p.IsMains())
			return false;

		if (m_bSource)
			return p.IsProvider() && !Finishing(GetOwner());

		if (p.IsProvider() && Holding(GetOwner()))
			return false;

		if (p.GetRole() != ROOT_AER_EPowerRole.CONSUMER && p.GetRole() != ROOT_AER_EPowerRole.BATTERY)
			return false;

		return Finishing(GetOwner()) || p.HasProvider();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		IEntity owner = GetOwner();
		if (m_bSource || Holding(owner))
			return true;

		ROOT_AER_PowerComponent p = ROOT_AER_PowerComponent.Of(owner);
		if (Finishing(owner))
		{
			IEntity child, parent;
			string reason = ROOT_AER_CableHand.Check(ROOT_AER_CableHand.POWER, ROOT_AER_CableHand.LocalDevice(), owner, ROOT_AER_CableHand.LocalLength(), child, parent);
			if (!reason.IsEmpty())
			{
				SetCannotPerformReason(reason);
				return false;
			}

			return true;
		}

		if (p.HasProvider())
			return true;

		SetCannotPerformReason("Take a power cable from a generator, battery or solar panel");
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!IsAuthority(pOwnerEntity) || !InReach(pOwnerEntity, pUserEntity))
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		ROOT_AER_HeldCable held = ROOT_AER_CableHand.GetHeld(playerId);
		ROOT_AER_PowerComponent p = ROOT_AER_PowerComponent.Of(pOwnerEntity);
		string reason;
		if (held && held.m_iKind == ROOT_AER_CableHand.POWER && held.m_Device == pOwnerEntity)
		{
			ROOT_AER_CableHand.Drop(playerId, "Power cable put back into " + ROOT_AER_CableHand.Name(pOwnerEntity) + ".");
			return;
		}

		if (m_bSource)
		{
			if (!ROOT_AER_CableHand.Take(pUserEntity, ROOT_AER_CableHand.POWER, pOwnerEntity, reason))
				Report(pUserEntity, reason);

			return;
		}

		if (held && held.m_iKind == ROOT_AER_CableHand.POWER)
		{
			if (!ROOT_AER_CableHand.Plug(pUserEntity, pOwnerEntity, reason))
				Report(pUserEntity, reason);

			return;
		}

		if (p.GetProvider())
			p.Disconnect();
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		IEntity owner = GetOwner();
		ROOT_AER_PowerComponent p = ROOT_AER_PowerComponent.Of(owner);
		if (Holding(owner))
		{
			outName = "Put power cable back";
			return true;
		}

		if (m_bSource)
		{
			outName = string.Format("Take power cable (%1 m)", p.GetCableLength());
			return true;
		}

		if (Finishing(owner))
		{
			float dist = vector.Distance(ROOT_AER_CableHand.LocalDevice().GetOrigin(), owner.GetOrigin());
			outName = string.Format("Plug power cable into %1 (%2 / %3 m)", ROOT_AER_CableHand.Name(owner), Math.Ceil(dist), ROOT_AER_CableHand.LocalLength());
			return true;
		}

		if (p.HasProvider())
			outName = "Unplug power cable";
		else
			outName = "Plug in power cable";

		return true;
	}
}
