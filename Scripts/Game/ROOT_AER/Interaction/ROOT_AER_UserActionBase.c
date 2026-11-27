//! Base of Advanced Equipment Reforged user actions: state-changing actions run their effect on the authority only.
class ROOT_AER_UserActionBase : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	protected bool IsAuthority(IEntity owner)
	{
		RplComponent rpl = RplComponent.Cast(owner.FindComponent(RplComponent));
		return !rpl || !rpl.IsProxy();
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: the acting character is alive, within reach and the device is placed.
	protected bool InReach(IEntity owner, IEntity user)
	{
		if (!owner || !user || !IsPlaced(owner) || ROOT_AER_Util.IsDestroyed(owner))
			return false;

		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (character && character.GetCharacterController() && character.GetCharacterController().IsDead())
			return false;

		return vector.Distance(owner.GetOrigin(), user.GetOrigin()) <= 5;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: shows a failure reason to the acting player.
	protected void Report(IEntity user, string reason)
	{
		if (reason.IsEmpty() || !user)
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		if (playerId <= 0)
			return;

		array<string> f = {"Advanced Equipment Reforged", reason};
		ROOT_AER_Net.SendToPlayer(playerId, ROOT_AER_EMsg.NOTIFY, ROOT_AER_Codec.Join(f));
	}

	protected float m_fScanTime = -10;
	protected bool m_bScanResult;
	protected IEntity m_ScanUser;

	//------------------------------------------------------------------------------------------------
	//! Result of an inventory scan, kept for 1 s (action menu evaluation).
	protected bool CachedScan(bool fresh, bool value)
	{
		if (fresh)
			m_fScanTime = -10;

		float now = GetGame().GetWorld().GetWorldTime() / 1000.0;
		if (fresh || now - m_fScanTime > 1)
		{
			m_fScanTime = now;
			m_bScanResult = value;
		}

		return m_bScanResult;
	}

	//------------------------------------------------------------------------------------------------
	//! Scan due (older than 1 s).
	protected bool ScanDue(IEntity user = null)
	{
		if (user != m_ScanUser)
		{
			m_ScanUser = user;
			return true;
		}

		return GetGame().GetWorld().GetWorldTime() / 1000.0 - m_fScanTime > 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Not for carried items or inventory content.
	protected bool IsPlaced(IEntity owner)
	{
		return !ROOT_AER_Util.IsCarried(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! Effects run on the authority only; clients need no broadcast.
	override bool CanBroadcastScript()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return IsPlaced(GetOwner()) && !ROOT_AER_Util.IsDestroyed(GetOwner());
	}
}
