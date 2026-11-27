[ComponentEditorProps(category: "Advanced Equipment Reforged", description: "Network router or network client")]
class ROOT_AER_NetworkComponentClass : ScriptComponentClass
{
}

//! Router or client of the virtual IP network. Links and addressing are managed by ROOT_AER_NetworkSystem on the authority.
//! Replicated: address, gateway, SSID, connection flag.
class ROOT_AER_NetworkComponent : ScriptComponent
{
	[Attribute("0", desc: "Router (otherwise client)", category: "Network")]
	protected bool m_bRouter;

	// Router
	[Attribute("", desc: "Router SSID; empty = device name", category: "Router")]
	protected string m_sSsid;

	[Attribute("", desc: "Router gateway a.b.c.1; empty = automatic 192.168.N.1", category: "Router")]
	protected string m_sGatewaySetting;

	[Attribute("-1", desc: "Wireless range in m, -1 = setting default", params: "-1 100000 1", category: "Router")]
	protected float m_fRange;

	[Attribute("1", desc: "Range multiplier", params: "0 100 0.1", category: "Router")]
	protected float m_fRangeMultiplier;

	[Attribute("1", desc: "Accept new wireless clients", category: "Router")]
	protected bool m_bAcceptConnections;

	[Attribute("", desc: "Wireless password; empty = open", category: "Router")]
	protected string m_sPassword;

	[Attribute("1", desc: "Allow SSH from other subnets", category: "Router")]
	protected bool m_bAllowExternalSsh;

	[Attribute("", desc: "Regular expressions of source IPs allowed for external SSH; empty = all", category: "Router")]
	protected ref array<string> m_aExternalAllow;

	[Attribute("0", desc: "Transfer speed in KB/s through this router, 0 = setting", params: "0 1000000 1", category: "Router")]
	protected float m_fRouterSpeed;

	// Client
	[Attribute("", desc: "Static IP inside the router subnet; empty = DHCP", category: "Client")]
	protected string m_sStaticIp;

	[Attribute("0", desc: "Device transfer speed in KB/s, 0 = router / setting", params: "0 1000000 1", category: "Client")]
	protected float m_fDeviceSpeed;

	[Attribute("0", desc: "Accept SSH logins", category: "Client")]
	protected bool m_bSshEnabled;

	[RplProp()]
	protected string m_sAddress;

	[RplProp()]
	protected string m_sGateway;

	[RplProp()]
	protected string m_sSsidRpl;

	[RplProp()]
	protected bool m_bConnected;

	protected ROOT_AER_NetworkComponent m_Parent;
	protected ref array<ROOT_AER_NetworkComponent> m_aChildren = {};
	protected ref array<ROOT_AER_NetworkComponent> m_aClients = {};
	protected ref map<string, ROOT_AER_NetworkComponent> m_mDhcp = new map<string, ROOT_AER_NetworkComponent>();
	protected int m_iDhcpCounter = 10;
	bool m_bAutoGateway;	//!< Gateway allocated automatically (may be moved for a restored router)
	protected RplComponent m_RplComponent;

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!m_aExternalAllow)
			m_aExternalAllow = {};

		if (!GetGame().InPlayMode() || !IsAuthority())
			return;

		ROOT_AER_NetworkSystem.Get().Register(this);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		if (IsAuthority() && GetGame().InPlayMode() && !ROOT_AER_Manager.IsEnding())
			ROOT_AER_NetworkSystem.Get().Unregister(this);

		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	static ROOT_AER_NetworkComponent Of(IEntity ent)
	{
		if (!ent)
			return null;

		return ROOT_AER_NetworkComponent.Cast(ent.FindComponent(ROOT_AER_NetworkComponent));
	}

	//------------------------------------------------------------------------------------------------
	bool IsAuthority()
	{
		return !m_RplComponent || !m_RplComponent.IsProxy();
	}

	//------------------------------------------------------------------------------------------------
	bool IsRouter()
	{
		return m_bRouter;
	}

	//------------------------------------------------------------------------------------------------
	string GetAddress()
	{
		return m_sAddress;
	}

	//------------------------------------------------------------------------------------------------
	string GetGateway()
	{
		return m_sGateway;
	}

	//------------------------------------------------------------------------------------------------
	string GetSsid()
	{
		if (!m_sSsidRpl.IsEmpty())
			return m_sSsidRpl;

		if (!m_sSsid.IsEmpty())
			return m_sSsid;

		ROOT_AER_DeviceComponent d = ROOT_AER_DeviceComponent.Of(GetOwner());
		if (d)
			return d.GetDeviceName();

		return "Network";
	}

	//------------------------------------------------------------------------------------------------
	bool IsConnected()
	{
		return m_bConnected;
	}

	//------------------------------------------------------------------------------------------------
	string GetGatewaySetting()
	{
		return m_sGatewaySetting;
	}

	//------------------------------------------------------------------------------------------------
	//! Configured range in m before multipliers.
	float GetBaseRange()
	{
		if (m_fRange < 0)
			return ROOT_AER_SettingsManager.Get().m_fDefaultWirelessRange;

		return m_fRange;
	}

	//------------------------------------------------------------------------------------------------
	float GetRange()
	{
		ROOT_AER_Settings s = ROOT_AER_SettingsManager.Get();
		float r = m_fRange;
		if (r < 0)
			r = s.m_fDefaultWirelessRange;

		return r * m_fRangeMultiplier * s.m_fWirelessRangeMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	bool AcceptsConnections()
	{
		return m_bAcceptConnections;
	}

	//------------------------------------------------------------------------------------------------
	string GetPassword()
	{
		return m_sPassword;
	}

	//------------------------------------------------------------------------------------------------
	bool AllowsExternalSsh()
	{
		return m_bAllowExternalSsh;
	}

	//------------------------------------------------------------------------------------------------
	array<string> GetExternalAllow()
	{
		return m_aExternalAllow;
	}

	//------------------------------------------------------------------------------------------------
	float GetRouterSpeed()
	{
		return m_fRouterSpeed;
	}

	//------------------------------------------------------------------------------------------------
	float GetDeviceSpeed()
	{
		return m_fDeviceSpeed;
	}

	//------------------------------------------------------------------------------------------------
	string GetStaticIp()
	{
		return m_sStaticIp;
	}

	//------------------------------------------------------------------------------------------------
	bool IsSshEnabled()
	{
		return m_bSshEnabled;
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AER_NetworkComponent GetParent()
	{
		return m_Parent;
	}

	//------------------------------------------------------------------------------------------------
	array<ROOT_AER_NetworkComponent> GetChildren()
	{
		return m_aChildren;
	}

	//------------------------------------------------------------------------------------------------
	array<ROOT_AER_NetworkComponent> GetClients()
	{
		return m_aClients;
	}

	//------------------------------------------------------------------------------------------------
	map<string, ROOT_AER_NetworkComponent> GetDhcp()
	{
		return m_mDhcp;
	}

	//------------------------------------------------------------------------------------------------
	//! Router: next free host address of the subnet.
	string NextFreeAddress()
	{
		string subnet = ROOT_AER_Util.Subnet(m_sGateway);
		for (int tries = 0; tries < 250; tries++)
		{
			string ip = subnet + "." + m_iDhcpCounter.ToString();
			m_iDhcpCounter++;
			if (m_iDhcpCounter > 254)
				m_iDhcpCounter = 10;

			if (!m_mDhcp.Get(ip))
				return ip;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! True when the device is switched on (or has no power component).
	bool IsPowered()
	{
		ROOT_AER_PowerComponent p = ROOT_AER_PowerComponent.Of(GetOwner());
		return !p || p.IsOn();
	}

	//------------------------------------------------------------------------------------------------
	//! Settings changed at runtime (GM, router admin, network app) for persistence.
	string PackConfig()
	{
		array<string> f = {m_sSsid, m_sPassword, m_fRange.ToString(), m_fRangeMultiplier.ToString(), ROOT_AER_Util.BoolStr(m_bAcceptConnections),
			ROOT_AER_Util.BoolStr(m_bAllowExternalSsh), ROOT_AER_Codec.JoinSub(m_aExternalAllow), m_fRouterSpeed.ToString(), m_sStaticIp,
			m_fDeviceSpeed.ToString(), ROOT_AER_Util.BoolStr(m_bSshEnabled), m_sGateway};
		return ROOT_AER_Codec.Join(f);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority.
	void UnpackConfig(string data)
	{
		array<string> f = ROOT_AER_Codec.Split(data);
		if (f.Count() < 11)
			return;

		m_sSsid = f[0];
		m_sSsidRpl = f[0];
		m_sPassword = f[1];
		m_fRange = f[2].ToFloat();
		m_fRangeMultiplier = f[3].ToFloat();
		m_bAcceptConnections = f[4] == "1";
		m_bAllowExternalSsh = f[5] == "1";
		m_aExternalAllow = ROOT_AER_Codec.SplitSub(f[6]);
		m_fRouterSpeed = f[7].ToFloat();
		m_sStaticIp = f[8];
		m_fDeviceSpeed = f[9].ToFloat();
		m_bSshEnabled = f[10] == "1";
		if (f.Count() > 11 && IsRouter() && !f[11].IsEmpty())
			ROOT_AER_NetworkSystem.Get().ReassignGateway(this, f[11]);

		Replication.BumpMe();
	}

	// Authority setters ------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	void SetAddress(string ip)
	{
		m_sAddress = ip;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	void SetGateway(string gw)
	{
		m_sGateway = gw;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	void SetConnected(bool connected)
	{
		m_bConnected = connected;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	void SetParent(ROOT_AER_NetworkComponent parent)
	{
		m_Parent = parent;
	}

	//------------------------------------------------------------------------------------------------
	void ConfigureRouter(string ssid, string password, float range, float multiplier, bool accept, bool allowExternalSsh, float speed)
	{
		if (!ssid.IsEmpty())
		{
			m_sSsid = ssid;
			m_sSsidRpl = ssid;
		}

		m_sPassword = password;
		if (range != 0)
			m_fRange = range;

		if (multiplier > 0)
			m_fRangeMultiplier = multiplier;

		m_bAcceptConnections = accept;
		m_bAllowExternalSsh = allowExternalSsh;
		if (speed >= 0)
			m_fRouterSpeed = speed;

		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	void SetExternalAllow(notnull array<string> list)
	{
		m_aExternalAllow.Copy(list);
	}

	//------------------------------------------------------------------------------------------------
	void SetGatewaySetting(string gw)
	{
		m_sGatewaySetting = gw;
	}

	//------------------------------------------------------------------------------------------------
	void SetStaticIp(string ip)
	{
		m_sStaticIp = ip;
	}

	//------------------------------------------------------------------------------------------------
	void SetDeviceSpeed(float kbps)
	{
		m_fDeviceSpeed = Math.Max(0, kbps);
	}

	//------------------------------------------------------------------------------------------------
	void SetSshEnabled(bool enabled)
	{
		m_bSshEnabled = enabled;
	}

	//------------------------------------------------------------------------------------------------
	void SetRouterSpeed(float kbps)
	{
		m_fRouterSpeed = Math.Max(0, kbps);
	}
}
