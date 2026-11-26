//! Shapes on one CanvasWidget (PolygonDrawCommand, LineDrawCommand). Colors are ARGB integers.
//! Polygons must be convex. Commit sends the commands to the canvas.
class ROOT_AE3_GamePaint
{
	protected CanvasWidget m_wCanvas;
	protected ref array<ref CanvasWidgetCommand> m_aCmds = {};

	//------------------------------------------------------------------------------------------------
	void ROOT_AE3_GamePaint(Widget parent, float x, float y, float w, float h)
	{
		m_wCanvas = CanvasWidget.Cast(GetGame().GetWorkspace().CreateWidget(WidgetType.CanvasWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR, Color.FromInt(Color.WHITE), 0, parent));
		if (m_wCanvas)
			ROOT_AE3_UI.Place(m_wCanvas, x, y, w, h);
	}

	//------------------------------------------------------------------------------------------------
	void Commit()
	{
		if (m_wCanvas)
			m_wCanvas.SetDrawCommands(m_aCmds);
	}

	//------------------------------------------------------------------------------------------------
	void Poly(notnull array<float> v, int color)
	{
		PolygonDrawCommand p = new PolygonDrawCommand();
		p.m_iColor = color;
		p.m_Vertices = v;
		m_aCmds.Insert(p);
	}

	//------------------------------------------------------------------------------------------------
	void Line(notnull array<float> v, float width, int color, bool close = false)
	{
		LineDrawCommand l = new LineDrawCommand();
		l.m_iColor = color;
		l.m_Vertices = v;
		l.m_fWidth = width;
		l.m_bShouldEnclose = close;
		m_aCmds.Insert(l);
	}

	//------------------------------------------------------------------------------------------------
	void Segment(float x1, float y1, float x2, float y2, float width, int color)
	{
		array<float> v = {x1, y1, x2, y2};
		Line(v, width, color);
	}

	//------------------------------------------------------------------------------------------------
	void Tri(float x1, float y1, float x2, float y2, float x3, float y3, int color)
	{
		array<float> v = {x1, y1, x2, y2, x3, y3};
		Poly(v, color);
	}

	//------------------------------------------------------------------------------------------------
	void Rect(float x, float y, float w, float h, int color)
	{
		array<float> v = {x, y, x + w, y, x + w, y + h, x, y + h};
		Poly(v, color);
	}

	//------------------------------------------------------------------------------------------------
	//! Outline of a rectangle, width inside the rectangle.
	void Frame(float x, float y, float w, float h, float width, int color)
	{
		Rect(x, y, w, width, color);
		Rect(x, y + h - width, w, width, color);
		Rect(x, y + width, width, h - width * 2, color);
		Rect(x + w - width, y + width, width, h - width * 2, color);
	}

	//------------------------------------------------------------------------------------------------
	//! Points of an ellipse turned by angle degrees.
	static array<float> EllipsePoints(float cx, float cy, float rx, float ry, float angle = 0, int seg = 28)
	{
		array<float> v = {};
		float a = angle * Math.DEG2RAD;
		float ca = Math.Cos(a);
		float sa = Math.Sin(a);
		for (int i = 0; i < seg; i++)
		{
			float t = i * Math.PI * 2 / seg;
			float px = Math.Cos(t) * rx;
			float py = Math.Sin(t) * ry;
			v.Insert(cx + px * ca - py * sa);
			v.Insert(cy + px * sa + py * ca);
		}

		return v;
	}

	//------------------------------------------------------------------------------------------------
	void Ellipse(float cx, float cy, float rx, float ry, int color, float angle = 0)
	{
		Poly(EllipsePoints(cx, cy, rx, ry, angle), color);
	}

	//------------------------------------------------------------------------------------------------
	void Circle(float cx, float cy, float r, int color)
	{
		int seg = 20;
		if (r > 12)
			seg = 32;

		Poly(EllipsePoints(cx, cy, r, r, 0, seg), color);
	}

	//------------------------------------------------------------------------------------------------
	void Ring(float cx, float cy, float r, float width, int color)
	{
		Line(EllipsePoints(cx, cy, r, r, 0, 32), width, color, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Filled circle with an edge.
	void Disc(float cx, float cy, float r, int fill, int edge, float edgeWidth = 1.5)
	{
		Circle(cx, cy, r, edge);
		Circle(cx, cy, r - edgeWidth, fill);
	}

	//------------------------------------------------------------------------------------------------
	//! Rectangle with rounded corners.
	void Round(float x, float y, float w, float h, float r, int color)
	{
		float rr = r;
		if (rr > w * 0.5)
			rr = w * 0.5;

		if (rr > h * 0.5)
			rr = h * 0.5;

		array<float> v = {};
		array<float> cxs = {x + w - rr, x + w - rr, x + rr, x + rr};
		array<float> cys = {y + rr, y + h - rr, y + h - rr, y + rr};
		array<float> start = {-90, 0, 90, 180};
		for (int c = 0; c < 4; c++)
		{
			for (int i = 0; i <= 5; i++)
			{
				float t = (start[c] + i * 18) * Math.DEG2RAD;
				v.Insert(cxs[c] + Math.Cos(t) * rr);
				v.Insert(cys[c] + Math.Sin(t) * rr);
			}
		}

		Poly(v, color);
	}

	//------------------------------------------------------------------------------------------------
	//! Convex shape given in unit coordinates (0-1) of the box x, y, s, with an outline.
	void Shape(float x, float y, float s, notnull array<float> unit, int fill, int edge, float edgeWidth = 1.2)
	{
		array<float> v = {};
		for (int i = 0; i + 1 < unit.Count(); i += 2)
		{
			v.Insert(x + unit[i] * s);
			v.Insert(y + unit[i + 1] * s);
		}

		Poly(v, fill);
		array<float> e = {};
		e.Copy(v);
		Line(e, edgeWidth, edge, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Ellipse in unit coordinates of the box x, y, s, with an outline.
	void UnitEllipse(float x, float y, float s, float ux, float uy, float rx, float ry, int fill, int edge, float edgeWidth = 1.2)
	{
		array<float> v = EllipsePoints(x + ux * s, y + uy * s, rx * s, ry * s);
		Poly(v, fill);
		array<float> e = {};
		e.Copy(v);
		Line(e, edgeWidth, edge, true);
	}
}
