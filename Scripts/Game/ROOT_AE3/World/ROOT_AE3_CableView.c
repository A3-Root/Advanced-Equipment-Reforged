//! Client: a cable drawn as a tube between two ports (procedural mesh on a local, not replicated entity).
//! Each end has a plug; the cable sags with its slack (cable length minus distance) and rests on the ground.
class ROOT_AE3_CableView
{
	protected static const int SIDES = 6;
	protected static const int CURVE = 46;					//!< Points of the hanging part
	protected static const int RINGS = 52;					//!< CURVE + 3 plug rings at each end
	protected static const float PLUG_LEN = 0.025;
	protected static const float PLUG_R = 0.007;
	protected static const float LEAD = 0.06;				//!< Straight part out of the port

	protected IEntity m_Entity;
	protected ref Resource m_Res;
	protected MeshObject m_Mesh;
	protected ResourceName m_Material;
	protected float m_fRadius;
	protected vector m_vA;
	protected vector m_vB;
	protected float m_fLen = -1;

	//------------------------------------------------------------------------------------------------
	void ROOT_AE3_CableView(ResourceName material, float radius)
	{
		m_Material = material;
		m_fRadius = radius;
	}

	//------------------------------------------------------------------------------------------------
	void ~ROOT_AE3_CableView()
	{
		Destroy();
	}

	//------------------------------------------------------------------------------------------------
	void Destroy()
	{
		if (m_Entity)
			SCR_EntityHelper.DeleteEntityAndChildren(m_Entity);

		m_Entity = null;
		m_Mesh = null;
		m_Res = null;
		m_fLen = -1;
	}

	//------------------------------------------------------------------------------------------------
	//! a, b: port points; aDir, bDir: unit directions out of the ports; len: cable length in m.
	//! Rebuilds only when an end moved more than 2 cm or the length changed.
	void Update(vector a, vector aDir, vector b, vector bDir, float len)
	{
		if (m_Entity && vector.DistanceSq(a, m_vA) < 0.0004 && vector.DistanceSq(b, m_vB) < 0.0004 && float.AlmostEqual(len, m_fLen))
			return;

		m_vA = a;
		m_vB = b;
		m_fLen = len;
		array<vector> pts = {};
		array<float> radii = {};
		Points(a, aDir, b, bDir, len, pts, radii);
		Build(a, pts, radii);
	}

	//------------------------------------------------------------------------------------------------
	protected void Points(vector a, vector aDir, vector b, vector bDir, float len, notnull array<vector> pts, notnull array<float> radii)
	{
		vector a1 = a + aDir * PLUG_LEN;
		vector b1 = b + bDir * PLUG_LEN;
		vector s = a + aDir * LEAD;
		vector e = b + bDir * LEAD;
		pts.Insert(a);
		radii.Insert(PLUG_R);
		pts.Insert(a1);
		radii.Insert(PLUG_R);
		pts.Insert(a1);
		radii.Insert(m_fRadius);

		float chord = vector.Distance(s, e);
		float slack = Math.Max(0, len - chord - LEAD * 2);
		float sag = Math.Sqrt(3 * chord * slack / 8);
		BaseWorld world = GetGame().GetWorld();
		for (int i = 0; i < CURVE; i++)
		{
			float t = i / (CURVE - 1.0);
			vector p = s + (e - s) * t;
			p[1] = p[1] - 4 * sag * t * (1 - t);
			float ground = world.GetSurfaceY(p[0], p[2]) + m_fRadius;
			if (p[1] < ground)
				p[1] = ground;

			pts.Insert(p);
			radii.Insert(m_fRadius);
		}

		pts.Insert(b1);
		radii.Insert(m_fRadius);
		pts.Insert(b1);
		radii.Insert(PLUG_R);
		pts.Insert(b);
		radii.Insert(PLUG_R);
	}

	//------------------------------------------------------------------------------------------------
	//! Rings around each point; triangles in both windings (seen from inside and outside).
	protected void Build(vector origin, notnull array<vector> pts, notnull array<float> radii)
	{
		vector verts[312];
		float uvs[624];
		int indices[3672];
		vector up = "0 1 0";
		int n = pts.Count();
		for (int i = 0; i < n; i++)
		{
			vector prev = pts[Math.Max(i - 1, 0)];
			vector next = pts[Math.Min(i + 1, n - 1)];
			vector t = next - prev;
			if (t.LengthSq() < 0.000001)
				t = pts[n - 1] - pts[0];

			t.Normalize();
			vector side = t * up;
			if (side.LengthSq() < 0.000001)
				side = t * "1 0 0";

			side.Normalize();
			vector bin = side * t;
			for (int k = 0; k < SIDES; k++)
			{
				float ang = k * Math.PI2 / SIDES;
				int v = i * SIDES + k;
				verts[v] = pts[i] - origin + (side * Math.Cos(ang) + bin * Math.Sin(ang)) * radii[i];
				uvs[v * 2] = k / (SIDES * 1.0);
				uvs[v * 2 + 1] = i * 0.1;
			}
		}

		int ii;
		for (int r = 0; r < n - 1; r++)
		{
			for (int k = 0; k < SIDES; k++)
			{
				int k2 = (k + 1) % SIDES;
				int v00 = r * SIDES + k;
				int v01 = r * SIDES + k2;
				int v10 = (r + 1) * SIDES + k;
				int v11 = (r + 1) * SIDES + k2;
				indices[ii] = v00; ii++;
				indices[ii] = v10; ii++;
				indices[ii] = v11; ii++;
				indices[ii] = v11; ii++;
				indices[ii] = v01; ii++;
				indices[ii] = v00; ii++;
				indices[ii] = v00; ii++;
				indices[ii] = v11; ii++;
				indices[ii] = v10; ii++;
				indices[ii] = v11; ii++;
				indices[ii] = v00; ii++;
				indices[ii] = v01; ii++;
			}
		}

		if (!m_Entity)
		{
			m_Entity = GetGame().SpawnEntity(GenericEntity, GetGame().GetWorld());
			int numVerts[] = {RINGS * SIDES};
			int numIndices[] = {(RINGS - 1) * SIDES * 12};
			string materials[] = {m_Material};
			m_Res = MeshObject.Create(1, numVerts, numIndices, materials, 0);
			if (m_Res && m_Res.IsValid())
				m_Mesh = m_Res.GetResource().ToMeshObject();
		}

		if (!m_Entity || !m_Mesh)
			return;

		m_Mesh.UpdateVerts(0, verts, uvs);
		m_Mesh.UpdateIndices(0, indices);
		m_Entity.SetOrigin(origin);
		m_Entity.SetObject(m_Mesh, "");
		m_Entity.Update();
	}
}
