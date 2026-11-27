//! Network cable of a device: plug a carried cable into a router (form with the routers within the longest
//! carried cable and the cable lengths, ROOT_AE3_GmDialog.OpenCable), or unplug it (cable back to the user).
//! Routers cascade to a parent router the same way.
class ROOT_AE3_ConnectNetworkAction : ROOT_AE3_UserActionBase
{
	//------------------------------------------------------------------------------------------------
	protected bool Wired(IEntity owner)
	{
		ROOT_AE3_ConnectionComponent link = ROOT_AE3_ConnectionComponent.Of(owner);
		return link && link.GetNetPeer() != RplId.Invalid();
	}

	//------------------------------------------------------------------------------------------------
	//! Distinct lengths of the carried cables, ascending.
	protected static void Lengths(IEntity user, notnull array<float> outLengths)
	{
		outLengths.Clear();
		array<IEntity> items = {};
		ROOT_AE3_CableComponent.Carried(user, items);
		foreach (IEntity item : items)
		{
			float l = ROOT_AE3_CableComponent.Of(item).GetLength();
			if (!outLengths.Contains(l))
				outLengths.Insert(l);
		}

		outLengths.Sort();
	}

	//------------------------------------------------------------------------------------------------
	//! Routers within reach m, nearest first.
	protected static void Routers(IEntity owner, float reach, notnull array<ROOT_AE3_NetworkComponent> outRouters, notnull array<float> outDist)
	{
		outRouters.Clear();
		outDist.Clear();
		foreach (ROOT_AE3_DeviceComponent d : ROOT_AE3_Manager.Get().GetDevices())
		{
			if (!d || d.IsHidden() || d.GetOwner() == owner || ROOT_AE3_Util.IsCarried(d.GetOwner()))
				continue;

			ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(d.GetOwner());
			if (!n || !n.IsRouter())
				continue;

			float dist = vector.Distance(owner.GetOrigin(), d.GetOwner().GetOrigin());
			if (dist > reach)
				continue;

			int at = 0;
			while (at < outDist.Count() && outDist[at] <= dist)
			{
				at++;
			}

			outRouters.InsertAt(n, at);
			outDist.InsertAt(dist, at);
		}
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
		if (Wired(GetOwner()))
			return true;

		if (!ScanDue(user))
		{
			if (!m_bScanResult)
				SetCannotPerformReason("No network cable");

			return m_bScanResult;
		}

		array<float> lengths = {};
		Lengths(user, lengths);
		if (!CachedScan(true, !lengths.IsEmpty()))
		{
			SetCannotPerformReason("No network cable");
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (Wired(pOwnerEntity))
		{
			if (IsAuthority(pOwnerEntity) && InReach(pOwnerEntity, pUserEntity))
				Unplug(pOwnerEntity, pUserEntity);

			return;
		}

		if (pUserEntity == SCR_PlayerController.GetLocalControlledEntity())
			OpenForm(pOwnerEntity, pUserEntity);
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
	//! Local player: form with the routers within the longest carried cable and the cable lengths.
	protected void OpenForm(IEntity owner, IEntity user)
	{
		array<float> lengths = {};
		Lengths(user, lengths);
		if (lengths.IsEmpty())
			return;

		array<ROOT_AE3_NetworkComponent> routers = {};
		array<float> dists = {};
		Routers(owner, lengths[lengths.Count() - 1], routers, dists);
		if (routers.IsEmpty())
		{
			SCR_HintManagerComponent.ShowCustomHint(string.Format("No router within %1 m.", lengths[lengths.Count() - 1]), "Advanced Equipment", 4);
			return;
		}

		array<string> ids = {};
		array<string> labels = {};
		foreach (int i, ROOT_AE3_NetworkComponent r : routers)
		{
			ROOT_AE3_DeviceComponent d = ROOT_AE3_DeviceComponent.Of(r.GetOwner());
			int id = d.GetRplId();
			ids.Insert(id.ToString());
			string name = d.GetDeviceName();
			if (!r.GetSsid().IsEmpty() && r.GetSsid() != name)
				name = r.GetSsid() + " (" + name + ")";

			labels.Insert(string.Format("%1 - %2 m", name, Math.Ceil(dists[i])));
		}

		// Shortest cable that reaches the nearest router.
		array<string> lens = {};
		array<string> lenLabels = {};
		string pick;
		foreach (float l : lengths)
		{
			lens.Insert(l.ToString());
			lenLabels.Insert(l.ToString() + " m");
			if (pick.IsEmpty() && l >= dists[0])
				pick = l.ToString();
		}

		array<ref ROOT_AE3_GmField> fields = {
			ROOT_AE3_GmField.Choice("Router", ids, labels, ids[0]),
			ROOT_AE3_GmField.Choice("Cable", lens, lenLabels, pick)
		};
		ROOT_AE3_GmDialog.OpenCable(owner, fields);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (Wired(GetOwner()))
			outName = "Unplug network cable";
		else
			outName = "Plug network cable...";

		return true;
	}
}
