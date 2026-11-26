//! State of one EWO pack (server).
class ROOT_AE3_EwoPack
{
	IEntity m_Pack;
	int m_iIndex;				//!< Gateway 77.95.N.1 while the network runs, 0 = none
	float m_fEnergy;
	bool m_bWifi;
	string m_sSsid;
	string m_sPassword;
	string m_sAppliedSsid;		//!< Name and password last applied to the router
	string m_sAppliedPassword;
	IEntity m_Router;
	ROOT_AE3_PowerComponent m_Source;
	ref array<IEntity> m_aCharging = {};
}

//------------------------------------------------------------------------------------------------
//! 77th JSOC EWO mode (setting m_bEwoMode). Packs whose prefab matches m_sEwoPrefabs (default: the
//! AN/PRC-77 radio backpack), worn, carried or placed, carry an energy pool (1 energy = 1 Wh = 1 % of a laptop
//! battery), broadcast a wireless network (hidden router ROOT_AE3_EwoRouter that follows the pack while the
//! network runs; name "EWO Net N", random 8 character password, gateway 77.95.N.1 with N the lowest free number
//! 1-254, external SSH only from other EWO subnets), charge laptops in the carrier's inventory and recharge
//! from a running AE3 power source in range, which carries the load like a charging battery.
//! A pack is tracked from the first panel request on; its state stays while EWO mode is off and is not saved.
//! Server owns all state; ROOT_AE3_EwoPanel sends requests (ROOT_AE3_EMsg.EWO with the pack RplId) and receives
//! ROOT_AE3_EMsg.EWO_STATE every tick while open (empty state: close the panel).
class ROOT_AE3_EwoSystem
{
	static const ResourceName ROUTER_PREFAB = "{47769E9EED112426}Prefabs/ROOT_AE3/Devices/ROOT_AE3_EwoRouter.et";
	static const float TICK = 3;
	static const string SUBNET_ALLOW = "77\\.95\\.\\d+\\..*";
	static const string PASSWORD_CHARS = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";

	protected static ref ROOT_AE3_EwoSystem s_Instance;
	protected ref array<ref ROOT_AE3_EwoPack> m_aPacks = {};
	protected ref map<int, IEntity> m_mWatchers = new map<int, IEntity>();	//!< Player with an open panel, pack
	protected ref array<ROOT_AE3_PowerComponent> m_aLoaded = {};		//!< Sources with an EWO load
	protected bool m_bRunning;
	protected int m_iNextName = 1;
	protected static string s_sPrefabSetting;
	protected static ref array<string> s_aPrefabs = {};

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_EwoSystem Get()
	{
		if (!s_Instance)
			s_Instance = new ROOT_AE3_EwoSystem();

		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	//! Drops all state (game end).
	static void Reset()
	{
		if (s_Instance)
			GetGame().GetCallqueue().Remove(s_Instance.Tick);

		s_Instance = null;
		s_sPrefabSetting = "";
		s_aPrefabs.Clear();
	}

	//------------------------------------------------------------------------------------------------
	void Start()
	{
		if (m_bRunning || !Replication.IsServer())
			return;

		m_bRunning = true;
		GetGame().GetCallqueue().CallLater(Tick, TICK * 1000, true);
	}

	//------------------------------------------------------------------------------------------------
	//! EWO mode is on and the item's prefab is one of the configured EWO packs. Server and client.
	static bool IsEwoPack(IEntity item)
	{
		ROOT_AE3_Settings s = ROOT_AE3_SettingsManager.Get();
		if (!item || !s.m_bEwoMode)
			return false;

		EntityPrefabData pd = item.GetPrefabData();
		if (!pd)
			return false;

		if (s.m_sEwoPrefabs != s_sPrefabSetting)
		{
			s_sPrefabSetting = s.m_sEwoPrefabs;
			s_aPrefabs.Clear();
			array<string> parts = {};
			s.m_sEwoPrefabs.Split(",", parts, true);
			foreach (string p : parts)
			{
				p.TrimInPlace();
				if (!p.IsEmpty())
					s_aPrefabs.Insert(ROOT_AE3_Util.Lower(p));
			}
		}

		string prefab = ROOT_AE3_Util.Lower(pd.GetPrefabName());
		foreach (string name : s_aPrefabs)
		{
			if (prefab.Contains(name))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! EWO pack worn by the character, null when none.
	static IEntity WornPack(IEntity character)
	{
		if (!character)
			return null;

		EquipedLoadoutStorageComponent loadout = EquipedLoadoutStorageComponent.Cast(character.FindComponent(EquipedLoadoutStorageComponent));
		if (!loadout)
			return null;

		IEntity pack = loadout.GetClothFromArea(LoadoutBackpackArea);
		if (IsEwoPack(pack))
			return pack;

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The pack is in the character's inventory, or placed within reach.
	static bool InReach(IEntity pack, IEntity character)
	{
		if (!pack || !character)
			return false;

		if (ROOT_AE3_Util.CarriedBy(pack, character))
			return true;

		return !ROOT_AE3_Util.IsCarried(pack) && vector.Distance(pack.GetOrigin(), character.GetOrigin()) <= ROOT_AE3_Manager.USE_RANGE;
	}

	//------------------------------------------------------------------------------------------------
	protected ROOT_AE3_EwoPack Find(IEntity pack)
	{
		foreach (ROOT_AE3_EwoPack p : m_aPacks)
		{
			if (p.m_Pack == pack)
				return p;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected ROOT_AE3_EwoPack Ensure(IEntity pack)
	{
		ROOT_AE3_EwoPack p = Find(pack);
		if (p)
			return p;

		p = new ROOT_AE3_EwoPack();
		p.m_Pack = pack;
		p.m_fEnergy = ROOT_AE3_SettingsManager.Get().m_fEwoEnergyMax;
		p.m_sSsid = "EWO Net " + m_iNextName.ToString();
		m_iNextName++;
		for (int i = 0; i < 8; i++)
		{
			int c = Math.RandomInt(0, PASSWORD_CHARS.Length());
			p.m_sPassword += PASSWORD_CHARS.Get(c);
		}

		m_aPacks.Insert(p);
		return p;
	}

	//------------------------------------------------------------------------------------------------
	//! Lowest gateway number 1-254 no running network uses, 0 when none is free.
	protected int FreeIndex()
	{
		for (int n = 1; n <= 254; n++)
		{
			bool used;
			foreach (ROOT_AE3_EwoPack p : m_aPacks)
			{
				if (p.m_iIndex == n)
				{
					used = true;
					break;
				}
			}

			if (!used)
				return n;
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected void Tick()
	{
		ROOT_AE3_Settings s = ROOT_AE3_SettingsManager.Get();
		map<ROOT_AE3_PowerComponent, float> loads = new map<ROOT_AE3_PowerComponent, float>();
		for (int i = m_aPacks.Count() - 1; i >= 0; i--)
		{
			ROOT_AE3_EwoPack p = m_aPacks[i];
			if (!p.m_Pack || ROOT_AE3_Util.IsDestroyed(p.m_Pack))
			{
				Stop(p);
				m_aPacks.Remove(i);
				continue;
			}

			if (!s.m_bEwoMode)
			{
				Stop(p);
				continue;
			}

			Update(p, s, loads);
		}

		ApplyLoads(loads);
		UpdateWatchers(s);
	}

	//------------------------------------------------------------------------------------------------
	//! EWO mode off or pack gone: network, source and charging stop; energy stays.
	protected void Stop(ROOT_AE3_EwoPack p)
	{
		p.m_bWifi = false;
		p.m_Source = null;
		p.m_aCharging.Clear();
		DeleteRouter(p);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the EWO recharge load of every source; sources no pack uses any more get none.
	protected void ApplyLoads(map<ROOT_AE3_PowerComponent, float> loads)
	{
		foreach (ROOT_AE3_PowerComponent old : m_aLoaded)
		{
			if (old && !loads.Contains(old))
				old.SetExternalLoad(0);
		}

		m_aLoaded.Clear();
		foreach (ROOT_AE3_PowerComponent src, float w : loads)
		{
			if (!src)
				continue;

			src.SetExternalLoad(w);
			m_aLoaded.Insert(src);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Update(ROOT_AE3_EwoPack p, ROOT_AE3_Settings s, map<ROOT_AE3_PowerComponent, float> loads)
	{
		float max = s.m_fEwoEnergyMax;
		vector pos = ROOT_AE3_Util.WorldPos(p.m_Pack);
		IEntity carrier = ROOT_AE3_Util.Carrier(p.m_Pack);
		int carrierId = PlayerOf(carrier);
		int notifyId = carrierId;
		if (notifyId <= 0)
			notifyId = WatcherOf(p.m_Pack);

		// Recharge from a running power source in range: the source carries 60 W per energy per minute.
		if (p.m_Source)
		{
			IEntity src = p.m_Source.GetOwner();
			bool sourceOk = src && p.m_Source.IsOn() && !ROOT_AE3_Util.IsCarried(src) && vector.Distance(pos, ROOT_AE3_Util.WorldPos(src)) <= s.m_fEwoSourceRange;
			if (sourceOk)
			{
				if (p.m_fEnergy < max)
				{
					p.m_fEnergy = Math.Min(max, p.m_fEnergy + s.m_fEwoRechargeRate * TICK / 60 * p.m_Source.GetExternalFactor());
					loads.Set(p.m_Source, loads.Get(p.m_Source) + s.m_fEwoRechargeRate * 60);
				}
			}
			else
			{
				p.m_Source = null;
				Tell(notifyId, "The EWO pack was unplugged from its power source.");
			}
		}

		// Wireless network.
		if (p.m_bWifi)
		{
			p.m_fEnergy = Math.Max(0, p.m_fEnergy - s.m_fEwoWifiDrain * TICK / 60);
			if (p.m_fEnergy <= 0)
			{
				p.m_bWifi = false;
				Tell(notifyId, "The EWO pack is empty: " + p.m_sSsid + " stopped broadcasting.");
			}
		}

		SyncRouter(p, pos, s, notifyId);

		// Laptop charging.
		float step = s.m_fEwoChargeRate * TICK / 60;
		for (int i = p.m_aCharging.Count() - 1; i >= 0; i--)
		{
			IEntity laptop = p.m_aCharging[i];
			ROOT_AE3_PowerComponent lp = ROOT_AE3_PowerComponent.Of(laptop);
			ROOT_AE3_ComputerComponent lc = ROOT_AE3_ComputerComponent.Of(laptop);
			string why;
			if (!laptop || !lp)
				why = "removed";
			else if (!carrier || !ROOT_AE3_Util.CarriedBy(laptop, carrier))
				why = "left the inventory";
			else if (lc && ROOT_AE3_Manager.Get().GetSessionOf(lc))
				why = "is in use";
			else if (lp.GetLevel() >= 1)
				why = "is full";
			else if (p.m_fEnergy <= 0)
				why = "stopped: the pack is empty";

			if (!why.IsEmpty())
			{
				p.m_aCharging.Remove(i);
				if (laptop)
					Tell(notifyId, Label(laptop) + " charging ended: " + why + ".");

				continue;
			}

			float add = Math.Min(step, Math.Min(p.m_fEnergy, (1 - lp.GetLevel()) * 100));
			lp.SetLevel(lp.GetLevel() + add / 100);
			p.m_fEnergy -= add;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Router exists only while the network runs; name and password are applied when they change.
	protected void SyncRouter(ROOT_AE3_EwoPack p, vector pos, ROOT_AE3_Settings s, int notifyId)
	{
		if (!p.m_bWifi)
		{
			DeleteRouter(p);
			return;
		}

		if (!p.m_Router && !SpawnRouter(p, pos))
		{
			p.m_bWifi = false;
			Tell(notifyId, "No free EWO network number: the network did not start.");
			return;
		}

		p.m_Router.SetOrigin(pos + "0 1 0");
		ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(p.m_Router);
		if (n && (p.m_sSsid != p.m_sAppliedSsid || p.m_sPassword != p.m_sAppliedPassword))
		{
			n.ConfigureRouter(p.m_sSsid, p.m_sPassword, s.m_fEwoWifiRange, 1, true, true, -1);
			p.m_sAppliedSsid = p.m_sSsid;
			p.m_sAppliedPassword = p.m_sPassword;
		}

		ROOT_AE3_PowerComponent rp = ROOT_AE3_PowerComponent.Of(p.m_Router);
		string reason;
		if (rp && !rp.IsOn())
			rp.TurnOn(reason);
	}

	//------------------------------------------------------------------------------------------------
	protected bool SpawnRouter(ROOT_AE3_EwoPack p, vector pos)
	{
		int index = FreeIndex();
		if (index <= 0)
			return false;

		Resource res = Resource.Load(ROUTER_PREFAB);
		if (!res || !res.IsValid())
		{
			Print("[AE3] EWO router prefab missing: " + ROUTER_PREFAB, LogLevel.WARNING);
			return false;
		}

		EntitySpawnParams sp = new EntitySpawnParams();
		sp.TransformMode = ETransformMode.WORLD;
		sp.Transform[3] = pos + "0 1 0";
		p.m_Router = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), sp);
		if (!p.m_Router)
			return false;

		p.m_iIndex = index;
		p.m_sAppliedSsid = "";
		p.m_sAppliedPassword = "";
		ROOT_AE3_DeviceComponent d = ROOT_AE3_DeviceComponent.Of(p.m_Router);
		if (d)
			d.SetDeviceName(p.m_sSsid);

		ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(p.m_Router);
		if (!n)
			return true;

		array<string> allow = {SUBNET_ALLOW};
		n.SetExternalAllow(allow);
		ROOT_AE3_NetworkSystem.Get().Register(n);
		ROOT_AE3_NetworkSystem.Get().ReassignGateway(n, "77.95." + index.ToString() + ".1");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteRouter(ROOT_AE3_EwoPack p)
	{
		if (p.m_Router)
			SCR_EntityHelper.DeleteEntityAndChildren(p.m_Router);

		p.m_Router = null;
		p.m_iIndex = 0;
	}

	//------------------------------------------------------------------------------------------------
	protected static int PlayerOf(IEntity character)
	{
		if (!character)
			return 0;

		return GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(character);
	}

	//------------------------------------------------------------------------------------------------
	//! A player with an open panel of the pack, 0 when none.
	protected int WatcherOf(IEntity pack)
	{
		foreach (int playerId, IEntity watched : m_mWatchers)
		{
			if (watched == pack)
				return playerId;
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Open panels get the state every tick; out of reach, dead players and EWO mode off close them.
	protected void UpdateWatchers(ROOT_AE3_Settings s)
	{
		array<int> closed = {};
		foreach (int playerId, IEntity pack : m_mWatchers)
		{
			IEntity character = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			ROOT_AE3_EwoPack p = Find(pack);
			if (!s.m_bEwoMode || !p || !Usable(character) || !InReach(pack, character))
			{
				closed.Insert(playerId);
				continue;
			}

			ROOT_AE3_Net.SendToPlayer(playerId, ROOT_AE3_EMsg.EWO_STATE, PackState(p, character));
		}

		foreach (int playerId : closed)
		{
			m_mWatchers.Remove(playerId);
			ROOT_AE3_Net.SendToPlayer(playerId, ROOT_AE3_EMsg.EWO_STATE, "");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Usable(IEntity character)
	{
		ChimeraCharacter cc = ChimeraCharacter.Cast(character);
		return cc && cc.GetCharacterController() && !cc.GetCharacterController().IsDead();
	}

	//------------------------------------------------------------------------------------------------
	protected static string Label(IEntity laptop)
	{
		ROOT_AE3_DeviceComponent d = ROOT_AE3_DeviceComponent.Of(laptop);
		if (d)
			return d.GetDeviceName();

		return "Laptop";
	}

	//------------------------------------------------------------------------------------------------
	protected static void Tell(int playerId, string text)
	{
		if (playerId <= 0)
			return;

		array<string> f = {"EWO", text};
		ROOT_AE3_Net.SendToPlayer(playerId, ROOT_AE3_EMsg.NOTIFY, ROOT_AE3_Codec.Join(f));
	}

	//------------------------------------------------------------------------------------------------
	//! Server: panel request for the pack packId (invalid = the worn pack). data: command, args (ROOT_AE3_Codec).
	//! state | close | wifi 0/1 | net ssid password | charge laptopRplId 0/1 | source 0/1
	void OnRequest(int playerId, RplId packId, string data)
	{
		array<string> a = ROOT_AE3_Codec.Split(data);
		string cmd;
		if (!a.IsEmpty())
			cmd = a[0];

		if (cmd == "close")
		{
			m_mWatchers.Remove(playerId);
			return;
		}

		IEntity character = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		IEntity pack;
		RplComponent rpl = RplComponent.Cast(Replication.FindItem(packId));
		if (rpl)
			pack = rpl.GetEntity();
		else
			pack = WornPack(character);

		if (!IsEwoPack(pack) || !Usable(character) || !InReach(pack, character))
		{
			Tell(playerId, "The EWO pack is out of reach (EWO mode must be on).");
			m_mWatchers.Remove(playerId);
			ROOT_AE3_Net.SendToPlayer(playerId, ROOT_AE3_EMsg.EWO_STATE, "");
			return;
		}

		ROOT_AE3_EwoPack p = Ensure(pack);
		m_mWatchers.Set(playerId, pack);
		ROOT_AE3_Settings s = ROOT_AE3_SettingsManager.Get();
		switch (cmd)
		{
			case "wifi":
			{
				p.m_bWifi = a.Count() > 1 && a[1] == "1" && p.m_fEnergy > 0;
				SyncRouter(p, ROOT_AE3_Util.WorldPos(pack), s, playerId);
				break;
			}

			case "net":
			{
				if (a.Count() > 1 && !a[1].IsEmpty())
					p.m_sSsid = Clip(a[1], 24);

				if (a.Count() > 2 && !a[2].IsEmpty())
					p.m_sPassword = Clip(a[2], 24);

				if (p.m_Router)
				{
					ROOT_AE3_DeviceComponent d = ROOT_AE3_DeviceComponent.Of(p.m_Router);
					if (d)
						d.SetDeviceName(p.m_sSsid);
				}

				SyncRouter(p, ROOT_AE3_Util.WorldPos(pack), s, playerId);
				break;
			}

			case "charge":
			{
				if (a.Count() < 3)
					break;

				ROOT_AE3_DeviceComponent ld = ROOT_AE3_DeviceComponent.FromRplId(a[1].ToInt());
				IEntity laptop;
				if (ld)
					laptop = ld.GetOwner();

				bool on = a[2] == "1";
				if (!on)
				{
					p.m_aCharging.RemoveItem(laptop);
					break;
				}

				ROOT_AE3_PowerComponent lp = ROOT_AE3_PowerComponent.Of(laptop);
				if (!ROOT_AE3_Util.CarriedBy(pack, character))
					Tell(playerId, "Carry the EWO pack to charge laptops.");
				else if (!laptop || !lp || !ROOT_AE3_ComputerComponent.Of(laptop) || !ROOT_AE3_Util.CarriedBy(laptop, character))
					Tell(playerId, "Only laptops in your inventory can charge.");
				else if (lp.GetCapacityKWh() <= 0 || lp.GetLevel() >= 1)
					Tell(playerId, Label(laptop) + " is full.");
				else if (p.m_fEnergy <= 0)
					Tell(playerId, "The EWO pack is empty.");
				else if (!p.m_aCharging.Contains(laptop))
					p.m_aCharging.Insert(laptop);

				break;
			}

			case "source":
			{
				p.m_Source = null;
				if (a.Count() > 1 && a[1] == "1")
				{
					p.m_Source = NearestSource(ROOT_AE3_Util.WorldPos(pack), s.m_fEwoSourceRange);
					if (!p.m_Source)
						Tell(playerId, "No running generator, battery or solar panel within " + s.m_fEwoSourceRange.ToString() + " m.");
				}

				break;
			}
		}

		ROOT_AE3_Net.SendToPlayer(playerId, ROOT_AE3_EMsg.EWO_STATE, PackState(p, character));
	}

	//------------------------------------------------------------------------------------------------
	protected static string Clip(string s, int n)
	{
		string res;
		for (int i = 0; i < s.Length() && res.Length() < n; i++)
		{
			int c = s.ToAscii(i);
			if (c >= 32 && c < 127 && c != 124)
				res += s.Get(i);
		}

		return res;
	}

	//------------------------------------------------------------------------------------------------
	protected ROOT_AE3_PowerComponent NearestSource(vector pos, float range)
	{
		ROOT_AE3_PowerComponent best;
		float bestDist = range;
		foreach (ROOT_AE3_DeviceComponent d : ROOT_AE3_Manager.Get().GetDevices())
		{
			if (!d)
				continue;

			ROOT_AE3_PowerComponent pc = ROOT_AE3_PowerComponent.Of(d.GetOwner());
			if (!pc || pc.GetRole() == ROOT_AE3_EPowerRole.CONSUMER || !pc.IsOn() || ROOT_AE3_Util.IsCarried(d.GetOwner()))
				continue;

			float dist = vector.Distance(pos, ROOT_AE3_Util.WorldPos(d.GetOwner()));
			if (dist <= bestDist)
			{
				bestDist = dist;
				best = pc;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! energy, max, wifi, ssid, password, gateway, source name, laptops (rplId, name, percent, charging)...
	protected string PackState(ROOT_AE3_EwoPack p, IEntity character)
	{
		ROOT_AE3_Settings s = ROOT_AE3_SettingsManager.Get();
		string source;
		if (p.m_Source)
		{
			ROOT_AE3_DeviceComponent sd = ROOT_AE3_DeviceComponent.Of(p.m_Source.GetOwner());
			if (sd)
				source = sd.GetDeviceName();
		}

		string gateway;
		if (p.m_iIndex > 0)
			gateway = "77.95." + p.m_iIndex.ToString() + ".1";

		array<string> f = {Math.Round(p.m_fEnergy).ToString(), Math.Round(s.m_fEwoEnergyMax).ToString(), ROOT_AE3_Util.BoolStr(p.m_bWifi),
			p.m_sSsid, p.m_sPassword, gateway, source};
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (inv && ROOT_AE3_Util.CarriedBy(p.m_Pack, character))
		{
			array<IEntity> items = {};
			inv.GetItems(items);
			foreach (IEntity item : items)
			{
				ROOT_AE3_PowerComponent lp = ROOT_AE3_PowerComponent.Of(item);
				ROOT_AE3_DeviceComponent ld = ROOT_AE3_DeviceComponent.Of(item);
				if (!lp || !ld || !ROOT_AE3_ComputerComponent.Of(item))
					continue;

				int id = ld.GetRplId();
				f.Insert(id.ToString());
				f.Insert(ld.GetDeviceName());
				f.Insert(Math.Round(lp.GetLevel() * 100).ToString());
				f.Insert(ROOT_AE3_Util.BoolStr(p.m_aCharging.Contains(item)));
			}
		}

		return ROOT_AE3_Codec.Join(f);
	}
}
