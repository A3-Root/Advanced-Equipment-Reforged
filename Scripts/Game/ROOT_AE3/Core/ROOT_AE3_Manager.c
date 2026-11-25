//! Device registry and server-side message routing. One instance per game.
//! Server: sessions, power and network simulation start here. Clients: registry only.
class ROOT_AE3_Manager
{
	protected static ref ROOT_AE3_Manager s_Instance;

	protected ref array<ROOT_AE3_DeviceComponent> m_aDevices = {};
	protected ref map<int, ref ROOT_AE3_Session> m_mSessions = new map<int, ref ROOT_AE3_Session>();
	protected ref map<int, string> m_mChunks = new map<int, string>();
	protected ref array<int> m_aChunkDropped = {};
	protected ref map<int, string> m_mDroppedHead = new map<int, string>();	//!< Start of a dropped request (request id, command)
	protected ref map<int, float> m_mLastOpen = new map<int, float>();
	protected bool m_bSystemsStarted;
	protected ref map<int, int> m_mRateCount = new map<int, int>();
	protected ref map<string, float> m_mAuthUntil = new map<string, float>();
	protected ref map<string, int> m_mAuthFails = new map<string, int>();
	protected float m_fRateWindow;
	protected static bool s_bEnding;
	static const int RATE_LIMIT = 60;	//!< messages per player and second
	static const int MAX_LINE = 1024;	//!< Characters of a terminal line, key or game move
	static const int MAX_PAYLOAD = ROOT_AE3_Filesystem.MAX_FILE + 4096;	//!< Largest file plus request fields
	static const float USE_RANGE = 4;

	protected ref ScriptInvokerBase<ROOT_AE3_DeviceInvoker> m_OnDeviceReady;

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_Manager Get()
	{
		if (!s_Instance)
			s_Instance = new ROOT_AE3_Manager();

		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	//! Drops all state (game end).
	static void Reset()
	{
		if (s_Instance)
			GetGame().GetCallqueue().Remove(s_Instance.CheckSessions);

		s_Instance = null;
	}

	//------------------------------------------------------------------------------------------------
	static void Register(ROOT_AE3_DeviceComponent device)
	{
		s_bEnding = false;
		ROOT_AE3_Manager m = Get();
		if (!m.m_aDevices.Contains(device))
			m.m_aDevices.Insert(device);

		if (Replication.IsServer())
			m.StartSystems();
	}

	//------------------------------------------------------------------------------------------------
	static void Unregister(ROOT_AE3_DeviceComponent device)
	{
		if (!s_Instance)
			return;

		s_Instance.m_aDevices.RemoveItem(device);
		if (Replication.IsServer())
			s_Instance.EndSessionsOf(device);
	}

	//------------------------------------------------------------------------------------------------
	//! True while the game is ending (world teardown): components skip side effects.
	static bool IsEnding()
	{
		return s_bEnding;
	}

	//------------------------------------------------------------------------------------------------
	static void SetEnding()
	{
		s_bEnding = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Failure reply to a desktop request that was dropped; head holds its request id and command.
	protected void ReplyFailed(int playerId, string head, string reason)
	{
		array<string> f = ROOT_AE3_Codec.Split(head);
		if (f.Count() < 2)
			return;

		array<string> reply = {f[0], "0", f[1], reason};
		ROOT_AE3_Net.SendToPlayer(playerId, ROOT_AE3_EMsg.GUI_REPLY, ROOT_AE3_Codec.Join(reply));
	}

	//------------------------------------------------------------------------------------------------
	//! Password attempts: while a player is delayed every attempt fails.
	bool CheckAuth(int playerId, bool ok)
	{
		string key = playerId.ToString();
		UUID uid = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		if (!uid.IsNull())
			key = uid;

		float now = GetGame().GetWorld().GetWorldTime() / 1000.0;
		if (now < m_mAuthUntil.Get(key))
			return false;

		if (ok)
		{
			m_mAuthFails.Remove(key);
			return true;
		}

		int fails = m_mAuthFails.Get(key) + 1;
		m_mAuthFails.Set(key, fails);
		m_mAuthUntil.Set(key, now + Math.Min(fails, 5));
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Simple per-player message budget per second.
	protected bool RateOk(int playerId)
	{
		float now = GetGame().GetWorld().GetWorldTime() / 1000.0;
		if (now - m_fRateWindow >= 1)
		{
			m_fRateWindow = now;
			m_mRateCount.Clear();
		}

		int n = m_mRateCount.Get(playerId) + 1;
		m_mRateCount.Set(playerId, n);
		return n <= RATE_LIMIT;
	}

	//------------------------------------------------------------------------------------------------
	//! Ends sessions of players who walked away, died or lost their character.
	protected void CheckSessions()
	{
		array<int> ended = {};
		array<string> reasons = {};
		for (int i = 0, n = m_mSessions.Count(); i < n; i++)
		{
			ROOT_AE3_Session s = m_mSessions.GetElement(i);
			if (s && ROOT_AE3_Net.TakeDropped(m_mSessions.GetKey(i)))
				s.Resync();

			string reason;
			if (s && !s.PlayerValid(reason))
			{
				ended.Insert(m_mSessions.GetKey(i));
				reasons.Insert(reason);
			}
		}

		foreach (int k, int id : ended)
		{
			EndSession(id, reasons[k]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Fired on the server when a computer finished its setup. Param: device.
	ScriptInvokerBase<ROOT_AE3_DeviceInvoker> GetOnDeviceReady()
	{
		if (!m_OnDeviceReady)
			m_OnDeviceReady = new ScriptInvokerBase<ROOT_AE3_DeviceInvoker>();

		return m_OnDeviceReady;
	}

	//------------------------------------------------------------------------------------------------
	void NotifyDeviceReady(ROOT_AE3_DeviceComponent device, bool restored = false)
	{
		if (m_OnDeviceReady)
			m_OnDeviceReady.Invoke(device, restored);
	}

	//------------------------------------------------------------------------------------------------
	array<ROOT_AE3_DeviceComponent> GetDevices()
	{
		return m_aDevices;
	}

	//------------------------------------------------------------------------------------------------
	void GetDevicesOfType(ROOT_AE3_EDeviceType type, notnull array<ROOT_AE3_DeviceComponent> outDevices)
	{
		foreach (ROOT_AE3_DeviceComponent d : m_aDevices)
		{
			if (d && d.GetDeviceType() == type)
				outDevices.Insert(d);
		}
	}

	//------------------------------------------------------------------------------------------------
	void GetComputers(notnull array<ROOT_AE3_ComputerComponent> outComputers)
	{
		foreach (ROOT_AE3_DeviceComponent d : m_aDevices)
		{
			if (!d)
				continue;

			ROOT_AE3_ComputerComponent c = ROOT_AE3_ComputerComponent.Of(d.GetOwner());
			if (c)
				outComputers.Insert(c);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void StartSystems()
	{
		if (m_bSystemsStarted)
			return;

		m_bSystemsStarted = true;
		ROOT_AE3_Intel.EnsureHomePage();
		GetGame().GetCallqueue().CallLater(CheckSessions, 2000, true);
		ROOT_AE3_PowerSystem.Get().Start();
		ROOT_AE3_NetworkSystem.Get().Start();
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AE3_Session GetSession(int playerId)
	{
		return m_mSessions.Get(playerId);
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AE3_Session GetSessionOf(ROOT_AE3_ComputerComponent computer)
	{
		for (int i = 0, n = m_mSessions.Count(); i < n; i++)
		{
			ROOT_AE3_Session s = m_mSessions.GetElement(i);
			if (s && s.m_Computer == computer)
				return s;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	void EndSessionsOf(ROOT_AE3_DeviceComponent device)
	{
		array<int> ids = {};
		for (int i = 0, n = m_mSessions.Count(); i < n; i++)
		{
			ROOT_AE3_Session s = m_mSessions.GetElement(i);
			if (s && s.m_Computer && s.m_Computer.GetOwner() == device.GetOwner())
				ids.Insert(m_mSessions.GetKey(i));
		}

		foreach (int id : ids)
		{
			EndSession(id, "The device is no longer available.");
		}
	}

	//------------------------------------------------------------------------------------------------
	void EndSession(int playerId, string reason)
	{
		ROOT_AE3_Session s = m_mSessions.Get(playerId);
		if (!s)
			return;

		s.Close();
		m_mSessions.Remove(playerId);
		ROOT_AE3_Net.SendToPlayer(playerId, ROOT_AE3_EMsg.ENDED, reason);
	}

	//------------------------------------------------------------------------------------------------
	//! Server: message from a client (or from the local player in SP / listen host).
	void OnClientMessage(int playerId, RplId deviceId, int op, string data)
	{
		if (op == ROOT_AE3_EMsg.CHUNK)
		{
			if (!RateOk(playerId))
			{
				m_mChunks.Remove(playerId);
				if (!m_aChunkDropped.Contains(playerId))
					m_aChunkDropped.Insert(playerId);

				return;
			}

			if (m_aChunkDropped.Contains(playerId))
				return;

			string buf = m_mChunks.Get(playerId) + data;
			if (buf.Length() > MAX_PAYLOAD)
			{
				m_mDroppedHead.Set(playerId, buf.Substring(0, Math.Min(buf.Length(), 256)));
				m_mChunks.Remove(playerId);
				m_aChunkDropped.Insert(playerId);
				return;
			}

			m_mChunks.Set(playerId, buf);
			return;
		}

		if (m_aChunkDropped.Contains(playerId))
		{
			m_aChunkDropped.RemoveItem(playerId);
			string head = m_mDroppedHead.Get(playerId);
			m_mDroppedHead.Remove(playerId);
			if (op == ROOT_AE3_EMsg.GUI)
				ReplyFailed(playerId, head, "Request too large.");

			return;
		}

		if (m_mChunks.Contains(playerId))
		{
			data = m_mChunks.Get(playerId) + data;
			m_mChunks.Remove(playerId);
		}

		if (data.Length() > MAX_PAYLOAD || !RateOk(playerId))
		{
			if (op == ROOT_AE3_EMsg.GUI)
			{
				string why = "Too many requests, try again.";
				if (data.Length() > MAX_PAYLOAD)
					why = "Request too large.";

				ReplyFailed(playerId, data.Substring(0, Math.Min(data.Length(), 256)), why);
			}

			return;
		}

		if ((op == ROOT_AE3_EMsg.TERM_LINE || op == ROOT_AE3_EMsg.TERM_TAB || op == ROOT_AE3_EMsg.TERM_KEY || op == ROOT_AE3_EMsg.GAME_INPUT) && data.Length() > MAX_LINE)
			data = data.Substring(0, MAX_LINE);

		if (op == ROOT_AE3_EMsg.SETTINGS)
		{
			ROOT_AE3_Net.SendToPlayer(playerId, ROOT_AE3_EMsg.SETTINGS, ROOT_AE3_SettingsManager.Get().PackClient());
			return;
		}

		if (op == ROOT_AE3_EMsg.OPEN)
		{
			float nowOpen = GetGame().GetWorld().GetWorldTime() / 1000.0;
			if (nowOpen - m_mLastOpen.Get(playerId) < 1)
				return;

			m_mLastOpen.Set(playerId, nowOpen);
			OpenSession(playerId, deviceId, Math.ClampInt(data.ToInt(), 0, 1));
			return;
		}

		ROOT_AE3_Session s = m_mSessions.Get(playerId);
		if (!s)
			return;

		if (op == ROOT_AE3_EMsg.CLOSE)
		{
			s.Close();
			m_mSessions.Remove(playerId);
			return;
		}

		string reason;
		if (!s.IsUsable() || !s.PlayerValid(reason))
		{
			if (reason.IsEmpty())
				reason = "The device is no longer available.";

			EndSession(playerId, reason);
			return;
		}

		bool terminal = s.m_iMode == 0 && s.HasAccess(0);
		bool desktop = s.m_iMode == 1 && s.HasAccess(1);
		switch (op)
		{
			case ROOT_AE3_EMsg.TERM_LINE:
			{
				if (terminal)
					s.OnLine(data);

				break;
			}

			case ROOT_AE3_EMsg.TERM_TAB:
			{
				if (terminal)
					s.OnTab(data);

				break;
			}

			case ROOT_AE3_EMsg.TERM_KEY: s.OnKey(data); break;
			case ROOT_AE3_EMsg.GUI: ROOT_AE3_GuiRouter.Handle(s, data); break;
			case ROOT_AE3_EMsg.GAME_INPUT:
			{
				if (desktop)
					ROOT_AE3_GameTableSystem.Get().OnInput(s, data);

				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenSession(int playerId, RplId deviceId, int mode)
	{
		ROOT_AE3_DeviceComponent dev = ROOT_AE3_DeviceComponent.FromRplId(deviceId);
		if (!dev)
			return;

		ROOT_AE3_ComputerComponent computer = ROOT_AE3_ComputerComponent.Of(dev.GetOwner());
		if (!computer)
			return;

		string reason;
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		ROOT_AE3_LidComponent lid = ROOT_AE3_LidComponent.Of(dev.GetOwner());
		ChimeraCharacter character = ChimeraCharacter.Cast(player);
		bool dead = character && character.GetCharacterController() && character.GetCharacterController().IsDead();
		if (!player || dead || ROOT_AE3_Util.IsDestroyed(dev.GetOwner()) || ROOT_AE3_Util.IsCarried(dev.GetOwner()) || vector.Distance(player.GetOrigin(), dev.GetOwner().GetOrigin()) > USE_RANGE || (lid && !lid.IsOpen()))
		{
			ROOT_AE3_Net.SendToPlayer(playerId, ROOT_AE3_EMsg.ENDED, "The computer is out of reach.");
			return;
		}

		if (!computer.CanOpen(playerId, reason))
		{
			ROOT_AE3_Net.SendToPlayer(playerId, ROOT_AE3_EMsg.ENDED, reason);
			return;
		}

		ROOT_AE3_Session old = m_mSessions.Get(playerId);
		if (old)
		{
			old.Close();
			m_mSessions.Remove(playerId);
		}

		ROOT_AE3_Session s = new ROOT_AE3_Session(playerId, computer);
		if (!s.Start(mode, false, reason))
		{
			ROOT_AE3_Net.SendToPlayer(playerId, ROOT_AE3_EMsg.ENDED, reason);
			return;
		}

		m_mSessions.Set(playerId, s);
	}

	//------------------------------------------------------------------------------------------------
	//! Server: Game Master access to a running computer, logged in as root.
	void OpenAsRoot(int playerId, ROOT_AE3_ComputerComponent computer)
	{
		if (!computer)
			return;

		string reason;
		if (!computer.CanOpen(playerId, reason))
		{
			ROOT_AE3_Net.SendToPlayer(playerId, ROOT_AE3_EMsg.ENDED, reason);
			return;
		}

		ROOT_AE3_Session old = m_mSessions.Get(playerId);
		if (old)
		{
			old.Close();
			m_mSessions.Remove(playerId);
		}

		ROOT_AE3_Session s = new ROOT_AE3_Session(playerId, computer);
		ROOT_AE3_Shell.WriteLog(computer, "/var/log/auth.log", "Game Master access as root");
		if (s.Start(1, true, reason))
			m_mSessions.Set(playerId, s);
		else
			ROOT_AE3_Net.SendToPlayer(playerId, ROOT_AE3_EMsg.ENDED, reason);
	}

	//------------------------------------------------------------------------------------------------
	//! Server: player left.
	void OnPlayerDisconnected(int playerId)
	{
		ROOT_AE3_Session s = m_mSessions.Get(playerId);
		if (s)
		{
			s.Close();
			m_mSessions.Remove(playerId);
		}

		m_mChunks.Remove(playerId);
		m_aChunkDropped.RemoveItem(playerId);
		m_mRateCount.Remove(playerId);
		m_mLastOpen.Remove(playerId);
		m_mDroppedHead.Remove(playerId);
	}
}

void ROOT_AE3_DeviceInvoker(ROOT_AE3_DeviceComponent device, bool restored);
typedef func ROOT_AE3_DeviceInvoker;
