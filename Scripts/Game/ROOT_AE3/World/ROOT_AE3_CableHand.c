//! Cable end held by a player between the two plug actions (ROOT_AE3_ConnectNetworkAction, ROOT_AE3_ConnectPowerAction).
//! Network: plugged into a device first (cable item carried), then into the other end; computer to router or router to
//! router (the first router becomes the client of the second). Power: taken from a source (generator, battery, solar
//! panel; cable length ROOT_AE3_PowerComponent.m_fCableLength), then plugged into a device that uses power.
//! The second plug fails when the devices are farther apart than the cable; the cable stays in the hand.
//! Server: one held cable per player, checked every second: dropped when the player dies or leaves, walks farther than
//! the cable from the first device, the first device is picked up, or the network cable item is gone.
//! Client: the local player's held cable (ROOT_AE3_EMsg.CABLE_HAND), drawn from the first device to the right hand.
class ROOT_AE3_HeldCable
{
	int m_iKind;
	IEntity m_Device;
	float m_fLength;
}

class ROOT_AE3_CableHand
{
	static const int NONE = 0;
	static const int NET = 1;
	static const int POWER = 2;

	protected static ref map<int, ref ROOT_AE3_HeldCable> s_mHeld = new map<int, ref ROOT_AE3_HeldCable>();
	protected static bool s_bTicking;

	protected static int s_iKind;
	protected static RplId s_DeviceId = RplId.Invalid();
	protected static float s_fLength;
	protected static ref ROOT_AE3_CableView s_View;

	//------------------------------------------------------------------------------------------------
	static void Reset()
	{
		s_mHeld.Clear();
		s_bTicking = false;
		GetGame().GetCallqueue().Remove(Tick);
		GetGame().GetCallqueue().Remove(UpdateView);
		s_iKind = NONE;
		s_DeviceId = RplId.Invalid();
		s_View = null;
	}

	// Rules (server and client) --------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Empty when the cable from first can be plugged into second. Network: child is the device that gets the
	//! cable (computer, or the first router), parent the router. Power: child the consumer, parent the source.
	static string Check(int kind, IEntity first, IEntity second, float length, out IEntity child, out IEntity parent)
	{
		if (!first || !second || first == second)
			return "Plug the other end into another device.";

		if (ROOT_AE3_Util.IsCarried(second) || ROOT_AE3_Util.IsDestroyed(second))
			return "That device is not available.";

		if (kind == NET)
		{
			ROOT_AE3_NetworkComponent n1 = ROOT_AE3_NetworkComponent.Of(first);
			ROOT_AE3_NetworkComponent n2 = ROOT_AE3_NetworkComponent.Of(second);
			if (!n1 || !n2)
				return "That device has no network port.";

			if (!n1.IsRouter() && !n2.IsRouter())
				return "Plug the other end into a router.";

			child = first;
			parent = second;
			if (n1.IsRouter() && !n2.IsRouter())
			{
				child = second;
				parent = first;
			}

			ROOT_AE3_ConnectionComponent link = ROOT_AE3_ConnectionComponent.Of(child);
			if (link && link.GetNetPeer() != RplId.Invalid())
				return Name(child) + " already has a network cable.";
		}
		else
		{
			ROOT_AE3_PowerComponent src = ROOT_AE3_PowerComponent.Of(first);
			ROOT_AE3_PowerComponent c = ROOT_AE3_PowerComponent.Of(second);
			if (!src || !src.IsProvider() || !c || c.IsMains() || (c.GetRole() != ROOT_AE3_EPowerRole.CONSUMER && c.GetRole() != ROOT_AE3_EPowerRole.BATTERY))
				return "Plug the power cable into a device that uses power.";

			if (c.HasProvider())
				return Name(second) + " is already plugged into a power source.";

			child = second;
			parent = first;
		}

		float dist = vector.Distance(child.GetOrigin(), parent.GetOrigin());
		if (dist > length)
			return string.Format("The cable is too short: %1 m needed, %2 m cable.", Math.Ceil(dist), length);

		return "";
	}

	//------------------------------------------------------------------------------------------------
	static string Name(IEntity ent)
	{
		ROOT_AE3_DeviceComponent d = ROOT_AE3_DeviceComponent.Of(ent);
		if (d)
			return d.GetDeviceName();

		return "The device";
	}

	// Server ---------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_HeldCable GetHeld(int playerId)
	{
		return s_mHeld.Get(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Server: first plug. Network: the longest carried network cable; power: the cable of the source.
	static bool Take(IEntity character, int kind, IEntity device, out string reason)
	{
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(character);
		if (playerId <= 0 || !device)
			return false;

		ROOT_AE3_HeldCable held = new ROOT_AE3_HeldCable();
		held.m_iKind = kind;
		held.m_Device = device;
		if (kind == NET)
		{
			IEntity item = ROOT_AE3_CableComponent.FindCarried(character, 0);
			if (!item)
			{
				reason = "No network cable in your inventory.";
				return false;
			}

			held.m_fLength = ROOT_AE3_CableComponent.Of(item).GetLength();
			ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(device);
			ROOT_AE3_ConnectionComponent link = ROOT_AE3_ConnectionComponent.Of(device);
			if (!n || (!n.IsRouter() && link && link.GetNetPeer() != RplId.Invalid()))
			{
				reason = Name(device) + " already has a network cable.";
				return false;
			}
		}
		else
		{
			ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(device);
			if (!p || !p.IsProvider())
				return false;

			held.m_fLength = p.GetCableLength();
		}

		s_mHeld.Set(playerId, held);
		Send(playerId, held);
		if (!s_bTicking)
		{
			s_bTicking = true;
			GetGame().GetCallqueue().CallLater(Tick, 1000, true);
		}

		string what = "Network cable";
		string next = "a router or computer";
		if (kind == POWER)
		{
			what = "Power cable";
			next = "a device that uses power";
		}

		Notify(playerId, string.Format("%1 (%2 m) plugged into %3. Plug the other end into %4.", what, held.m_fLength, Name(device), next));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Server: second plug.
	static bool Plug(IEntity character, IEntity device, out string reason)
	{
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(character);
		ROOT_AE3_HeldCable held = s_mHeld.Get(playerId);
		if (!held)
			return false;

		IEntity child, parent;
		reason = Check(held.m_iKind, held.m_Device, device, held.m_fLength, child, parent);
		if (!reason.IsEmpty())
			return false;

		if (held.m_iKind == NET)
		{
			ROOT_AE3_NetworkComponent cn = ROOT_AE3_NetworkComponent.Of(child);
			ROOT_AE3_NetworkComponent pn = ROOT_AE3_NetworkComponent.Of(parent);
			if (!cn.IsRouter() && !pn.AcceptsConnections())
			{
				reason = "The router does not accept new connections.";
				return false;
			}

			IEntity item = ROOT_AE3_CableComponent.FindCarried(character, held.m_fLength);
			if (!item)
			{
				Drop(playerId, "The network cable is no longer in your inventory.");
				return false;
			}

			ResourceName prefab;
			EntityPrefabData pd = item.GetPrefabData();
			if (pd)
				prefab = pd.GetPrefabName();

			if (!ROOT_AE3_ConnectionComponent.ConnectNetwork(child, parent, reason, held.m_fLength, prefab))
				return false;

			ROOT_AE3_CableComponent.Consume(character, item);
		}
		else
		{
			ROOT_AE3_PowerComponent c = ROOT_AE3_PowerComponent.Of(child);
			if (!c.ConnectTo(ROOT_AE3_PowerComponent.Of(parent), reason, held.m_fLength))
				return false;
		}

		Drop(playerId, string.Format("%1 plugged into %2 (%3 m cable).", Name(child), Name(parent), held.m_fLength));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Server: removes the held cable; note goes to the player.
	static void Drop(int playerId, string note = "")
	{
		if (!s_mHeld.Contains(playerId))
			return;

		s_mHeld.Remove(playerId);
		Send(playerId, null);
		if (!note.IsEmpty())
			Notify(playerId, note);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Tick()
	{
		array<int> ids = {};
		foreach (int playerId, ROOT_AE3_HeldCable held : s_mHeld)
		{
			ids.Insert(playerId);
		}

		foreach (int playerId : ids)
		{
			ROOT_AE3_HeldCable held = s_mHeld.Get(playerId);
			IEntity character = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			ChimeraCharacter cc = ChimeraCharacter.Cast(character);
			if (!character || !held.m_Device || ROOT_AE3_Util.IsCarried(held.m_Device) || ROOT_AE3_Util.IsDestroyed(held.m_Device)
				|| (cc && cc.GetCharacterController() && cc.GetCharacterController().IsDead()))
			{
				Drop(playerId, "The cable came out.");
				continue;
			}

			if (vector.Distance(character.GetOrigin(), held.m_Device.GetOrigin()) > held.m_fLength + 1)
			{
				Drop(playerId, string.Format("The cable came out: more than %1 m from %2.", held.m_fLength, Name(held.m_Device)));
				continue;
			}

			if (held.m_iKind == NET && !ROOT_AE3_CableComponent.FindCarried(character, held.m_fLength))
				Drop(playerId, "The network cable is no longer in your inventory.");
		}

		if (s_mHeld.IsEmpty())
		{
			s_bTicking = false;
			GetGame().GetCallqueue().Remove(Tick);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void Send(int playerId, ROOT_AE3_HeldCable held)
	{
		array<string> f = {"0", "0", "0"};
		if (held)
		{
			ROOT_AE3_DeviceComponent d = ROOT_AE3_DeviceComponent.Of(held.m_Device);
			int rid;
			if (d)
				rid = d.GetRplId();

			f = {held.m_iKind.ToString(), rid.ToString(), held.m_fLength.ToString()};
		}

		ROOT_AE3_Net.SendToPlayer(playerId, ROOT_AE3_EMsg.CABLE_HAND, ROOT_AE3_Codec.Join(f));
	}

	//------------------------------------------------------------------------------------------------
	protected static void Notify(int playerId, string text)
	{
		array<string> f = {"Advanced Equipment", text};
		ROOT_AE3_Net.SendToPlayer(playerId, ROOT_AE3_EMsg.NOTIFY, ROOT_AE3_Codec.Join(f));
	}

	// Client ---------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Client: data kind, device RplId, length (kind 0: no cable).
	static void OnMessage(string data)
	{
		array<string> f = ROOT_AE3_Codec.Split(data);
		s_iKind = NONE;
		s_DeviceId = RplId.Invalid();
		if (f.Count() >= 3)
		{
			s_iKind = f[0].ToInt();
			s_DeviceId = f[1].ToInt();
			s_fLength = f[2].ToFloat();
		}

		GetGame().GetCallqueue().Remove(UpdateView);
		s_View = null;
		if (s_iKind != NONE && !System.IsConsoleApp())
			GetGame().GetCallqueue().CallLater(UpdateView, 50, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Client: kind of the cable in the local player's hand (NONE, NET, POWER).
	static int LocalKind()
	{
		if (s_iKind != NONE && !LocalDevice())
			return NONE;

		return s_iKind;
	}

	//------------------------------------------------------------------------------------------------
	static IEntity LocalDevice()
	{
		ROOT_AE3_DeviceComponent d = ROOT_AE3_DeviceComponent.FromRplId(s_DeviceId);
		if (!d)
			return null;

		return d.GetOwner();
	}

	//------------------------------------------------------------------------------------------------
	static float LocalLength()
	{
		return s_fLength;
	}

	//------------------------------------------------------------------------------------------------
	//! Client: cable from the first device to the right hand of the local character.
	protected static void UpdateView()
	{
		IEntity device = LocalDevice();
		IEntity character = SCR_PlayerController.GetLocalControlledEntity();
		ROOT_AE3_ConnectionComponent link = ROOT_AE3_ConnectionComponent.Of(device);
		if (!device || !character || !link)
		{
			s_View = null;
			return;
		}

		vector hand = character.GetOrigin() + character.VectorToParent("0.2 1.0 0.3");
		Animation anim = character.GetAnimation();
		if (anim)
		{
			TNodeId bone = anim.GetBoneIndex("righthandprop");
			vector mat[4];
			if (bone >= 0 && anim.GetBoneMatrix(bone, mat))
				hand = character.CoordToParent(mat[3]);
		}

		vector a, aDir;
		if (s_iKind == POWER)
			link.PowerPort(a, aDir);
		else
			link.NetPort(1, a, aDir);

		if (!s_View)
		{
			if (s_iKind == POWER)
				s_View = new ROOT_AE3_CableView(ROOT_AE3_ConnectionComponent.POWER_MATERIAL);
			else
				s_View = new ROOT_AE3_CableView(ROOT_AE3_ConnectionComponent.NET_MATERIAL);
		}

		float len = Math.Max(vector.Distance(a, hand) + 0.4, Math.Min(s_fLength, vector.Distance(a, hand) * 1.15 + 0.3));
		s_View.Update(a, aDir, hand, "0 -1 0", len);
	}
}
