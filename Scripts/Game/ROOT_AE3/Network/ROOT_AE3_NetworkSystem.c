//! Server-side network: gateway allocation, router tree, DHCP, wireless links, routing and SSH policy.
class ROOT_AE3_NetworkSystem
{
	protected static ref ROOT_AE3_NetworkSystem s_Instance;
	protected ref array<ROOT_AE3_NetworkComponent> m_aRouters = {};
	protected ref array<ROOT_AE3_NetworkComponent> m_aClients = {};
	protected int m_iNextSubnet = 1;
	protected ref array<ref ROOT_AE3_ConnectCondition> m_aConditions = {};
	protected ref array<ref ROOT_AE3_RangeModifier> m_aRangeModifiers = {};
	protected bool m_bRunning;

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_NetworkSystem Get()
	{
		if (!s_Instance)
			s_Instance = new ROOT_AE3_NetworkSystem();

		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	//! Drops all state (game end).
	static void Reset()
	{
		if (s_Instance)
			GetGame().GetCallqueue().Remove(s_Instance.Sweep);

		s_Instance = null;
	}

	//------------------------------------------------------------------------------------------------
	void Start()
	{
		if (m_bRunning || !Replication.IsServer())
			return;

		m_bRunning = true;
		GetGame().GetCallqueue().CallLater(Sweep, 5000, true);
	}

	//------------------------------------------------------------------------------------------------
	void Register(ROOT_AE3_NetworkComponent n)
	{
		Start();
		if (n.IsRouter())
		{
			if (m_aRouters.Contains(n))
				return;

			m_aRouters.Insert(n);
			string gw = n.GetGatewaySetting();
			int a, b, c, d;
			n.m_bAutoGateway = false;
			if (gw.IsEmpty() || !ROOT_AE3_Util.ParseIp(gw, a, b, c, d) || FindRouterBySubnet(ROOT_AE3_Util.Subnet(gw)))
			{
				gw = AllocateGateway();
				n.m_bAutoGateway = true;
			}

			n.SetGateway(gw);
			n.SetAddress(gw);
			n.GetDhcp().Set(gw, n);
			return;
		}

		if (!m_aClients.Contains(n))
			m_aClients.Insert(n);
	}

	//------------------------------------------------------------------------------------------------
	void Unregister(ROOT_AE3_NetworkComponent n)
	{
		if (n.IsRouter())
		{
			array<ROOT_AE3_NetworkComponent> list = {};
			list.Copy(n.GetClients());
			foreach (ROOT_AE3_NetworkComponent c : list)
			{
				Disconnect(c);
			}

			list.Clear();
			list.Copy(n.GetChildren());
			foreach (ROOT_AE3_NetworkComponent r : list)
			{
				UnlinkRouter(r);
			}

			UnlinkRouter(n);
			m_aRouters.RemoveItem(n);
			return;
		}

		Disconnect(n);
		m_aClients.RemoveItem(n);
	}

	//------------------------------------------------------------------------------------------------
	//! Restored router gateway; ignored when another router uses that subnet.
	void ReassignGateway(ROOT_AE3_NetworkComponent n, string gw)
	{
		int a, b, c, d;
		if (!ROOT_AE3_Util.ParseIp(gw, a, b, c, d) || gw == n.GetGateway())
			return;

		ROOT_AE3_NetworkComponent other = FindRouterBySubnet(ROOT_AE3_Util.Subnet(gw));
		if (other && other != n)
		{
			if (!other.m_bAutoGateway)
				return;

			// The saved subnet wins over one handed out automatically before the restore.
			other.GetDhcp().Remove(other.GetGateway());
			other.SetGateway("0.0.0.0");
			string moved = AllocateGateway();
			ReassignGateway(other, moved);
		}

		n.m_bAutoGateway = false;

		array<ROOT_AE3_NetworkComponent> list = {};
		list.Copy(n.GetClients());
		foreach (ROOT_AE3_NetworkComponent client : list)
		{
			Disconnect(client);
		}

		n.GetDhcp().Remove(n.GetGateway());
		n.SetGateway(gw);
		n.SetAddress(gw);
		n.GetDhcp().Set(gw, n);
	}

	//------------------------------------------------------------------------------------------------
	void AddConnectCondition(ROOT_AE3_ConnectCondition c)
	{
		if (c && !m_aConditions.Contains(c))
			m_aConditions.Insert(c);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveConnectCondition(ROOT_AE3_ConnectCondition c)
	{
		m_aConditions.RemoveItem(c);
	}

	//------------------------------------------------------------------------------------------------
	void AddRangeModifier(ROOT_AE3_RangeModifier m)
	{
		if (m && !m_aRangeModifiers.Contains(m))
			m_aRangeModifiers.Insert(m);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveRangeModifier(ROOT_AE3_RangeModifier m)
	{
		m_aRangeModifiers.RemoveItem(m);
	}

	//------------------------------------------------------------------------------------------------
	//! Wireless range of router after the registered modifiers.
	float RangeOf(ROOT_AE3_NetworkComponent router)
	{
		float r = router.GetRange();
		foreach (ROOT_AE3_RangeModifier m : m_aRangeModifiers)
		{
			if (m)
				r = m.Modify(router, r);
		}

		return Math.Max(0, r);
	}

	//------------------------------------------------------------------------------------------------
	//! All registered conditions allow the link.
	bool Allowed(ROOT_AE3_NetworkComponent client, ROOT_AE3_NetworkComponent router, out string reason)
	{
		foreach (ROOT_AE3_ConnectCondition c : m_aConditions)
		{
			if (c && !c.Allow(client, router, reason))
			{
				if (reason.IsEmpty())
					reason = "Connection blocked.";

				return false;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Drops every client and router link of a router (picked up).
	protected void Isolate(ROOT_AE3_NetworkComponent n)
	{
		array<ROOT_AE3_NetworkComponent> list = {};
		list.Copy(n.GetClients());
		foreach (ROOT_AE3_NetworkComponent c : list)
		{
			Disconnect(c);
		}

		list.Clear();
		list.Copy(n.GetChildren());
		foreach (ROOT_AE3_NetworkComponent r : list)
		{
			UnlinkRouter(r);
		}

		UnlinkRouter(n);
	}

	//------------------------------------------------------------------------------------------------
	protected string AllocateGateway()
	{
		for (int tries = 0; tries < 254; tries++)
		{
			string subnet = "192.168." + m_iNextSubnet.ToString();
			m_iNextSubnet++;
			if (m_iNextSubnet > 254)
				m_iNextSubnet = 1;

			if (!FindRouterBySubnet(subnet))
				return subnet + ".1";
		}

		for (int b = 0; b < 256; b++)
		{
			string wide = "10.0." + b.ToString();
			if (!FindRouterBySubnet(wide))
				return wide + ".1";
		}

		return "10.1.0.1";
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AE3_NetworkComponent FindRouterBySubnet(string subnet)
	{
		foreach (ROOT_AE3_NetworkComponent r : m_aRouters)
		{
			if (r && ROOT_AE3_Util.Subnet(r.GetGateway()) == subnet)
				return r;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	array<ROOT_AE3_NetworkComponent> GetRouters()
	{
		return m_aRouters;
	}

	//------------------------------------------------------------------------------------------------
	//! Routers in wireless range of a position, with distance.
	void Scan(vector pos, notnull array<ROOT_AE3_NetworkComponent> found)
	{
		foreach (ROOT_AE3_NetworkComponent r : m_aRouters)
		{
			if (r && r.IsPowered() && vector.Distance(pos, r.GetOwner().GetOrigin()) <= RangeOf(r))
				found.Insert(r);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Wireless or editor connection of a client to a router. ignoreRange for editor / GM links.
	bool Connect(ROOT_AE3_NetworkComponent client, ROOT_AE3_NetworkComponent router, string password, out string reason, bool ignoreRange = false)
	{
		if (!client || !router || !router.IsRouter())
		{
			reason = "Invalid network.";
			return false;
		}

		if (!Allowed(client, router, reason))
			return false;

		if (client.IsRouter())
			return LinkRouter(client, router, reason);

		if (!ignoreRange)
		{
			if (!router.IsPowered())
			{
				reason = "Network is not available.";
				return false;
			}

			if (!router.AcceptsConnections())
			{
				reason = "Network does not accept new connections.";
				return false;
			}

			if (vector.Distance(client.GetOwner().GetOrigin(), router.GetOwner().GetOrigin()) > RangeOf(router))
			{
				reason = "Network is out of range.";
				return false;
			}

			if (!router.GetPassword().IsEmpty() && router.GetPassword() != password)
			{
				reason = "Wrong network password.";
				return false;
			}
		}

		Disconnect(client);
		string ip;
		string staticIp = client.GetStaticIp();
		if (!staticIp.IsEmpty() && ROOT_AE3_Util.Subnet(staticIp) == ROOT_AE3_Util.Subnet(router.GetGateway()) && !router.GetDhcp().Get(staticIp))
			ip = staticIp;
		else
			ip = router.NextFreeAddress();

		if (ip.IsEmpty())
		{
			reason = "No free address.";
			return false;
		}

		router.GetDhcp().Set(ip, client);
		router.GetClients().Insert(client);
		client.SetParent(router);
		client.SetAddress(ip);
		client.SetGateway(router.GetGateway());
		client.SetConnected(true);
		ROOT_AE3_Log.Net(string.Format("%1 connected to %2 as %3", client.GetOwner(), router.GetSsid(), ip));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void Disconnect(ROOT_AE3_NetworkComponent client)
	{
		if (!client || client.IsRouter())
			return;

		ROOT_AE3_NetworkComponent router = client.GetParent();
		if (router)
		{
			router.GetClients().RemoveItem(client);
			router.GetDhcp().Remove(client.GetAddress());
			ROOT_AE3_ConnectionComponent link = ROOT_AE3_ConnectionComponent.Of(client.GetOwner());
			if (link)
				link.RemoveWired(router.GetOwner());
		}

		client.SetParent(null);
		if (client.IsConnected() || !client.GetAddress().IsEmpty())
		{
			client.SetAddress("");
			client.SetGateway("");
			client.SetConnected(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Cascades child under parent (router to router cable).
	bool LinkRouter(ROOT_AE3_NetworkComponent child, ROOT_AE3_NetworkComponent parent, out string reason)
	{
		if (child == parent)
		{
			reason = "Invalid network.";
			return false;
		}

		ROOT_AE3_NetworkComponent p = parent;
		while (p)
		{
			if (p == child)
			{
				reason = "Connection would form a loop.";
				return false;
			}

			p = p.GetParent();
		}

		UnlinkRouter(child);
		child.SetParent(parent);
		parent.GetChildren().Insert(child);
		child.SetConnected(true);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void UnlinkRouter(ROOT_AE3_NetworkComponent child)
	{
		ROOT_AE3_NetworkComponent parent = child.GetParent();
		if (parent)
		{
			parent.GetChildren().RemoveItem(child);
			ROOT_AE3_ConnectionComponent link = ROOT_AE3_ConnectionComponent.Of(child.GetOwner());
			if (link)
				link.RemoveWired(parent.GetOwner());
		}

		child.SetParent(null);
		if (child.IsConnected())
			child.SetConnected(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Topmost router of the chain.
	ROOT_AE3_NetworkComponent RootOf(ROOT_AE3_NetworkComponent router)
	{
		while (router && router.GetParent())
		{
			router = router.GetParent();
		}

		return router;
	}

	//------------------------------------------------------------------------------------------------
	//! Router owning the subnet of an IP.
	ROOT_AE3_NetworkComponent ResolveRouter(string ip)
	{
		return FindRouterBySubnet(ROOT_AE3_Util.Subnet(ip));
	}

	//------------------------------------------------------------------------------------------------
	//! Device holding an IP (client or router).
	ROOT_AE3_NetworkComponent Resolve(string ip)
	{
		ROOT_AE3_NetworkComponent r = ResolveRouter(ip);
		if (!r)
			return null;

		return r.GetDhcp().Get(ip);
	}

	//------------------------------------------------------------------------------------------------
	//! Router a node attaches to (itself for routers).
	protected ROOT_AE3_NetworkComponent AttachRouter(ROOT_AE3_NetworkComponent n)
	{
		if (n.IsRouter())
			return n;

		return n.GetParent();
	}

	//------------------------------------------------------------------------------------------------
	//! True when every router between a and b is powered and both share one router tree.
	bool CanReach(ROOT_AE3_NetworkComponent a, ROOT_AE3_NetworkComponent b, out int hops)
	{
		hops = 0;
		if (!a || !b || !a.IsPowered() || !b.IsPowered())
			return false;

		ROOT_AE3_NetworkComponent ra = AttachRouter(a);
		ROOT_AE3_NetworkComponent rb = AttachRouter(b);
		if (!ra || !rb)
			return false;

		array<ROOT_AE3_NetworkComponent> chainA = {};
		ROOT_AE3_NetworkComponent p = ra;
		while (p)
		{
			chainA.Insert(p);
			p = p.GetParent();
		}

		p = rb;
		int hb = 0;
		while (p)
		{
			int idx = chainA.Find(p);
			if (idx >= 0)
			{
				for (int i = 0; i <= idx; i++)
				{
					if (!chainA[i].IsPowered())
						return false;
				}

				hops = idx + hb + 1;
				return true;
			}

			if (!p.IsPowered())
				return false;

			hb++;
			p = p.GetParent();
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! SSH policy: target accepts SSH, external allow list of the target router.
	bool SshAllowed(ROOT_AE3_NetworkComponent from, ROOT_AE3_NetworkComponent target, out string reason)
	{
		int hops;
		if (!CanReach(from, target, hops))
		{
			reason = "Host unreachable.";
			return false;
		}

		if (!target.IsSshEnabled())
		{
			reason = "Connection refused.";
			return false;
		}

		ROOT_AE3_NetworkComponent tr = AttachRouter(target);
		if (tr && ROOT_AE3_Util.Subnet(from.GetAddress()) != ROOT_AE3_Util.Subnet(target.GetAddress()))
		{
			if (!tr.AllowsExternalSsh())
			{
				reason = "Connection refused by the network.";
				return false;
			}

			array<string> allow = tr.GetExternalAllow();
			if (allow && !allow.IsEmpty())
			{
				foreach (string re : allow)
				{
					if (ROOT_AE3_Regex.Full(re, from.GetAddress(), false, false))
						return true;
				}

				reason = "Connection refused by the network.";
				return false;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Transfer speed in KB/s: device > router chain > setting.
	float GetTransferSpeed(ROOT_AE3_NetworkComponent n)
	{
		if (n && n.GetDeviceSpeed() > 0)
			return n.GetDeviceSpeed();

		ROOT_AE3_NetworkComponent r;
		if (n)
			r = AttachRouter(n);

		while (r)
		{
			if (r.GetRouterSpeed() > 0)
				return r.GetRouterSpeed();

			r = r.GetParent();
		}

		return ROOT_AE3_SettingsManager.Get().m_fTransferSpeedNetwork;
	}

	//------------------------------------------------------------------------------------------------
	//! Drops wireless clients out of range or with a switched off router.
	protected void Sweep()
	{
		array<ROOT_AE3_NetworkComponent> stretched = {};
		foreach (ROOT_AE3_NetworkComponent router : m_aRouters)
		{
			if (router && (!router.GetClients().IsEmpty() || !router.GetChildren().IsEmpty() || router.GetParent()) && ROOT_AE3_Util.IsCarried(router.GetOwner()))
			{
				Isolate(router);
				continue;
			}

			if (!router || !router.GetParent())
				continue;

			ROOT_AE3_ConnectionComponent rlink = ROOT_AE3_ConnectionComponent.Of(router.GetOwner());
			if (rlink && rlink.WireStretched(router.GetParent().GetOwner()))
				stretched.Insert(router);
		}

		foreach (ROOT_AE3_NetworkComponent cut : stretched)
		{
			UnlinkRouter(cut);
		}

		for (int i = m_aClients.Count() - 1; i >= 0; i--)
		{
			ROOT_AE3_NetworkComponent c = m_aClients[i];
			if (!c)
			{
				m_aClients.Remove(i);
				continue;
			}

			ROOT_AE3_NetworkComponent r = c.GetParent();
			if (!r)
				continue;

			if (ROOT_AE3_Util.IsCarried(c.GetOwner()) || ROOT_AE3_Util.IsCarried(r.GetOwner()))
			{
				Disconnect(c);
				continue;
			}

			ROOT_AE3_ConnectionComponent link = ROOT_AE3_ConnectionComponent.Of(c.GetOwner());
			if (link && link.IsWiredTo(r.GetOwner()))
			{
				if (link.WireStretched(r.GetOwner()))
					Disconnect(c);

				continue;
			}

			string blocked;
			if (!r.IsPowered() || vector.Distance(c.GetOwner().GetOrigin(), r.GetOwner().GetOrigin()) > RangeOf(r) || !Allowed(c, r, blocked))
				Disconnect(c);
		}
	}
}
