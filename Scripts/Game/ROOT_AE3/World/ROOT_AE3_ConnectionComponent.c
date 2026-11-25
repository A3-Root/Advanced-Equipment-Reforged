[ComponentEditorProps(category: "Advanced Equipment", description: "Power and network links set in the World Editor (by entity name)")]
class ROOT_AE3_ConnectionComponentClass : ScriptComponentClass
{
}

//! Wired power / network links. Editor links are made on the authority after the world loaded;
//! wired network links ignore the wireless range.
class ROOT_AE3_ConnectionComponent : ScriptComponent
{
	[Attribute("", desc: "Entity name of the power source", category: "Connections")]
	protected string m_sPowerSource;

	[Attribute("", desc: "Entity name of the router (network) or parent router (for routers)", category: "Connections")]
	protected string m_sNetwork;

	protected ref array<IEntity> m_aWired = {};
	protected ref array<float> m_aWiredLength = {};
	protected bool m_bRestored;

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (!GetGame().InPlayMode() || !Replication.IsServer())
			return;

		GetGame().GetCallqueue().CallLater(LinkFromEditor, 1500, false);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		GetGame().GetCallqueue().Remove(LinkFromEditor);
		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_ConnectionComponent Of(IEntity ent)
	{
		if (!ent)
			return null;

		return ROOT_AE3_ConnectionComponent.Cast(ent.FindComponent(ROOT_AE3_ConnectionComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! Persistence restored the links: editor links are not applied.
	void MarkRestored()
	{
		m_bRestored = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void LinkFromEditor()
	{
		if (m_bRestored)
			return;

		string reason;
		if (!m_sPowerSource.IsEmpty())
		{
			IEntity src = GetGame().GetWorld().FindEntityByName(m_sPowerSource);
			if (!ConnectPower(GetOwner(), src, reason))
				ROOT_AE3_Log.Warn(string.Format("%1: power link to '%2' failed: %3", GetOwner(), m_sPowerSource, reason));
		}

		if (!m_sNetwork.IsEmpty())
		{
			IEntity router = GetGame().GetWorld().FindEntityByName(m_sNetwork);
			if (!ConnectNetwork(GetOwner(), router, reason))
				ROOT_AE3_Log.Warn(string.Format("%1: network link to '%2' failed: %3", GetOwner(), m_sNetwork, reason));
		}
	}

	//------------------------------------------------------------------------------------------------
	bool IsWiredTo(IEntity other)
	{
		return m_aWired.Contains(other);
	}

	//------------------------------------------------------------------------------------------------
	void AddWired(IEntity other)
	{
		if (!other || m_aWired.Contains(other))
			return;

		m_aWired.Insert(other);
		m_aWiredLength.Insert(vector.Distance(GetOwner().GetOrigin(), other.GetOrigin()));
	}

	//------------------------------------------------------------------------------------------------
	void RemoveWired(IEntity other)
	{
		int i = m_aWired.Find(other);
		if (i < 0)
			return;

		m_aWired.Remove(i);
		m_aWiredLength.Remove(i);
	}

	//------------------------------------------------------------------------------------------------
	//! Cable to other stretched beyond its plugged length.
	bool WireStretched(IEntity other)
	{
		int i = m_aWired.Find(other);
		return i >= 0 && vector.Distance(GetOwner().GetOrigin(), other.GetOrigin()) > m_aWiredLength[i] + 2;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: wired power link from consumer to source.
	static bool ConnectPower(IEntity consumer, IEntity source, out string reason)
	{
		ROOT_AE3_PowerComponent c = ROOT_AE3_PowerComponent.Of(consumer);
		ROOT_AE3_PowerComponent s = ROOT_AE3_PowerComponent.Of(source);
		if (!c || !s)
		{
			reason = "Both objects need a power component.";
			return false;
		}

		return c.ConnectTo(s, reason);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: wired network link of a device to a router (router to router cascades).
	static bool ConnectNetwork(IEntity device, IEntity router, out string reason)
	{
		ROOT_AE3_NetworkComponent d = ROOT_AE3_NetworkComponent.Of(device);
		ROOT_AE3_NetworkComponent r = ROOT_AE3_NetworkComponent.Of(router);
		if (!d || !r)
		{
			reason = "Both objects need a network component.";
			return false;
		}

		if (!ROOT_AE3_NetworkSystem.Get().Connect(d, r, "", reason, true))
			return false;

		ROOT_AE3_ConnectionComponent link = Of(device);
		if (link)
			link.AddWired(router);

		return true;
	}
}
