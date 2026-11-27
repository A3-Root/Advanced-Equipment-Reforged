//! Client: a cable drawn between two points from local, not replicated segment entities (ROOT_AE3_CableSegment.xob,
//! 0.25 m along +Z, scaled to fit). The cable sags with its slack (cable length minus distance) and rests on the ground.
class ROOT_AE3_CableView
{
	protected static const ResourceName MODEL = "{94B4C52A99D8F7B0}Assets/ROOT_AE3/ROOT_AE3_CableSegment/ROOT_AE3_CableSegment.xob";
	protected static const float SEG_LEN = 0.25;
	protected static const int MAX_SEGMENTS = 160;
	protected static const float LEAD = 0.06;				//!< Straight part out of the port

	protected ref array<IEntity> m_aSegments = {};
	protected ref Resource m_Res;
	protected string m_sRemap;
	protected vector m_vA;
	protected vector m_vB;
	protected float m_fLen = -1;

	//------------------------------------------------------------------------------------------------
	void ROOT_AE3_CableView(ResourceName material)
	{
		m_sRemap = string.Format("$remap 'AE3_Plastic_Dark' '%1';", material);
	}

	//------------------------------------------------------------------------------------------------
	void ~ROOT_AE3_CableView()
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
		m_Res = null;
		m_fLen = -1;
	}

	//------------------------------------------------------------------------------------------------
	//! a, b: end points; aDir, bDir: unit directions out of the ports; len: cable length in m.
	//! Moves the segments only when an end moved more than 2 cm or the length changed.
	void Update(vector a, vector aDir, vector b, vector bDir, float len)
	{
		if (!m_aSegments.IsEmpty() && vector.DistanceSq(a, m_vA) < 0.0004 && vector.DistanceSq(b, m_vB) < 0.0004 && float.AlmostEqual(len, m_fLen))
			return;

		m_vA = a;
		m_vB = b;
		m_fLen = len;
		array<vector> pts = {};
		Points(a, aDir, b, bDir, len, pts);
		Place(pts);
	}

	//------------------------------------------------------------------------------------------------
	protected void Points(vector a, vector aDir, vector b, vector bDir, float len, notnull array<vector> pts)
	{
		vector s = a + aDir * LEAD;
		vector e = b + bDir * LEAD;
		float chord = vector.Distance(s, e);
		float slack = Math.Max(0, len - chord - LEAD * 2);
		float sag = Math.Sqrt(3 * chord * slack / 8);
		float arc = chord + slack;
		int n = Math.ClampInt(Math.Ceil(arc / SEG_LEN), 2, MAX_SEGMENTS - 2);
		BaseWorld world = GetGame().GetWorld();
		pts.Insert(a);
		for (int i = 0; i <= n; i++)
		{
			float t = i / (n * 1.0);
			vector p = s + (e - s) * t;
			p[1] = p[1] - 4 * sag * t * (1 - t);
			float ground = world.GetSurfaceY(p[0], p[2]) + 0.005;
			if (p[1] < ground)
				p[1] = ground;

			pts.Insert(p);
		}

		pts.Insert(b);
	}

	//------------------------------------------------------------------------------------------------
	//! One segment entity per pair of points, oriented along it and scaled to its length.
	protected void Place(notnull array<vector> pts)
	{
		if (!m_Res)
		{
			m_Res = Resource.Load(MODEL);
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
	}
}
