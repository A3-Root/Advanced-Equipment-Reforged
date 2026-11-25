//! Cameras for the desktop CCTV app. Registered on the server; names and RplIds are sent to clients on request.
class ROOT_AE3_Cctv
{
	protected static ref array<IEntity> s_aCameras = {};
	protected static ref array<string> s_aNames = {};

	//------------------------------------------------------------------------------------------------
	//! Drops all state (game end).
	static void Reset()
	{
		s_aCameras.Clear();
		s_aNames.Clear();
	}

	//------------------------------------------------------------------------------------------------
	static void Register(IEntity camera, string name)
	{
		int i = s_aCameras.Find(camera);
		if (i >= 0)
		{
			s_aNames[i] = ROOT_AE3_Codec.Clean(name);
			return;
		}

		s_aCameras.Insert(camera);
		s_aNames.Insert(ROOT_AE3_Codec.Clean(name));
	}

	//------------------------------------------------------------------------------------------------
	//! Name, position, forward direction and RplId (-1 when not replicated) of every live camera.
	static void List(notnull array<string> outFields)
	{
		for (int i = s_aCameras.Count() - 1; i >= 0; i--)
		{
			if (!s_aCameras[i])
			{
				s_aCameras.Remove(i);
				s_aNames.Remove(i);
			}
		}

		foreach (int i, IEntity cam : s_aCameras)
		{
			vector m[4];
			cam.GetWorldTransform(m);
			outFields.Insert(s_aNames[i]);
			outFields.Insert(m[3].ToString(false));
			outFields.Insert(m[2].ToString(false));
			RplComponent rpl = RplComponent.Cast(cam.FindComponent(RplComponent));
			int id = -1;
			if (rpl)
				id = Replication.FindItemId(rpl);

			outFields.Insert(id.ToString());
		}
	}
}
