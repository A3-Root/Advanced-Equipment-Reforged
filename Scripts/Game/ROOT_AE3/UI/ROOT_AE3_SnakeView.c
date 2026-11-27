//! Retro Snake on a pixel canvas (80x50, or 40x25 with --big). Client only.
//! Steering: arrow keys / WASD through the menu navigation actions, or the on-screen arrows. Esc quits.
class ROOT_AE3_SnakeView : ROOT_AE3_UiListener
{
	protected static const int ID_UP = 1;
	protected static const int ID_DOWN = 2;
	protected static const int ID_LEFT = 3;
	protected static const int ID_RIGHT = 4;
	protected static const int ID_RESTART = 5;

	protected FrameWidget m_wRoot;
	protected CanvasWidget m_wCanvas;
	protected TextWidget m_wInfo;
	protected ref array<ref CanvasWidgetCommand> m_aCommands = {};
	protected ref array<int> m_aBody = {};
	protected int m_iCols = 80;
	protected int m_iRows = 50;
	protected float m_fCell;
	protected int m_iDir = 1;		//!< 0 up, 1 right, 2 down, 3 left
	protected int m_iNextDir = 1;
	protected int m_iFood = -1;
	protected int m_iScore;
	protected bool m_bDead;
	protected bool m_bSplash = true;
	protected float m_fTimer;
	protected float m_fStep = 0.08;

	//------------------------------------------------------------------------------------------------
	void ROOT_AE3_SnakeView(Widget parent, bool big)
	{
		if (big)
		{
			m_iCols = 40;
			m_iRows = 25;
			m_fStep = 0.12;
		}

		float sw, sh;
		ROOT_AE3_UI.ScreenSize(sw, sh);
		m_fCell = Math.Floor(Math.Min(sw * 0.8 / m_iCols, sh * 0.75 / m_iRows));
		float w = m_fCell * m_iCols;
		float h = m_fCell * m_iRows;
		float x = (sw - w) * 0.5;
		float y = (sh - h) * 0.5;
		m_wRoot = ROOT_AE3_UI.Frame(parent, 0, 0, sw, sh);
		ROOT_AE3_UI.Rect(m_wRoot, 0, 0, sw, sh, ROOT_AE3_UI.Rgb(0, 0, 0, 0.92));
		m_wCanvas = CanvasWidget.Cast(GetGame().GetWorkspace().CreateWidget(WidgetType.CanvasWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR, Color.FromInt(Color.WHITE), 1, m_wRoot));
		ROOT_AE3_UI.Place(m_wCanvas, x, y, w, h);
		m_wInfo = ROOT_AE3_UI.Text(m_wRoot, x, y - 34, w, 28, "", 20, ROOT_AE3_UI.Rgb(0.4, 1, 0.4), true);
		float by = y + h + 10;
		ROOT_AE3_UI.Button(m_wRoot, x, by, 60, 30, "Left", ID_LEFT, this);
		ROOT_AE3_UI.Button(m_wRoot, x + 66, by, 60, 30, "Up", ID_UP, this);
		ROOT_AE3_UI.Button(m_wRoot, x + 132, by, 60, 30, "Down", ID_DOWN, this);
		ROOT_AE3_UI.Button(m_wRoot, x + 198, by, 60, 30, "Right", ID_RIGHT, this);
		ROOT_AE3_UI.Button(m_wRoot, x + w - 100, by, 100, 30, "Restart", ID_RESTART, this);
		Reset();
	}

	//------------------------------------------------------------------------------------------------
	void Destroy()
	{
		ReleaseHandlers();
		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();
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
		PlaceFood();
		Draw();
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
	protected void Steer(int dir)
	{
		if (m_bSplash)
		{
			m_bSplash = false;
			return;
		}

		if ((dir + 2) % 4 != m_iDir)
			m_iNextDir = dir;
	}

	//------------------------------------------------------------------------------------------------
	protected void ReadKeys()
	{
		InputManager im = GetGame().GetInputManager();
		if (im.GetActionTriggered("MenuUp"))
			Steer(0);
		else if (im.GetActionTriggered("MenuDown"))
			Steer(2);
		else if (im.GetActionTriggered("MenuLeft"))
			Steer(3);
		else if (im.GetActionTriggered("MenuRight"))
			Steer(1);
		else if (im.GetActionTriggered("MenuSelect") && (m_bDead || m_bSplash))
		{
			m_bSplash = false;
			Reset();
		}
	}

	//------------------------------------------------------------------------------------------------
	void Update(float dt)
	{
		ReadKeys();
		if (m_bSplash || m_bDead)
		{
			Draw();
			return;
		}

		m_fTimer += dt;
		if (m_fTimer < m_fStep)
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
	protected void Cell(int idx, int color)
	{
		int col = idx % m_iCols;
		int row = idx / m_iCols;
		float x = col * m_fCell;
		float y = row * m_fCell;
		PolygonDrawCommand p = new PolygonDrawCommand();
		p.m_iColor = color;
		array<float> v = {x, y, x + m_fCell - 1, y, x + m_fCell - 1, y + m_fCell - 1, x, y + m_fCell - 1};
		p.m_Vertices = ROOT_AE3_UI.ToPixels(v);
		m_aCommands.Insert(p);
	}

	//------------------------------------------------------------------------------------------------
	protected void Draw()
	{
		m_aCommands.Clear();
		PolygonDrawCommand bg = new PolygonDrawCommand();
		float w = m_fCell * m_iCols;
		float h = m_fCell * m_iRows;
		bg.m_iColor = 0xFF101810;
		array<float> v = {0, 0, w, 0, w, h, 0, h};
		bg.m_Vertices = ROOT_AE3_UI.ToPixels(v);
		m_aCommands.Insert(bg);
		if (m_iFood >= 0)
			Cell(m_iFood, 0xFFFF4040);

		foreach (int i, int b : m_aBody)
		{
			if (i == 0)
				Cell(b, 0xFFB0FFB0);
			else
				Cell(b, 0xFF40D040);
		}

		m_wCanvas.SetDrawCommands(m_aCommands);
		if (m_bSplash)
			m_wInfo.SetText("SNAKE - press an arrow key to start, Esc to quit");
		else if (m_bDead)
			m_wInfo.SetText(string.Format("GAME OVER - score %1 - Enter or Restart to play again", m_iScore));
		else
			m_wInfo.SetText(string.Format("SNAKE   score %1", m_iScore));
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		switch (id)
		{
			case ID_UP: Steer(0); return true;
			case ID_RIGHT: Steer(1); return true;
			case ID_DOWN: Steer(2); return true;
			case ID_LEFT: Steer(3); return true;
			case ID_RESTART:
			{
				m_bSplash = false;
				Reset();
				return true;
			}
		}

		return false;
	}
}
