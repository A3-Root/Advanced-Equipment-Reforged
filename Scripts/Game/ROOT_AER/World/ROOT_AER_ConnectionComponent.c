[ComponentEditorProps(category: "Advanced Equipment Reforged", description: "Power and network links set in the World Editor (by entity name), cable ports")]
class ROOT_AER_ConnectionComponentClass : ScriptComponentClass
{
}

//! Wired power / network links. Editor links are made on the authority after the world loaded;
//! wired network links ignore the wireless range. A wired network link keeps its cable length and the cable
//! item prefab (given back when unplugged, dropped at the device when the cable is pulled out).
//! Clients draw the network cable to the router and the power cable to the source (ROOT_AER_CableView).
class ROOT_AER_ConnectionComponent : ScriptComponent
{
	[Attribute("", desc: "Entity name of the power source", category: "Connections")]
	protected string m_sPowerSource;

	[Attribute("", desc: "Entity name of the router (network) or parent router (for routers)", category: "Connections")]
	protected string m_sNetwork;

	[Attribute("0 0.05 0", desc: "Model-relative position of the network port", category: "Cable ports")]
	protected vector m_vNetPort;

	[Attribute("0 0 1", desc: "Model-relative direction out of the network port", category: "Cable ports")]
	protected vector m_vNetPortDir;

	[Attribute("0 0 0", desc: "Offset between network ports (routers: cables of other devices use ports 1-3)", category: "Cable ports")]
	protected vector m_vNetPortStep;

	[Attribute("0 0.05 0", desc: "Model-relative position of the power port", category: "Cable ports")]
	protected vector m_vPowerPort;

	[Attribute("0 0 1", desc: "Model-relative direction out of the power port", category: "Cable ports")]
	protected vector m_vPowerPortDir;

	static const ResourceName NET_MATERIAL = "{CFA85D4E05855592}Assets/ROOT_AER/Data/AER_Plastic_Black.emat";
	static const ResourceName POWER_MATERIAL = "{BA1A4CB5E941E41D}Assets/ROOT_AER/Data/AER_Accent_Orange.emat";

	protected ref array<IEntity> m_aWired = {};
	protected ref array<float> m_aWiredCable = {};			//!< Cable length in m
	protected ref array<ResourceName> m_aWiredItem = {};	//!< Cable item prefab, empty = no item
	protected bool m_bRestored;

	[RplProp()]
	protected RplId m_NetPeer = RplId.Invalid();		//!< Router at the other end of the network cable

	[RplProp()]
	protected float m_fNetCable;

	protected ref ROOT_AER_CableView m_NetView;
	protected ref ROOT_AER_CableView m_PowerView;

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (!GetGame().InPlayMode())
			return;

		if (Replication.IsServer())
			GetGame().GetCallqueue().CallLater(LinkFromEditor, 1500, false);

		if (!System.IsConsoleApp())
			GetGame().GetCallqueue().CallLater(UpdateViews, 250, true);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		GetGame().GetCallqueue().Remove(LinkFromEditor);
		GetGame().GetCallqueue().Remove(UpdateViews);
		m_NetView = null;
		m_PowerView = null;
		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	static ROOT_AER_ConnectionComponent Of(IEntity ent)
	{
		if (!ent)
			return null;

		return ROOT_AER_ConnectionComponent.Cast(ent.FindComponent(ROOT_AER_ConnectionComponent));
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
				ROOT_AER_Log.Warn(string.Format("%1: power link to '%2' failed: %3", GetOwner(), m_sPowerSource, reason));
		}

		if (!m_sNetwork.IsEmpty())
		{
			IEntity router = GetGame().GetWorld().FindEntityByName(m_sNetwork);
			if (!ConnectNetwork(GetOwner(), router, reason))
				ROOT_AER_Log.Warn(string.Format("%1: network link to '%2' failed: %3", GetOwner(), m_sNetwork, reason));
		}
	}

	//------------------------------------------------------------------------------------------------
	bool IsWiredTo(IEntity other)
	{
		return m_aWired.Contains(other);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority. cable < 0: no cable item, length = current distance + 2 m.
	void AddWired(IEntity other, float cable = -1, ResourceName item = "")
	{
		if (!other || m_aWired.Contains(other))
			return;

		if (cable <= 0)
			cable = vector.Distance(GetOwner().GetOrigin(), other.GetOrigin()) + 2;

		m_aWired.Insert(other);
		m_aWiredCable.Insert(cable);
		m_aWiredItem.Insert(item);
		ROOT_AER_DeviceComponent d = ROOT_AER_DeviceComponent.Of(other);
		if (d)
		{
			m_NetPeer = d.GetRplId();
			m_fNetCable = cable;
			Replication.BumpMe();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Authority. A cable item still recorded with the link is dropped at the device.
	void RemoveWired(IEntity other)
	{
		int i = m_aWired.Find(other);
		if (i < 0)
			return;

		ResourceName item = m_aWiredItem[i];
		m_aWired.Remove(i);
		m_aWiredCable.Remove(i);
		m_aWiredItem.Remove(i);
		if (!item.IsEmpty())
			ROOT_AER_CableComponent.Give(item, GetOwner(), null);

		m_NetPeer = RplId.Invalid();
		m_fNetCable = 0;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: cable item prefab of the link to other; the link no longer drops it when removed.
	ResourceName TakeCable(IEntity other)
	{
		int i = m_aWired.Find(other);
		if (i < 0)
			return ResourceName.Empty;

		ResourceName item = m_aWiredItem[i];
		m_aWiredItem[i] = ResourceName.Empty;
		return item;
	}

	//------------------------------------------------------------------------------------------------
	//! Router at the end of the network cable (replicated), invalid = no cable.
	RplId GetNetPeer()
	{
		return m_NetPeer;
	}

	//------------------------------------------------------------------------------------------------
	float GetCableLength(IEntity other)
	{
		int i = m_aWired.Find(other);
		if (i < 0)
			return 0;

		return m_aWiredCable[i];
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetCableItem(IEntity other)
	{
		int i = m_aWired.Find(other);
		if (i < 0)
			return ResourceName.Empty;

		return m_aWiredItem[i];
	}

	//------------------------------------------------------------------------------------------------
	//! Distance to other beyond the cable length.
	bool WireStretched(IEntity other)
	{
		int i = m_aWired.Find(other);
		return i >= 0 && vector.Distance(GetOwner().GetOrigin(), other.GetOrigin()) > m_aWiredCable[i];
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: wired power link from consumer to source.
	static bool ConnectPower(IEntity consumer, IEntity source, out string reason)
	{
		ROOT_AER_PowerComponent c = ROOT_AER_PowerComponent.Of(consumer);
		ROOT_AER_PowerComponent s = ROOT_AER_PowerComponent.Of(source);
		if (!c || !s)
		{
			reason = "Both objects need a power component.";
			return false;
		}

		return c.ConnectTo(s, reason);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: wired network link of a device to a router (router to router cascades).
	//! cable: cable length in m (< 0: no cable item, distance + 2 m); item: cable item prefab given back on unplug.
	static bool ConnectNetwork(IEntity device, IEntity router, out string reason, float cable = -1, ResourceName item = "")
	{
		ROOT_AER_NetworkComponent d = ROOT_AER_NetworkComponent.Of(device);
		ROOT_AER_NetworkComponent r = ROOT_AER_NetworkComponent.Of(router);
		if (!d || !r)
		{
			reason = "Both objects need a network component.";
			return false;
		}

		if (!ROOT_AER_NetworkSystem.Get().Connect(d, r, "", reason, true))
			return false;

		ROOT_AER_ConnectionComponent link = Of(device);
		if (link)
			link.AddWired(router, cable, item);

		return true;
	}

	// Cable ports and views (clients) -----------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! World position and direction of the network port; index > 0 adds index * m_vNetPortStep.
	void NetPort(int index, out vector pos, out vector dir)
	{
		float k = index;
		ToWorld(m_vNetPort + m_vNetPortStep * k, m_vNetPortDir, pos, dir);
	}

	//------------------------------------------------------------------------------------------------
	void PowerPort(out vector pos, out vector dir)
	{
		ToWorld(m_vPowerPort, m_vPowerPortDir, pos, dir);
	}

	//------------------------------------------------------------------------------------------------
	protected void ToWorld(vector local, vector localDir, out vector pos, out vector dir)
	{
		vector mat[4];
		GetOwner().GetWorldTransform(mat);
		pos = local.Multiply4(mat);
		dir = (mat[0] * localDir[0] + mat[1] * localDir[1] + mat[2] * localDir[2]).Normalized();
	}

	//------------------------------------------------------------------------------------------------
	//! Client, every 250 ms: cable to the router and to the power source while both ends are placed.
	protected void UpdateViews()
	{
		IEntity owner = GetOwner();
		bool placed = owner && !ROOT_AER_Util.IsCarried(owner);
		vector a, aDir, b, bDir;

		IEntity router;
		ROOT_AER_DeviceComponent rd = ROOT_AER_DeviceComponent.FromRplId(m_NetPeer);
		if (m_NetPeer != RplId.Invalid() && rd)
			router = rd.GetOwner();

		ROOT_AER_ConnectionComponent rl = Of(router);
		if (placed && rl && !ROOT_AER_Util.IsCarried(router))
		{
			if (!m_NetView)
				m_NetView = new ROOT_AER_CableView(NET_MATERIAL);

			NetPort(0, a, aDir);
			ROOT_AER_DeviceComponent own = ROOT_AER_DeviceComponent.Of(owner);
			int port = 1;
			if (own)
			{
				int rid = own.GetRplId();
				int slot = rid % 3;
				port = 1 + slot;
			}

			rl.NetPort(port, b, bDir);
			m_NetView.Update(a, aDir, b, bDir, m_fNetCable);
		}
		else
		{
			m_NetView = null;
		}

		IEntity source;
		ROOT_AER_PowerComponent p = ROOT_AER_PowerComponent.Of(owner);
		if (p && p.GetProviderId() != RplId.Invalid())
		{
			ROOT_AER_DeviceComponent sd = ROOT_AER_DeviceComponent.FromRplId(p.GetProviderId());
			if (sd)
				source = sd.GetOwner();
		}

		ROOT_AER_ConnectionComponent sl = Of(source);
		if (placed && sl && !ROOT_AER_Util.IsCarried(source))
		{
			if (!m_PowerView)
				m_PowerView = new ROOT_AER_CableView(POWER_MATERIAL, ROOT_AER_CableView.POWER_MODEL);

			PowerPort(a, aDir);
			sl.PowerPort(b, bDir);
			m_PowerView.Update(a, aDir, b, bDir, p.GetLinkLength());
		}
		else
		{
			m_PowerView = null;
		}
	}
}
