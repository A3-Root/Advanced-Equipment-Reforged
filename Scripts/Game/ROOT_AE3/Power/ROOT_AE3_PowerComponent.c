enum ROOT_AE3_EPowerRole
{
	CONSUMER,
	GENERATOR,
	BATTERY,
	SOLAR
}

enum ROOT_AE3_EPowerState
{
	OFF,
	ON,
	STANDBY,
	CRASHED
}

[ComponentEditorProps(category: "Advanced Equipment", description: "Power consumer, generator, battery or solar panel")]
class ROOT_AE3_PowerComponentClass : ScriptComponentClass
{
}

//! Electrical model of a device. Simulated by ROOT_AE3_PowerSystem on the authority.
//! Replicated: state, battery level, fuel, external power flag, load.
class ROOT_AE3_PowerComponent : ScriptComponent
{
	[Attribute("0", uiwidget: UIWidgets.ComboBox, desc: "Role", enums: ParamEnumArray.FromEnum(ROOT_AE3_EPowerRole), category: "Power")]
	protected ROOT_AE3_EPowerRole m_eRole;

	[Attribute("10", desc: "Draw in W when on (consumer)", params: "0 100000 0.1", category: "Power")]
	protected float m_fOnDrawW;

	[Attribute("0.1", desc: "Draw in W in standby (consumer)", params: "0 100000 0.1", category: "Power")]
	protected float m_fStandbyDrawW;

	[Attribute("0", desc: "Maximum output in W (generator, battery, solar peak)", params: "0 10000000 1", category: "Power")]
	protected float m_fOutputW;

	[Attribute("0.1", desc: "Battery capacity in kWh (battery, internal battery of a consumer; 0 = none)", params: "0 10000 0.01", category: "Power"), RplProp()]
	protected float m_fCapacityKWh;

	[Attribute("50", desc: "Recharge power in W of the battery", params: "0 100000 1", category: "Power")]
	protected float m_fRechargeW;

	[Attribute("0", desc: "Fuel tank in L (generator)", params: "0 10000 0.1", category: "Power")]
	protected float m_fFuelCapacityL;

	[Attribute("0", desc: "Fuel use in L/h while running (generator)", params: "0 1000 0.01", category: "Power")]
	protected float m_fBurnLph;

	[Attribute("0", desc: "Solar panel: tilt of the cells in degrees from horizontal, towards the front of the model", params: "0 90 1", category: "Power")]
	protected float m_fPanelTilt;

	[Attribute("1000", desc: "Output in W from the internal battery or buffer (consumer battery, solar buffer, battery without output set)", params: "0 100000 1", category: "Power")]
	protected float m_fBufferOutputW;

	[Attribute("0", desc: "Permanently powered from mains", category: "Power"), RplProp()]
	protected bool m_bMains;

	[Attribute("0", desc: "Runs only on external power (no internal battery use)", category: "Power"), RplProp()]
	protected bool m_bRequiresExternal;

	[Attribute("0", desc: "Powered on at mission start", category: "Power")]
	protected bool m_bStartOn;

	[Attribute("1", desc: "Battery level at start (0-1)", params: "0 1 0.01", category: "Power")]
	protected float m_fStartLevel;

	[Attribute("1", desc: "Fuel level at start (0-1)", params: "0 1 0.01", category: "Power")]
	protected float m_fStartFuel;

	[Attribute("15", desc: "Length in m of the power cable of this source (generator, battery, solar panel)", params: "1 200 1", category: "Power"), RplProp()]
	protected float m_fCableLength;

	[RplProp(onRplName: "OnStateRpl")]
	protected int m_iState;

	[RplProp()]
	protected float m_fLevel = 1;

	[RplProp()]
	protected float m_fFuelL;

	[RplProp()]
	protected bool m_bExternal;

	[RplProp()]
	protected float m_fLoadW;

	[RplProp()]
	protected bool m_bHasProvider;

	protected ROOT_AE3_PowerComponent m_Provider;

	[RplProp()]
	protected RplId m_ProviderId = RplId.Invalid();		//!< Device of m_Provider, for the cable view
	protected ref array<ROOT_AE3_PowerComponent> m_aConsumers = {};
	protected float m_fExtraDrawW;
	protected float m_fExternalLoadW;
	protected float m_fExternalFactor;
	protected float m_fSyncedLevel = -1;
	protected float m_fSyncedFuel = -1;
	protected bool m_bRestored;
	protected int m_iRestoredState = -1;
	[RplProp()]
	protected float m_fLinkLength;	//!< Cable length of the link
	protected float m_fChargeFactor = 1;	//!< Share of the recharge rate the provider can deliver
	protected RplComponent m_RplComponent;
	protected ref ScriptInvokerBase<ROOT_AE3_PowerStateInvoker> m_OnStateChanged;

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!GetGame().InPlayMode())
			return;

		if (IsAuthority())
		{
			m_fLevel = m_fStartLevel;
			m_fFuelL = m_fFuelCapacityL * m_fStartFuel;
			if (m_bMains)
				m_bExternal = true;

			ROOT_AE3_PowerSystem.Get().Register(this);
			if (m_bStartOn)
				GetGame().GetCallqueue().CallLater(StartOn, 500, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		if (IsAuthority() && !ROOT_AE3_Manager.IsEnding())
		{
			Disconnect();
			array<ROOT_AE3_PowerComponent> consumers = {};
			consumers.Copy(m_aConsumers);
			foreach (ROOT_AE3_PowerComponent c : consumers)
			{
				if (c)
					c.Disconnect();
			}

			m_aConsumers.Clear();
			ROOT_AE3_PowerSystem sys = ROOT_AE3_PowerSystem.Peek();
			if (sys)
				sys.Unregister(this);
		}

		GetGame().GetCallqueue().Remove(StartOn);
		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! Persistence restored the state; the editor start state no longer applies.
	void MarkRestored()
	{
		m_bRestored = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: state saved by persistence; applied now and again once links are restored.
	void SetRestoredState(int state)
	{
		m_bRestored = true;
		m_iRestoredState = state;
		ApplyRestoredState();
	}

	//------------------------------------------------------------------------------------------------
	void ApplyRestoredState()
	{
		if (m_iRestoredState < 0 || m_iState == m_iRestoredState)
			return;

		string reason;
		if (m_iRestoredState == ROOT_AE3_EPowerState.OFF)
		{
			TurnOff();
			m_iRestoredState = -1;
			return;
		}

		if (m_iState == ROOT_AE3_EPowerState.OFF && !TurnOn(reason))
			return;

		if (m_iRestoredState == ROOT_AE3_EPowerState.STANDBY)
			Standby(reason);
		else if (m_iRestoredState == ROOT_AE3_EPowerState.CRASHED)
			Crash();

		m_iRestoredState = -1;
	}

	//------------------------------------------------------------------------------------------------
	protected void StartOn()
	{
		if (m_bRestored)
			return;

		string reason;
		TurnOn(reason);
	}

	//------------------------------------------------------------------------------------------------
	bool IsAuthority()
	{
		return !m_RplComponent || !m_RplComponent.IsProxy();
	}

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_PowerComponent Of(IEntity ent)
	{
		if (!ent)
			return null;

		return ROOT_AE3_PowerComponent.Cast(ent.FindComponent(ROOT_AE3_PowerComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! Fired on authority and proxies. Params: component, old state, new state.
	ScriptInvokerBase<ROOT_AE3_PowerStateInvoker> GetOnStateChanged()
	{
		if (!m_OnStateChanged)
			m_OnStateChanged = new ScriptInvokerBase<ROOT_AE3_PowerStateInvoker>();

		return m_OnStateChanged;
	}

	//------------------------------------------------------------------------------------------------
	protected int m_iLastState;

	protected void OnStateRpl()
	{
		if (m_OnStateChanged)
			m_OnStateChanged.Invoke(this, m_iLastState, m_iState);

		m_iLastState = m_iState;
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AE3_EPowerRole GetRole()
	{
		return m_eRole;
	}

	//------------------------------------------------------------------------------------------------
	bool IsProvider()
	{
		return m_eRole != ROOT_AE3_EPowerRole.CONSUMER;
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AE3_EPowerState GetState()
	{
		return m_iState;
	}

	//------------------------------------------------------------------------------------------------
	bool IsOn()
	{
		return m_iState == ROOT_AE3_EPowerState.ON;
	}

	//------------------------------------------------------------------------------------------------
	float GetLevel()
	{
		return m_fLevel;
	}

	//------------------------------------------------------------------------------------------------
	float GetCapacityKWh()
	{
		return m_fCapacityKWh;
	}

	//------------------------------------------------------------------------------------------------
	float GetFuel()
	{
		return m_fFuelL;
	}

	//------------------------------------------------------------------------------------------------
	float GetFuelCapacity()
	{
		return m_fFuelCapacityL;
	}

	//------------------------------------------------------------------------------------------------
	float GetOutputW()
	{
		return m_fOutputW;
	}

	//------------------------------------------------------------------------------------------------
	float GetLoadW()
	{
		return m_fLoadW;
	}

	//------------------------------------------------------------------------------------------------
	bool HasExternalPower()
	{
		return m_bExternal;
	}

	//------------------------------------------------------------------------------------------------
	bool IsMains()
	{
		return m_bMains;
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AE3_PowerComponent GetProvider()
	{
		return m_Provider;
	}

	//------------------------------------------------------------------------------------------------
	RplId GetProviderId()
	{
		return m_ProviderId;
	}

	//------------------------------------------------------------------------------------------------
	float GetLinkLength()
	{
		return m_fLinkLength;
	}

	//------------------------------------------------------------------------------------------------
	//! Replicated: power cable length of this source.
	float GetCableLength()
	{
		return m_fCableLength;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority.
	void SetCableLength(float length)
	{
		m_fCableLength = Math.Clamp(length, 1, 200);
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: devices farther apart than the cable of the link.
	bool LinkStretched()
	{
		return m_Provider && vector.Distance(GetOwner().GetOrigin(), m_Provider.GetOwner().GetOrigin()) > m_fLinkLength;
	}

	//------------------------------------------------------------------------------------------------
	//! Replicated: plugged into a source.
	bool HasProvider()
	{
		return m_bHasProvider;
	}

	//------------------------------------------------------------------------------------------------
	array<ROOT_AE3_PowerComponent> GetConsumers()
	{
		return m_aConsumers;
	}

	//------------------------------------------------------------------------------------------------
	//! Current consumption in W (consumer draw plus battery charging).
	float GetDrawW()
	{
		float w = GetConsumerDrawW();

		if (m_fCapacityKWh > 0 && m_fLevel < 1 && m_eRole != ROOT_AE3_EPowerRole.GENERATOR && m_eRole != ROOT_AE3_EPowerRole.SOLAR)
			w += m_fRechargeW;

		return w;
	}

	//------------------------------------------------------------------------------------------------
	float GetPanelTilt()
	{
		return m_fPanelTilt;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: permanent mains power.
	void SetMains(bool mains)
	{
		m_bMains = mains;
		Replication.BumpMe();
		if (mains)
			SetExternal(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: share 0-1 of the recharge rate set by the power system.
	void SetChargeFactor(float factor)
	{
		m_fChargeFactor = Math.Clamp(factor, 0, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Consumption of the device itself in W, without battery charging.
	float GetConsumerDrawW()
	{
		if (m_eRole != ROOT_AE3_EPowerRole.CONSUMER)
			return 0;

		if (m_iState == ROOT_AE3_EPowerState.ON || m_iState == ROOT_AE3_EPowerState.CRASHED)
			return m_fOnDrawW + m_fExtraDrawW;

		if (m_iState == ROOT_AE3_EPowerState.STANDBY)
			return m_fStandbyDrawW;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: added draw (games and similar). Replaces the previous extra value of the same source total.
	void SetExtraDraw(float w)
	{
		m_fExtraDrawW = Math.Max(0, w);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: draw in W of a load outside the AE3 links on a provider (EWO pack recharge). Served like battery
	//! charging from what the consumers leave; GetExternalFactor is the 0-1 share served in the last power tick.
	void SetExternalLoad(float w)
	{
		m_fExternalLoadW = Math.Max(0, w);
	}

	//------------------------------------------------------------------------------------------------
	float GetExternalLoadW()
	{
		return m_fExternalLoadW;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: set by the power system.
	void SetExternalFactor(float factor)
	{
		m_fExternalFactor = Math.Clamp(factor, 0, 1);
	}

	//------------------------------------------------------------------------------------------------
	float GetExternalFactor()
	{
		return m_fExternalFactor;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: sets power configuration (GM, World Editor, API).
	void Configure(float onW, float standbyW, float capacityKWh, bool mains, bool requiresExternal)
	{
		if (onW >= 0)
			m_fOnDrawW = onW;

		if (standbyW >= 0)
			m_fStandbyDrawW = standbyW;

		if (capacityKWh >= 0)
			m_fCapacityKWh = capacityKWh;

		m_bMains = mains;
		m_bRequiresExternal = requiresExternal;
		Replication.BumpMe();
		if (mains)
			SetExternal(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority.
	void SetLevel(float level)
	{
		m_fLevel = Math.Clamp(level, 0, 1);
		SyncLevel(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority. Fuel in L.
	void SetFuel(float liters)
	{
		m_fFuelL = Math.Clamp(liters, 0, m_fFuelCapacityL);
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: replicates the level when it moved by more than the threshold.
	void SyncLevel(bool force = false)
	{
		ROOT_AE3_Settings s = ROOT_AE3_SettingsManager.Get();
		if (!force && !s.m_bPowerEnableStateSync)
			return;

		if (force || Math.AbsFloat(m_fLevel - m_fSyncedLevel) >= s.m_fPowerChangeThreshold)
		{
			m_fSyncedLevel = m_fLevel;
			Replication.BumpMe();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Authority.
	void SetExternal(bool hasExternal)
	{
		if (m_bMains)
			hasExternal = true;

		if (m_bExternal == hasExternal)
			return;

		m_bExternal = hasExternal;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! Authority.
	void SetLoad(float w)
	{
		if (Math.AbsFloat(m_fLoadW - w) < 0.5)
			return;

		m_fLoadW = w;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! True when the device can draw energy now.
	bool HasEnergy()
	{
		if (m_bExternal || m_bMains)
			return true;

		if (m_bRequiresExternal)
			return false;

		return m_fCapacityKWh > 0 && m_fLevel > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority. Returns false with a reason when the device cannot start.
	bool TurnOn(out string reason)
	{
		if (ROOT_AE3_Util.IsDestroyed(GetOwner()))
		{
			reason = "The device is destroyed.";
			return false;
		}

		if (!IsAuthority())
			return false;

		if (m_iState == ROOT_AE3_EPowerState.ON)
			return true;

		switch (m_eRole)
		{
			case ROOT_AE3_EPowerRole.GENERATOR:
			{
				if (m_fFuelL <= 0)
				{
					reason = "No fuel.";
					return false;
				}

				break;
			}

			case ROOT_AE3_EPowerRole.CONSUMER:
			{
				ROOT_AE3_PowerSystem.Get().RefreshExternal(this);
				if (!HasEnergy())
				{
					reason = "No power.";
					return false;
				}

				break;
			}
		}

		SetState(ROOT_AE3_EPowerState.ON);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority.
	void TurnOff()
	{
		SetState(ROOT_AE3_EPowerState.OFF);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority.
	bool Standby(out string reason)
	{
		if (m_eRole != ROOT_AE3_EPowerRole.CONSUMER)
		{
			reason = "Standby is not supported.";
			return false;
		}

		if (m_iState != ROOT_AE3_EPowerState.ON)
		{
			reason = "The device is not running.";
			return false;
		}

		SetState(ROOT_AE3_EPowerState.STANDBY);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: blue screen; the device needs a power cycle.
	void Crash()
	{
		if (m_iState == ROOT_AE3_EPowerState.ON || m_iState == ROOT_AE3_EPowerState.STANDBY)
			SetState(ROOT_AE3_EPowerState.CRASHED);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority.
	void SetState(ROOT_AE3_EPowerState state)
	{
		if (!IsAuthority() || m_iState == state)
			return;

		m_iState = state;
		Replication.BumpMe();
		OnStateRpl();
		if (state == ROOT_AE3_EPowerState.OFF && IsProvider())
		{
			foreach (ROOT_AE3_PowerComponent c : m_aConsumers)
			{
				if (c)
					c.SetExternal(false);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: connects this consumer to a provider; one provider per consumer.
	//! cable: link length in m; < 0: the longer of the cable of the provider and the distance + 2 m (links without a
	//! player: Game Master, World Editor, script, persistence).
	bool ConnectTo(ROOT_AE3_PowerComponent provider, out string reason, float cable = -1)
	{
		if (!provider || provider == this)
		{
			reason = "Invalid power source.";
			return false;
		}

		if (!provider.IsProvider())
		{
			reason = "Target is not a power source.";
			return false;
		}

		if (m_eRole == ROOT_AE3_EPowerRole.GENERATOR || m_eRole == ROOT_AE3_EPowerRole.SOLAR)
		{
			reason = "A source cannot be powered.";
			return false;
		}

		ROOT_AE3_PowerComponent p = provider;
		while (p)
		{
			if (p == this)
			{
				reason = "Connection would form a loop.";
				return false;
			}

			p = p.m_Provider;
		}

		Disconnect();
		m_Provider = provider;
		m_fLinkLength = cable;
		if (cable <= 0)
			m_fLinkLength = Math.Max(provider.GetCableLength(), vector.Distance(GetOwner().GetOrigin(), provider.GetOwner().GetOrigin()) + 2);

		provider.m_aConsumers.Insert(this);
		m_bHasProvider = true;
		ROOT_AE3_DeviceComponent pd = ROOT_AE3_DeviceComponent.Of(provider.GetOwner());
		if (pd)
			m_ProviderId = pd.GetRplId();

		Replication.BumpMe();
		ROOT_AE3_PowerSystem.Get().RefreshExternal(this);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority.
	void Disconnect()
	{
		if (m_Provider)
			m_Provider.m_aConsumers.RemoveItem(this);

		m_Provider = null;
		if (m_bHasProvider)
		{
			m_bHasProvider = false;
			m_ProviderId = RplId.Invalid();
			Replication.BumpMe();
		}

		if (!m_bMains)
			SetExternal(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: removes every consumer of this provider.
	void DisconnectConsumers()
	{
		array<ROOT_AE3_PowerComponent> list = {};
		list.Copy(m_aConsumers);
		foreach (ROOT_AE3_PowerComponent c : list)
		{
			if (c)
				c.Disconnect();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Authority, called by the power system.
	void Simulate_Consume(float dt)
	{
		if (m_eRole == ROOT_AE3_EPowerRole.CONSUMER && !m_bExternal && m_fCapacityKWh > 0 && !m_bRequiresExternal)
		{
			float use = GetConsumerDrawW();
			if (m_iState != ROOT_AE3_EPowerState.OFF && use > 0)
			{
				m_fLevel = Math.Max(0, m_fLevel - use * dt / 3600000.0 / m_fCapacityKWh);
				SyncLevel();
				if (m_fLevel <= 0)
					TurnOff();
			}
		}
		else if (m_eRole == ROOT_AE3_EPowerRole.CONSUMER && !m_bExternal && m_iState != ROOT_AE3_EPowerState.OFF)
		{
			TurnOff();
		}

		if (m_bExternal && m_fCapacityKWh > 0 && m_fLevel < 1 && m_eRole != ROOT_AE3_EPowerRole.GENERATOR && m_eRole != ROOT_AE3_EPowerRole.SOLAR)
		{
			m_fLevel = Math.Min(1, m_fLevel + m_fRechargeW * m_fChargeFactor * dt / 3600000.0 / m_fCapacityKWh);
			SyncLevel();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: available output of a provider in W for this tick.
	float Simulate_Available(float sunFactor)
	{
		switch (m_eRole)
		{
			case ROOT_AE3_EPowerRole.GENERATOR:
			{
				if (m_iState == ROOT_AE3_EPowerState.ON && m_fFuelL > 0)
					return m_fOutputW;

				return 0;
			}

			case ROOT_AE3_EPowerRole.SOLAR:
			{
				if (m_iState != ROOT_AE3_EPowerState.ON)
					return 0;

				return m_fOutputW * sunFactor + BatteryOutput();
			}

			case ROOT_AE3_EPowerRole.BATTERY:
				return BatteryOutput();
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected float BatteryOutput()
	{
		if (m_eRole == ROOT_AE3_EPowerRole.BATTERY && m_iState == ROOT_AE3_EPowerState.OFF)
			return 0;

		if (m_fCapacityKWh > 0 && m_fLevel > 0)
		{
			if (m_fOutputW > 0 && m_eRole == ROOT_AE3_EPowerRole.BATTERY)
				return m_fOutputW;

			return m_fBufferOutputW;
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: applies the supplied load for this tick.
	void Simulate_Supply(float loadW, float dt, float sunFactor)
	{
		SetLoad(loadW);
		switch (m_eRole)
		{
			case ROOT_AE3_EPowerRole.GENERATOR:
			{
				if (m_iState == ROOT_AE3_EPowerState.ON)
				{
					m_fFuelL = Math.Max(0, m_fFuelL - m_fBurnLph * dt / 3600.0);
					if (Math.AbsFloat(m_fFuelL - m_fSyncedFuel) >= 0.05 || m_fFuelL <= 0)
					{
						m_fSyncedFuel = m_fFuelL;
						Replication.BumpMe();
					}
					if (m_fFuelL <= 0)
						TurnOff();
				}

				break;
			}

			case ROOT_AE3_EPowerRole.SOLAR:
			{
				float solar = 0;
				if (m_iState == ROOT_AE3_EPowerState.ON)
					solar = m_fOutputW * sunFactor;

				float net = solar - loadW;
				if (m_fCapacityKWh > 0)
				{
					m_fLevel = Math.Clamp(m_fLevel + net * dt / 3600000.0 / m_fCapacityKWh, 0, 1);
					SyncLevel();
				}

				break;
			}

			case ROOT_AE3_EPowerRole.BATTERY:
			{
				if (m_fCapacityKWh > 0)
				{
					m_fLevel = Math.Max(0, m_fLevel - loadW * dt / 3600000.0 / m_fCapacityKWh);
					SyncLevel();
				}

				break;
			}
		}
	}
}

void ROOT_AE3_PowerStateInvoker(ROOT_AE3_PowerComponent power, int oldState, int newState);
typedef func ROOT_AE3_PowerStateInvoker;
