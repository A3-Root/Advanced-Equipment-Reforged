//! Server-side power simulation. Every UpdateInterval seconds: providers supply their consumers,
//! overloaded generators and batteries shut down, unsupplied consumers run on internal batteries.
class ROOT_AE3_PowerSystem
{
	protected static ref ROOT_AE3_PowerSystem s_Instance;
	protected ref array<ROOT_AE3_PowerComponent> m_aAll = {};
	protected bool m_bRunning;
	protected float m_fInterval = 5;

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_PowerSystem Get()
	{
		if (!s_Instance)
			s_Instance = new ROOT_AE3_PowerSystem();

		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	//! Drops all state (game end).
	static void Reset()
	{
		if (s_Instance)
			GetGame().GetCallqueue().Remove(s_Instance.Tick);

		s_Instance = null;
	}

	//------------------------------------------------------------------------------------------------
	void Register(ROOT_AE3_PowerComponent p)
	{
		if (!m_aAll.Contains(p))
			m_aAll.Insert(p);

		Start();
	}

	//------------------------------------------------------------------------------------------------
	//! Instance without creating one (component teardown).
	static ROOT_AE3_PowerSystem Peek()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	void Unregister(ROOT_AE3_PowerComponent p)
	{
		m_aAll.RemoveItem(p);
	}

	//------------------------------------------------------------------------------------------------
	void Start()
	{
		if (m_bRunning || !Replication.IsServer())
			return;

		m_bRunning = true;
		m_fInterval = ROOT_AE3_SettingsManager.Get().m_fPowerUpdateInterval;
		GetGame().GetCallqueue().CallLater(Tick, m_fInterval * 1000, true);
	}

	//------------------------------------------------------------------------------------------------
	//! 0-1 output factor of a solar panel from sun elevation and panel tilt.
	float SunFactor(ROOT_AE3_PowerComponent panel)
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return 0;

		TimeAndWeatherManagerEntity tw = world.GetTimeAndWeatherManager();
		if (!tw || !tw.IsDayHour(tw.GetTimeOfTheDay()))
			return 0;

		vector sunDir, moonDir;
		float phase;
		tw.GetCurrentSunMoonDirAndPhase(sunDir, moonDir, phase);
		vector toSun = -sunDir.Normalized();
		if (toSun[1] < 0)
			toSun = sunDir.Normalized();

		vector m[4];
		panel.GetOwner().GetWorldTransform(m);
		float tilt = panel.GetPanelTilt() * Math.DEG2RAD;
		vector normal = (m[1] * Math.Cos(tilt) + m[2] * Math.Sin(tilt)).Normalized();
		float f = vector.Dot(normal, toSun);
		return Math.Clamp(f, 0, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: updates the external power flag of a consumer from its provider chain.
	void RefreshExternal(ROOT_AE3_PowerComponent c)
	{
		if (c.IsMains())
		{
			c.SetExternal(true);
			return;
		}

		ROOT_AE3_PowerComponent p = c.GetProvider();
		if (!p)
		{
			c.SetExternal(false);
			return;
		}

		c.SetExternal(p.Simulate_Available(SunFactor(p)) > 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void Tick()
	{
		float dt = m_fInterval;
		for (int i = m_aAll.Count() - 1; i >= 0; i--)
		{
			if (!m_aAll[i])
				m_aAll.Remove(i);
		}

		// Carried devices lose their cable; destroyed devices switch off.
		foreach (ROOT_AE3_PowerComponent cp : m_aAll)
		{
			if (cp.GetState() != ROOT_AE3_EPowerState.OFF && ROOT_AE3_Util.IsDestroyed(cp.GetOwner()))
			{
				cp.TurnOff();
				cp.DisconnectConsumers();
			}

			if (cp.GetProvider() && (ROOT_AE3_Util.IsCarried(cp.GetOwner()) || cp.LinkStretched()))
				cp.Disconnect();
		}

		// Providers supply their consumers.
		foreach (ROOT_AE3_PowerComponent p : m_aAll)
		{
			if (!p.IsProvider())
				continue;

			if (!p.GetConsumers().IsEmpty() && ROOT_AE3_Util.IsCarried(p.GetOwner()))
			{
				p.DisconnectConsumers();
				continue;
			}

			float sun;
			if (p.GetRole() == ROOT_AE3_EPowerRole.SOLAR)
				sun = SunFactor(p);

			float available = p.Simulate_Available(sun);
			float demand = 0;
			float charge = 0;
			foreach (ROOT_AE3_PowerComponent c : p.GetConsumers())
			{
				if (!c)
					continue;

				float own = c.GetConsumerDrawW();
				demand += own;
				charge += c.GetDrawW() - own;
			}

			bool supplying = available > 0;
			if (supplying && demand > available && p.GetRole() != ROOT_AE3_EPowerRole.SOLAR)
			{
				// Overload: the provider switches itself off.
				p.TurnOff();
				supplying = false;
				ROOT_AE3_Log.Debug(string.Format("Power overload on %1 (%2 W > %3 W)", p.GetOwner(), demand, available));
			}

			if (p.GetRole() == ROOT_AE3_EPowerRole.SOLAR && demand > available)
				supplying = false;

			// Batteries charge from what is left after the consumers.
			float factor = 0;
			if (supplying && charge > 0)
				factor = Math.Clamp((available - demand) / charge, 0, 1);

			foreach (ROOT_AE3_PowerComponent c : p.GetConsumers())
			{
				if (!c)
					continue;

				c.SetExternal(supplying);
				c.SetChargeFactor(factor);
			}

			float load;
			if (supplying)
				load = demand + charge * factor;

			p.Simulate_Supply(load, dt, sun);
		}

		// Consumers without a provider.
		foreach (ROOT_AE3_PowerComponent c : m_aAll)
		{
			if (!c.GetProvider() && !c.IsMains())
				c.SetExternal(false);

			c.Simulate_Consume(dt);
		}
	}
}
