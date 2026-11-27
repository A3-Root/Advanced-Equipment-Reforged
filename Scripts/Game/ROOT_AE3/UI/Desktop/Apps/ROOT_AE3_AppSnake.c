//! Snake in a window. Arrow keys or WASD steer, Enter or Space starts and restarts.
//! Pauses while the window is not focused. Client only.
class ROOT_AE3_AppSnake : ROOT_AE3_App
{
	protected static const int ID_NEW = 1;
	protected static const int ID_SIZE = 2;
	protected static const float BAR = 40;

	protected static int s_iBest;
	protected static bool s_bLarge;

	protected CanvasWidget m_wCanvas;
	protected TextWidget m_wScore;
	protected TextWidget m_wInfo;
	protected ref array<ref CanvasWidgetCommand> m_aCommands = {};
	protected ref array<int> m_aBody = {};
	protected int m_iCols = 32;
	protected int m_iRows = 20;
	protected float m_fCell = 16;
	protected float m_fOffX;
	protected float m_fOffY;
	protected int m_iDir = 1;		//!< 0 up, 1 right, 2 down, 3 left
	protected int m_iNextDir = 1;
	protected int m_iFood = -1;
	protected int m_iScore;
	protected bool m_bDead;
	protected bool m_bRunning;
	protected float m_fTimer;

	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Snake";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "snake";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(680, 520, 0);
	}

	//------------------------------------------------------------------------------------------------
	override void Build()
	{
		float w = Width();
		float h = Height();
		ROOT_AE3_UI.Rect(GetContent(), 0, 0, w, h, ROOT_AE3_UI.Rgb(0.05, 0.07, 0.05));
		MainBtn(8, 7, 100, 26, "New game", ID_NEW);
		string size = "Large board";
		if (s_bLarge)
			size = "Small board";

		Btn(114, 7, 110, 26, size, ID_SIZE);
		m_wScore = ROOT_AE3_UI.Text(GetContent(), 232, 11, w - 240, 20, "", 15, ROOT_AE3_UI.Rgb(0.6, 1, 0.6), false, false, 2);
		int cols = 32;
		int rows = 20;
		if (s_bLarge)
		{
			cols = 48;
			rows = 30;
		}

		if (cols != m_iCols || rows != m_iRows || m_aBody.IsEmpty())
		{
			m_iCols = cols;
			m_iRows = rows;
			Reset();
			m_bRunning = false;
		}

		float aw = w - 16;
		float ah = h - BAR - 8;
		m_fCell = Math.Max(4, Math.Floor(Math.Min(aw / m_iCols, ah / m_iRows)));
		float bw = m_fCell * m_iCols;
		float bh = m_fCell * m_iRows;
		m_fOffX = (w - bw) * 0.5;
		m_fOffY = BAR + (ah - bh) * 0.5;
		m_wCanvas = CanvasWidget.Cast(GetGame().GetWorkspace().CreateWidget(WidgetType.CanvasWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR, Color.FromInt(Color.WHITE), 1, GetContent()));
		ROOT_AE3_UI.Place(m_wCanvas, m_fOffX, m_fOffY, bw, bh);
		m_wInfo = ROOT_AE3_UI.Text(GetContent(), m_fOffX, m_fOffY + bh * 0.5 - 30, bw, 60, "", 18, ROOT_AE3_UI.Rgb(1, 1, 1), false, true, 1);
		m_wInfo.SetShadow(3, 0xFF000000, 0.9, 0, 1);
		Draw();
	}

	//------------------------------------------------------------------------------------------------
	protected void Reset()
	{
		m_aBody.Clear();
		int mid = (m_iRows / 2) * m_iCols + m_iCols / 2;
		for (int i = 0; i < 4; i++)
		{
			m_aBody.Insert(mid - i);
		}

		m_iDir = 1;
		m_iNextDir = 1;
		m_iScore = 0;
		m_bDead = false;
		m_fTimer = 0;
		PlaceFood();
	}

	//------------------------------------------------------------------------------------------------
	protected void PlaceFood()
	{
		for (int tries = 0; tries < 1000; tries++)
		{
			int c = Math.RandomInt(0, m_iCols * m_iRows);
			if (!m_aBody.Contains(c))
			{
				m_iFood = c;
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Start()
	{
		if (m_bDead || m_aBody.IsEmpty())
			Reset();

		m_bRunning = true;
		Draw();
	}

	//------------------------------------------------------------------------------------------------
	protected void Steer(int dir)
	{
		if (!m_bRunning || m_bDead)
		{
			Start();
			return;
		}

		if ((dir + 2) % 4 != m_iDir)
			m_iNextDir = dir;
	}

	//------------------------------------------------------------------------------------------------
	protected float StepTime()
	{
		return Math.Max(0.05, 0.13 - m_iScore * 0.002);
	}

	//------------------------------------------------------------------------------------------------
	override void Update(float dt)
	{
		if (!m_bRunning || m_bDead || !m_wCanvas)
			return;

		if (!IsFocused())
		{
			if (m_wInfo)
				m_wInfo.SetText("Paused - click the window to continue");

			return;
		}

		if (m_wInfo)
			m_wInfo.SetText("");

		m_fTimer += dt;
		if (m_fTimer < StepTime())
			return;

		m_fTimer = 0;
		m_iDir = m_iNextDir;
		int head = m_aBody[0];
		int r = head / m_iCols;
		int c = head % m_iCols;
		switch (m_iDir)
		{
			case 0: r--; break;
			case 1: c++; break;
			case 2: r++; break;
			case 3: c--; break;
		}

		int next = r * m_iCols + c;
		if (r < 0 || r >= m_iRows || c < 0 || c >= m_iCols || m_aBody.Contains(next))
		{
			m_bDead = true;
			m_bRunning = false;
			s_iBest = Math.Max(s_iBest, m_iScore);
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.ERROR);
			Draw();
			return;
		}

		m_aBody.InsertAt(next, 0);
		if (next == m_iFood)
		{
			m_iScore++;
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.CLICK);
			PlaceFood();
		}
		else
		{
			m_aBody.Remove(m_aBody.Count() - 1);
		}

		Draw();
	}

	//------------------------------------------------------------------------------------------------
	protected void Cell(int idx, int color, float inset)
	{
		int col = idx % m_iCols;
		int row = idx / m_iCols;
		float x = col * m_fCell + inset;
		float y = row * m_fCell + inset;
		float s = m_fCell - inset * 2;
		PolygonDrawCommand p = new PolygonDrawCommand();
		p.m_iColor = color;
		array<float> v = {x, y, x + s, y, x + s, y + s, x, y + s};
		p.m_Vertices = ROOT_AE3_UI.ToPixels(v);
		m_aCommands.Insert(p);
	}

	//------------------------------------------------------------------------------------------------
	protected void Draw()
	{
		if (!m_wCanvas)
			return;

		m_aCommands.Clear();
		float w = m_fCell * m_iCols;
		float h = m_fCell * m_iRows;
		PolygonDrawCommand bg = new PolygonDrawCommand();
		bg.m_iColor = 0xFF14201A;
		array<float> v = {0, 0, w, 0, w, h, 0, h};
		bg.m_Vertices = ROOT_AE3_UI.ToPixels(v);
		m_aCommands.Insert(bg);
		if (m_iFood >= 0)
			Cell(m_iFood, 0xFFFF5050, m_fCell * 0.15);

		foreach (int i, int b : m_aBody)
		{
			if (i == 0)
				Cell(b, 0xFFC8FFC8, 1);
			else
				Cell(b, 0xFF46D046, 1);
		}

		m_wCanvas.SetDrawCommands(m_aCommands);
		if (m_wScore)
			m_wScore.SetText(string.Format("Score %1     Best %2", m_iScore, Math.Max(s_iBest, m_iScore)));

		if (!m_wInfo)
			return;

		if (m_bDead)
			m_wInfo.SetText(string.Format("Game over - score %1\nPress Enter or New game", m_iScore));
		else if (!m_bRunning)
			m_wInfo.SetText("Press an arrow key or Enter to start\nArrow keys or WASD steer");
		else
			m_wInfo.SetText("");
	}

	//------------------------------------------------------------------------------------------------
	override bool OnKey(KeyCode key, bool ctrl)
	{
		switch (key)
		{
			case KeyCode.KC_UP:
			case KeyCode.KC_W:
				Steer(0);
				return true;

			case KeyCode.KC_RIGHT:
			case KeyCode.KC_D:
				Steer(1);
				return true;

			case KeyCode.KC_DOWN:
			case KeyCode.KC_S:
				Steer(2);
				return true;

			case KeyCode.KC_LEFT:
			case KeyCode.KC_A:
				Steer(3);
				return true;

			case KeyCode.KC_RETURN:
			case KeyCode.KC_NUMPADENTER:
			case KeyCode.KC_SPACE:
			{
				if (!m_bRunning)
					Start();

				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (id == ID_NEW)
		{
			Reset();
			Start();
			return true;
		}

		if (id == ID_SIZE)
		{
			s_bLarge = !s_bLarge;
			Redraw();
			return true;
		}

		return false;
	}
}
