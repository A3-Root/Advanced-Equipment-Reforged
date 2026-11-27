//! Client: a cable drawn between two points from local, not replicated segment entities (0.25 m along +Z, scaled to
//! fit). Network cables use ROOT_AER_CableSegment (7 mm), power cables ROOT_AER_CableSegmentPower (11 mm). A cable
//! with more than COIL_SLACK of spare length runs down to a coil (ROOT_AER_NetworkCable model) on the floor halfway;
//! otherwise it sags with its slack. Points stay above the terrain and the floor found under the middle.
class ROOT_AER_CableView
{
	static const ResourceName NET_MODEL = "{94B4C52A99D8F7B0}Assets/ROOT_AER/ROOT_AER_CableSegment/ROOT_AER_CableSegment.xob";
	static const ResourceName POWER_MODEL = "{C0E4F070C24AC863}Assets/ROOT_AER/ROOT_AER_CableSegmentPower/ROOT_AER_CableSegmentPower.xob";
	protected static const ResourceName COIL_MODEL = "{043FF542270ABA41}Assets/ROOT_AER/ROOT_AER_NetworkCable/ROOT_AER_NetworkCable.xob";
	protected static const float SEG_LEN = 0.25;
	protected static const int MAX_SEGMENTS = 320;
	protected static const float LEAD = 0.06;				//!< Straight part out of the port
	protected static const float COIL_SLACK = 1.5;			//!< Spare length in m above which the rest is coiled

	protected ref array<IEntity> m_aSegments = {};
	protected IEntity m_Coil;
	protected ref Resource m_Res;
	protected ref Resource m_CoilRes;
	protected ResourceName m_Model;
	protected string m_sRemap;
	protected string m_sCoilRemap;
	protected vector m_vA;
	protected vector m_vB;
	protected float m_fLen = -1;
	protected bool m_bCoil;
	protected vector m_vCoil;
	protected float m_fCoilYaw;

	//------------------------------------------------------------------------------------------------
	void ROOT_AER_CableView(ResourceName material, ResourceName model = NET_MODEL)
	{
		m_Model = model;
		m_sRemap = string.Format("$remap 'AER_Plastic_Dark' '%1';", material);
		m_sCoilRemap = m_sRemap;
	}

	//------------------------------------------------------------------------------------------------
	void ~ROOT_AER_CableView()
	{
		Destroy();
	}

	//------------------------------------------------------------------------------------------------
	void Destroy()
	{
		foreach (IEntity e : m_aSegments)
		{
			if (e)
				delete e;
		}

		m_aSegments.Clear();
		if (m_Coil)
			delete m_Coil;

		m_Coil = null;
		m_Res = null;
		m_CoilRes = null;
		m_fLen = -1;
	}

	//------------------------------------------------------------------------------------------------
	//! a, b: end points; aDir, bDir: unit directions out of the ports; len: cable length in m.
	//! Moves the segments only when an end moved more than 2 cm or the length changed.
	void Update(vector a, vector aDir, vector b, vector bDir, float len, bool allowCoil = true)
	{
		if (!m_aSegments.IsEmpty() && vector.DistanceSq(a, m_vA) < 0.0004 && vector.DistanceSq(b, m_vB) < 0.0004 && float.AlmostEqual(len, m_fLen))
			return;

		m_vA = a;
		m_vB = b;
		m_fLen = len;
		array<vector> pts = {};
		Points(a, aDir, b, bDir, len, allowCoil, pts);
		Place(pts);
	}

	//------------------------------------------------------------------------------------------------
	//! Height of the first surface (terrain, floor, table) under p, or fallback.
	protected static float FloorY(vector p, float fallback, IEntity exclude = null)
	{
		TraceParam param = new TraceParam();
		param.Start = p + vector.Up * 0.3;
		param.End = p - vector.Up * 40;
		param.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
		param.Exclude = exclude;
		float f = GetGame().GetWorld().TraceMove(param, null);
		if (f >= 1)
			return fallback;

		return param.Start[1] + (param.End[1] - param.Start[1]) * f;
	}

	//------------------------------------------------------------------------------------------------
	protected void Points(vector a, vector aDir, vector b, vector bDir, float len, bool allowCoil, notnull array<vector> pts)
	{
		vector s = a + aDir * LEAD;
		vector e = b + bDir * LEAD;
		float chord = vector.Distance(s, e);
		float slack = Math.Max(0, len - chord - LEAD * 2);
		vector mid = (s + e) * 0.5;
		BaseWorld world = GetGame().GetWorld();
		float floor = FloorY(mid, world.GetSurfaceY(mid[0], mid[2]));
		pts.Insert(a);
		m_bCoil = allowCoil && slack > COIL_SLACK;
		if (m_bCoil)
		{
			m_vCoil = mid;
			m_vCoil[1] = floor + 0.002;
			vector flat = e - s;
			m_fCoilYaw = Math.Atan2(flat[0], flat[2]) * Math.RAD2DEG;
			float half = vector.Distance(s, m_vCoil) * 1.08 + 0.1;
			Hang(s, m_vCoil, half, floor, pts, false);
			float half2 = vector.Distance(m_vCoil, e) * 1.08 + 0.1;
			Hang(m_vCoil, e, half2, floor, pts, true);
		}
		else
		{
			Hang(s, e, len - LEAD * 2, floor, pts, true);
		}

		pts.Insert(b);
	}

	//------------------------------------------------------------------------------------------------
	//! Hanging curve from p0 to p1 with a cable of length len; points not below the terrain or floor.
	protected void Hang(vector p0, vector p1, float len, float floor, notnull array<vector> pts, bool last)
	{
		float chord = vector.Distance(p0, p1);
		float slack = Math.Max(0, len - chord);
		float sag = Math.Sqrt(3 * chord * slack / 8);
		int n = Math.ClampInt(Math.Ceil((chord + slack) / SEG_LEN), 2, MAX_SEGMENTS / 2);
		BaseWorld world = GetGame().GetWorld();
		int count = n;
		if (last)
			count = n + 1;

		for (int i = 0; i < count; i++)
		{
			float t = i / (n * 1.0);
			vector p = p0 + (p1 - p0) * t;
			p[1] = p[1] - 4 * sag * t * (1 - t);
			float ground = Math.Max(world.GetSurfaceY(p[0], p[2]), floor) + 0.006;
			if (p[1] < ground)
				p[1] = ground;

			pts.Insert(p);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! One segment entity per pair of points, oriented along it and scaled to its length; the coil when used.
	protected void Place(notnull array<vector> pts)
	{
		if (!m_Res)
		{
			m_Res = Resource.Load(m_Model);
			if (!m_Res || !m_Res.IsValid())
				return;
		}

		VObject obj = m_Res.GetResource().ToVObject();
		if (!obj)
			return;

		BaseWorld world = GetGame().GetWorld();
		int count = pts.Count() - 1;
		while (m_aSegments.Count() > count)
		{
			IEntity last = m_aSegments[m_aSegments.Count() - 1];
			m_aSegments.Remove(m_aSegments.Count() - 1);
			if (last)
				delete last;
		}

		while (m_aSegments.Count() < count)
		{
			IEntity seg = GetGame().SpawnEntity(GenericEntity, world);
			if (!seg)
				return;

			seg.SetObject(obj, m_sRemap);
			seg.SetFlags(EntityFlags.VISIBLE, false);
			m_aSegments.Insert(seg);
		}

		vector up = "0 1 0";
		for (int i = 0; i < count; i++)
		{
			vector p0 = pts[i];
			vector d = pts[i + 1] - p0;
			float l = d.Length();
			IEntity seg = m_aSegments[i];
			if (!seg)
				continue;

			if (l < 0.001)
			{
				seg.ClearFlags(EntityFlags.VISIBLE, false);
				continue;
			}

			seg.SetFlags(EntityFlags.VISIBLE, false);
			vector z = d * (1 / l);
			vector x = up * z;
			if (x.LengthSq() < 0.000001)
				x = "1 0 0" * z;

			x.Normalize();
			vector y = z * x;
			vector mat[4];
			mat[0] = x;
			mat[1] = y;
			mat[2] = z;
			mat[3] = p0;
			seg.SetWorldTransform(mat);
			seg.SetScale(l / SEG_LEN);
			seg.Update();
		}

		PlaceCoil();
	}

	//------------------------------------------------------------------------------------------------
	protected void PlaceCoil()
	{
		if (!m_bCoil)
		{
			if (m_Coil)
				delete m_Coil;

			m_Coil = null;
			return;
		}

		if (!m_Coil)
		{
			if (!m_CoilRes)
				m_CoilRes = Resource.Load(COIL_MODEL);

			if (!m_CoilRes || !m_CoilRes.IsValid())
				return;

			VObject obj = m_CoilRes.GetResource().ToVObject();
			if (!obj)
				return;

			m_Coil = GetGame().SpawnEntity(GenericEntity, GetGame().GetWorld());
			if (!m_Coil)
				return;

			m_Coil.SetObject(obj, m_sCoilRemap);
			m_Coil.SetFlags(EntityFlags.VISIBLE, false);
		}

		vector mat[4];
		Math3D.AnglesToMatrix(Vector(m_fCoilYaw, 0, 0), mat);
		mat[3] = m_vCoil;
		m_Coil.SetWorldTransform(mat);
		m_Coil.Update();
	}
}
