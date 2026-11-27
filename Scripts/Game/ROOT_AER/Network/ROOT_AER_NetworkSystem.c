//! Server-side network: gateway allocation, router tree, DHCP, wireless links, routing and SSH policy.
class ROOT_AER_NetworkSystem
{
	protected static ref ROOT_AER_NetworkSystem s_Instance;
	protected ref array<ROOT_AER_NetworkComponent> m_aRouters = {};
	protected ref array<ROOT_AER_NetworkComponent> m_aClients = {};
	protected int m_iNextSubnet = 1;
	protected ref array<ref ROOT_AER_ConnectCondition> m_aConditions = {};
	protected ref array<ref ROOT_AER_RangeModifier> m_aRangeModifiers = {};
	protected bool m_bRunning;

	//------------------------------------------------------------------------------------------------
	static ROOT_AER_NetworkSystem Get()
	{
		if (!s_Instance)
			s_Instance = new ROOT_AER_NetworkSystem();

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
	void Register(ROOT_AER_NetworkComponent n)
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
			if (gw.IsEmpty() || !ROOT_AER_Util.ParseIp(gw, a, b, c, d) || FindRouterBySubnet(ROOT_AER_Util.Subnet(gw)))
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
	void Unregister(ROOT_AER_NetworkComponent n)
	{
		if (n.IsRouter())
		{
			array<ROOT_AER_NetworkComponent> list = {};
			list.Copy(n.GetClients());
			foreach (ROOT_AER_NetworkComponent c : list)
			{
				Disconnect(c);
			}

			list.Clear();
			list.Copy(n.GetChildren());
			foreach (ROOT_AER_NetworkComponent r : list)
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
	void ReassignGateway(ROOT_AER_NetworkComponent n, string gw)
	{
		int a, b, c, d;
		if (!ROOT_AER_Util.ParseIp(gw, a, b, c, d) || gw == n.GetGateway())
			return;

		ROOT_AER_NetworkComponent other = FindRouterBySubnet(ROOT_AER_Util.Subnet(gw));
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

		array<ROOT_AER_NetworkComponent> list = {};
		list.Copy(n.GetClients());
		foreach (ROOT_AER_NetworkComponent client : list)
		{
			Disconnect(client);
		}

		n.GetDhcp().Remove(n.GetGateway());
		n.SetGateway(gw);
		n.SetAddress(gw);
		n.GetDhcp().Set(gw, n);
	}

	//------------------------------------------------------------------------------------------------
	void AddConnectCondition(ROOT_AER_ConnectCondition c)
	{
		if (c && !m_aConditions.Contains(c))
			m_aConditions.Insert(c);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveConnectCondition(ROOT_AER_ConnectCondition c)
	{
		m_aConditions.RemoveItem(c);
	}

	//------------------------------------------------------------------------------------------------
	void AddRangeModifier(ROOT_AER_RangeModifier m)
	{
		if (m && !m_aRangeModifiers.Contains(m))
			m_aRangeModifiers.Insert(m);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveRangeModifier(ROOT_AER_RangeModifier m)
	{
		m_aRangeModifiers.RemoveItem(m);
	}

	//------------------------------------------------------------------------------------------------
	//! Wireless range of router after the registered modifiers.
	float RangeOf(ROOT_AER_NetworkComponent router)
	{
		float r = router.GetRange();
		foreach (ROOT_AER_RangeModifier m : m_aRangeModifiers)
		{
			if (m)
				r = m.Modify(router, r);
		}

		return Math.Max(0, r);
	}

	//------------------------------------------------------------------------------------------------
	//! All registered conditions allow the link.
	bool Allowed(ROOT_AER_NetworkComponent client, ROOT_AER_NetworkComponent router, out string reason)
	{
		foreach (ROOT_AER_ConnectCondition c : m_aConditions)
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
	protected void Isolate(ROOT_AER_NetworkComponent n)
	{
		array<ROOT_AER_NetworkComponent> list = {};
		list.Copy(n.GetClients());
		foreach (ROOT_AER_NetworkComponent c : list)
		{
			Disconnect(c);
		}

		list.Clear();
		list.Copy(n.GetChildren());
		foreach (ROOT_AER_NetworkComponent r : list)
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
	ROOT_AER_NetworkComponent FindRouterBySubnet(string subnet)
	{
		foreach (ROOT_AER_NetworkComponent r : m_aRouters)
		{
			if (r && ROOT_AER_Util.Subnet(r.GetGateway()) == subnet)
				return r;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	array<ROOT_AER_NetworkComponent> GetRouters()
	{
		return m_aRouters;
	}

	//------------------------------------------------------------------------------------------------
	//! Routers in wireless range of a position, with distance.
	void Scan(vector pos, notnull array<ROOT_AER_NetworkComponent> found)
	{
		foreach (ROOT_AER_NetworkComponent r : m_aRouters)
		{
			if (r && r.IsPowered() && vector.Distance(pos, ROOT_AER_Util.WorldPos(r.GetOwner())) <= RangeOf(r))
				found.Insert(r);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Wireless or editor connection of a client to a router. ignoreRange for editor / GM links.
	bool Connect(ROOT_AER_NetworkComponent client, ROOT_AER_NetworkComponent router, string password, out string reason, bool ignoreRange = false)
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

			if (vector.Distance(ROOT_AER_Util.WorldPos(client.GetOwner()), ROOT_AER_Util.WorldPos(router.GetOwner())) > RangeOf(router))
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
		if (!staticIp.IsEmpty() && ROOT_AER_Util.Subnet(staticIp) == ROOT_AER_Util.Subnet(router.GetGateway()) && !router.GetDhcp().Get(staticIp))
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
		ROOT_AER_Log.Net(string.Format("%1 connected to %2 as %3", client.GetOwner(), router.GetSsid(), ip));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void Disconnect(ROOT_AER_NetworkComponent client)
	{
		if (!client || client.IsRouter())
			return;

		ROOT_AER_NetworkComponent router = client.GetParent();
		if (router)
		{
			router.GetClients().RemoveItem(client);
			router.GetDhcp().Remove(client.GetAddress());
			ROOT_AER_ConnectionComponent link = ROOT_AER_ConnectionComponent.Of(client.GetOwner());
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
	bool LinkRouter(ROOT_AER_NetworkComponent child, ROOT_AER_NetworkComponent parent, out string reason)
	{
		if (child == parent)
		{
			reason = "Invalid network.";
			return false;
		}

		ROOT_AER_NetworkComponent p = parent;
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
	void UnlinkRouter(ROOT_AER_NetworkComponent child)
	{
		ROOT_AER_NetworkComponent parent = child.GetParent();
		if (parent)
		{
			parent.GetChildren().RemoveItem(child);
			ROOT_AER_ConnectionComponent link = ROOT_AER_ConnectionComponent.Of(child.GetOwner());
			if (link)
				link.RemoveWired(parent.GetOwner());
		}

		child.SetParent(null);
		if (child.IsConnected())
			child.SetConnected(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Topmost router of the chain.
	ROOT_AER_NetworkComponent RootOf(ROOT_AER_NetworkComponent router)
	{
		while (router && router.GetParent())
		{
			router = router.GetParent();
		}

		return router;
	}

	//------------------------------------------------------------------------------------------------
	//! Router owning the subnet of an IP.
	ROOT_AER_NetworkComponent ResolveRouter(string ip)
	{
		return FindRouterBySubnet(ROOT_AER_Util.Subnet(ip));
	}

	//------------------------------------------------------------------------------------------------
	//! Device holding an IP (client or router), or the connected computer with that hostname.
	ROOT_AER_NetworkComponent Resolve(string ip)
	{
		int a, b, c, d;
		if (!ROOT_AER_Util.ParseIp(ip, a, b, c, d))
			return ResolveHost(ip);

		ROOT_AER_NetworkComponent r = ResolveRouter(ip);
		if (!r)
			return null;

		return r.GetDhcp().Get(ip);
	}

	//------------------------------------------------------------------------------------------------
	//! Connected computer with a hostname (case insensitive).
	ROOT_AER_NetworkComponent ResolveHost(string host)
	{
		string want = ROOT_AER_Util.Lower(host);
		if (want.IsEmpty())
			return null;

		foreach (ROOT_AER_DeviceComponent dev : ROOT_AER_Manager.Get().GetDevices())
		{
			ROOT_AER_ComputerComponent comp = ROOT_AER_ComputerComponent.Of(dev.GetOwner());
			if (!comp || ROOT_AER_Util.Lower(comp.GetHostname()) != want)
				continue;

			ROOT_AER_NetworkComponent n = comp.GetNetwork();
			if (n && !n.GetAddress().IsEmpty())
				return n;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Router a node attaches to (itself for routers).
	protected ROOT_AER_NetworkComponent AttachRouter(ROOT_AER_NetworkComponent n)
	{
		if (n.IsRouter())
			return n;

		return n.GetParent();
	}

	//------------------------------------------------------------------------------------------------
	//! True when every router between a and b is powered and both share one router tree.
	bool CanReach(ROOT_AER_NetworkComponent a, ROOT_AER_NetworkComponent b, out int hops)
	{
		hops = 0;
		if (!a || !b || !a.IsPowered() || !b.IsPowered())
			return false;

		ROOT_AER_NetworkComponent ra = AttachRouter(a);
		ROOT_AER_NetworkComponent rb = AttachRouter(b);
		if (!ra || !rb)
			return false;

		array<ROOT_AER_NetworkComponent> chainA = {};
		ROOT_AER_NetworkComponent p = ra;
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
	bool SshAllowed(ROOT_AER_NetworkComponent from, ROOT_AER_NetworkComponent target, out string reason)
	{
		int hops;
		ROOT_AER_PowerComponent tp = ROOT_AER_PowerComponent.Of(target.GetOwner());
		if (tp && tp.GetState() == ROOT_AER_EPowerState.STANDBY)
		{
			reason = "Host is in standby.";
			return false;
		}

		if (!CanReach(from, target, hops))
		{
			reason = "Host unreachable.";
			return false;
		}

		if (!target.IsSshEnabled())
		{
			reason = "Connection refused: remote access is off on the host.";
			return false;
		}

		ROOT_AER_NetworkComponent tr = AttachRouter(target);
		if (tr && ROOT_AER_Util.Subnet(from.GetAddress()) != ROOT_AER_Util.Subnet(target.GetAddress()))
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
					if (ROOT_AER_Regex.Full(re, from.GetAddress(), false, false))
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
	float GetTransferSpeed(ROOT_AER_NetworkComponent n)
	{
		if (n && n.GetDeviceSpeed() > 0)
			return n.GetDeviceSpeed();

		ROOT_AER_NetworkComponent r;
		if (n)
			r = AttachRouter(n);

		while (r)
		{
			if (r.GetRouterSpeed() > 0)
				return r.GetRouterSpeed();

			r = r.GetParent();
		}

		return ROOT_AER_SettingsManager.Get().m_fTransferSpeedNetwork;
	}

	//------------------------------------------------------------------------------------------------
	//! Drops wireless clients out of range or with a switched off router.
	protected void Sweep()
	{
		array<ROOT_AER_NetworkComponent> stretched = {};
		foreach (ROOT_AER_NetworkComponent router : m_aRouters)
		{
			if (router && (!router.GetClients().IsEmpty() || !router.GetChildren().IsEmpty() || router.GetParent()) && ROOT_AER_Util.IsCarried(router.GetOwner()))
			{
				Isolate(router);
				continue;
			}

			if (!router || !router.GetParent())
				continue;

			ROOT_AER_ConnectionComponent rlink = ROOT_AER_ConnectionComponent.Of(router.GetOwner());
			if (rlink && rlink.WireStretched(router.GetParent().GetOwner()))
				stretched.Insert(router);
		}

		foreach (ROOT_AER_NetworkComponent cut : stretched)
		{
			UnlinkRouter(cut);
		}

		for (int i = m_aClients.Count() - 1; i >= 0; i--)
		{
			ROOT_AER_NetworkComponent c = m_aClients[i];
			if (!c)
			{
				m_aClients.Remove(i);
				continue;
			}

			ROOT_AER_NetworkComponent r = c.GetParent();
			if (!r)
				continue;

			ROOT_AER_ConnectionComponent link = ROOT_AER_ConnectionComponent.Of(c.GetOwner());
			bool wired = link && link.IsWiredTo(r.GetOwner());
			if (ROOT_AER_Util.IsCarried(r.GetOwner()) || (wired && ROOT_AER_Util.IsCarried(c.GetOwner())))
			{
				Disconnect(c);
				continue;
			}

			if (wired)
			{
				if (link.WireStretched(r.GetOwner()))
					Disconnect(c);

				continue;
			}

			string blocked;
			if (!r.IsPowered() || vector.Distance(ROOT_AER_Util.WorldPos(c.GetOwner()), ROOT_AER_Util.WorldPos(r.GetOwner())) > RangeOf(r) || !Allowed(c, r, blocked))
				Disconnect(c);
		}
	}
}
