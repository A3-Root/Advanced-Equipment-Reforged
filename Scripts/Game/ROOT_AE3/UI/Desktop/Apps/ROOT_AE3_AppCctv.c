//! CCTV viewer for cameras registered by the mission.
class ROOT_AE3_AppCctv : ROOT_AE3_App
{
	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "CCTV";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "cctv";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(760, 520, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected ROOT_AE3_ListView m_List;
	protected ref array<vector> m_aPos = {};
	protected ref array<vector> m_aDir = {};
	protected RenderTargetWidget m_wView;
	protected static const int CAMERA_INDEX = 3;	//!< World camera slot used for the feed
	protected ref array<int> m_aIds = {};
	protected int m_iSelected = -1;

	override void Build()
	{
		float w = Width();
		float h = Height();
		m_List = List(1, 6, 6, 200, h - 12);
		array<string> headers = {"Cameras"};
		array<float> widths = {200};
		m_List.SetColumns(headers, widths);
		m_wView = RenderTargetWidget.Cast(GetGame().GetWorkspace().CreateWidget(WidgetType.RenderTargetWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR, Color.FromInt(Color.WHITE), 0, GetContent()));
		ROOT_AE3_UI.Place(m_wView, 212, 6, w - 218, h - 12);
		Request0("cctv_list");
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (!ok)
			return;

		m_List.Clear();
		m_aPos.Clear();
		m_aDir.Clear();
		m_aIds.Clear();
		for (int i = 0; i + 3 < payload.Count(); i += 4)
		{
			array<string> row = {payload[i]};
			m_List.AddRow(row);
			m_aPos.Insert(payload[i + 1].ToVector());
			m_aDir.Insert(payload[i + 2].ToVector());
			m_aIds.Insert(payload[i + 3].ToInt());
		}

		m_List.Render();
		Status(string.Format("%1 cameras", m_aPos.Count()));
	}

	//------------------------------------------------------------------------------------------------
	override void OnListSelect(int listId, int row)
	{
		if (row < 0 || row >= m_aPos.Count() || !m_wView)
			return;

		m_iSelected = row;
		BaseWorld world = GetGame().GetWorld();
		PlaceCamera();
		world.SetCameraVerticalFOV(CAMERA_INDEX, 60);
		m_wView.SetWorld(world, CAMERA_INDEX);
		Status("Viewing " + m_List.m_aRows[row][0]);
	}

	//------------------------------------------------------------------------------------------------
	//! Camera pose from the replicated camera entity when streamed in, else the pose sent by the server.
	protected void PlaceCamera()
	{
		if (m_iSelected < 0 || m_iSelected >= m_aPos.Count())
			return;

		vector mat[4];
		IEntity cam;
		if (m_aIds[m_iSelected] >= 0)
		{
			RplComponent rpl = RplComponent.Cast(Replication.FindItem(m_aIds[m_iSelected]));
			if (rpl)
				cam = rpl.GetEntity();
		}

		if (cam)
		{
			cam.GetWorldTransform(mat);
		}
		else
		{
			Math3D.DirectionAndUpMatrix(m_aDir[m_iSelected], vector.Up, mat);
			mat[3] = m_aPos[m_iSelected];
		}

		GetGame().GetWorld().SetCameraEx(CAMERA_INDEX, mat);
	}

	//------------------------------------------------------------------------------------------------
	override void Update(float dt)
	{
		PlaceCamera();
	}
}
