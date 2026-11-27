//! Widget construction helpers for the script-built desktop and terminal. Positions in reference pixels
//! inside FrameWidget parents.
class ROOT_AE3_UI
{
	static const ResourceName FONT = "{3E7733BAC8C831F6}UI/Fonts/RobotoCondensed/RobotoCondensed_Regular.fnt";
	static const ResourceName FONT_MONO = "{0E041C5B1F27DCEA}ui/fonts/robotomono_msdf_28.fnt";
	static const ResourceName ICONS = "{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset";
	static const ResourceName LAYOUT_EDIT = "{CC1D9D6F84A4BB01}UI/layouts/ROOT_AE3/ROOT_AE3_EditBox.layout";
	static const ResourceName LAYOUT_EDIT_MONO = "{738CBA095FA40085}UI/layouts/ROOT_AE3/ROOT_AE3_EditBoxMono.layout";
	static const ResourceName LAYOUT_MULTI = "{D3F6811D286EDCC2}UI/layouts/ROOT_AE3/ROOT_AE3_MultiEdit.layout";
	protected static ref array<EditBoxWidget> s_aPlaceEdits = {};
	protected static ref array<TextWidget> s_aPlaceTexts = {};

	//------------------------------------------------------------------------------------------------
	protected static WorkspaceWidget Ws()
	{
		return GetGame().GetWorkspace();
	}

	//------------------------------------------------------------------------------------------------
	static void Place(Widget w, float x, float y, float width, float height)
	{
		FrameSlot.SetAnchorMin(w, 0, 0);
		FrameSlot.SetAnchorMax(w, 0, 0);
		FrameSlot.SetPos(w, x, y);
		FrameSlot.SetSize(w, width, height);
	}

	//------------------------------------------------------------------------------------------------
	//! Fills the parent with margins.
	static void Fill(Widget w, float left = 0, float top = 0, float right = 0, float bottom = 0)
	{
		FrameSlot.SetAnchorMin(w, 0, 0);
		FrameSlot.SetAnchorMax(w, 1, 1);
		FrameSlot.SetOffsets(w, left, top, right, bottom);
	}

	//------------------------------------------------------------------------------------------------
	//! Container only: ignores the cursor so widgets behind it stay clickable; its children receive events.
	static FrameWidget Frame(Widget parent, float x, float y, float width, float height, bool clip = true)
	{
		int flags = WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR;
		if (clip)
			flags |= WidgetFlags.CLIPCHILDREN;

		FrameWidget f = FrameWidget.Cast(Ws().CreateWidget(WidgetType.FrameWidgetTypeID, flags, Color.FromInt(Color.WHITE), 0, parent));
		Place(f, x, y, width, height);
		return f;
	}

	//------------------------------------------------------------------------------------------------
	//! Solid rectangle. Receives clicks when id >= 0 and a listener is given.
	static ImageWidget Rect(Widget parent, float x, float y, float width, float height, Color color, int id = -1, ROOT_AE3_UiListener listener = null)
	{
		int flags = WidgetFlags.VISIBLE | WidgetFlags.BLEND | WidgetFlags.STRETCH;
		if (id < 0)
			flags |= WidgetFlags.IGNORE_CURSOR;

		ImageWidget img = ImageWidget.Cast(Ws().CreateWidget(WidgetType.ImageWidgetTypeID, flags, color, 0, parent));
		Place(img, x, y, width, height);
		if (id >= 0 && listener)
			Listen(img, id, listener);

		return img;
	}

	//------------------------------------------------------------------------------------------------
	//! align: 0 left, 1 center, 2 right. Tactical style: monospace, 2 sizes smaller.
	static TextWidget Text(Widget parent, float x, float y, float width, float height, string text, int size = 16, Color color = null, bool mono = false, bool wrap = false, int align = 0)
	{
		ref Color col = color;
		if (!col)
			col = Color.FromInt(Color.WHITE);

		if (ROOT_AE3_Theme.IsTactical())
		{
			if (!mono)
				size = Math.Max(10, size - 2);

			mono = true;
		}

		TextWidget t = TextWidget.Cast(Ws().CreateWidget(WidgetType.TextWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.BLEND, col, 0, parent));
		Place(t, x, y, width, height);
		if (mono)
			t.SetFont(FONT_MONO);
		else
			t.SetFont(FONT);

		t.SetExactFontSize(size);
		t.SetTextWrapping(wrap);
		t.ClearFlags(WidgetFlags.CENTER | WidgetFlags.RALIGN);
		if (align == 1)
			t.SetFlags(WidgetFlags.CENTER);
		else if (align == 2)
			t.SetFlags(WidgetFlags.RALIGN);

		t.SetText(text);
		return t;
	}

	//------------------------------------------------------------------------------------------------
	static RichTextWidget Rich(Widget parent, float x, float y, float width, float height, string text, int size = 16, Color color = null)
	{
		ref Color col = color;
		if (!col)
			col = Color.FromInt(Color.WHITE);

		RichTextWidget t = RichTextWidget.Cast(Ws().CreateWidget(WidgetType.RichTextWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.BLEND, col, 0, parent));
		Place(t, x, y, width, height);
		t.SetFont(FONT);
		t.SetExactFontSize(size);
		t.SetTextWrapping(true);
		t.SetText(text);
		return t;
	}

	//------------------------------------------------------------------------------------------------
	//! Icon from ROOT_AE3_Icons, else from icons_wrapperUI-64.imageset.
	static ImageWidget Icon(Widget parent, float x, float y, float size, string name, Color color = null)
	{
		ref Color col = color;
		if (!col)
			col = Color.FromInt(Color.WHITE);

		ImageWidget img = ImageWidget.Cast(Ws().CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.BLEND | WidgetFlags.STRETCH | WidgetFlags.IGNORE_CURSOR, col, 0, parent));
		Place(img, x, y, size, size);
		if (ROOT_AE3_Icons.Has(name))
			img.LoadImageTexture(0, ROOT_AE3_Icons.Get(name));
		else
			img.LoadImageFromSet(0, ICONS, name);

		return img;
	}

	//------------------------------------------------------------------------------------------------
	//! Image from a texture resource (.edds).
	static ImageWidget Picture(Widget parent, float x, float y, float width, float height, ResourceName texture)
	{
		ImageWidget img = ImageWidget.Cast(Ws().CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.BLEND | WidgetFlags.STRETCH | WidgetFlags.IGNORE_CURSOR, Color.FromInt(Color.WHITE), 0, parent));
		Place(img, x, y, width, height);
		if (!texture.IsEmpty())
			img.LoadImageTexture(0, texture);

		return img;
	}

	//------------------------------------------------------------------------------------------------
	//! Clickable area that highlights on hover and press. Transparent backgrounds get the theme hover color.
	static ImageWidget Hit(Widget parent, float x, float y, float width, float height, int id, ROOT_AE3_UiListener listener, Color bg = null, Color hover = null)
	{
		ref Color back = bg;
		if (!back)
			back = ROOT_AE3_Theme.Button();

		ref Color over = hover;
		if (!over && back.A() < 0.1)
			over = ROOT_AE3_Theme.Hover();

		ImageWidget b = Rect(parent, x, y, width, height, back);
		b.ClearFlags(WidgetFlags.IGNORE_CURSOR);
		ROOT_AE3_WidgetHandler h = Listen(b, id, listener);
		if (h)
			h.SetButton(back, over);

		return b;
	}

	//------------------------------------------------------------------------------------------------
	//! Clickable button: background rectangle plus centered label; highlights on hover and press.
	//! Tactical style: 1 px outline, upper case label.
	static ImageWidget Button(Widget parent, float x, float y, float width, float height, string label, int id, ROOT_AE3_UiListener listener, Color bg = null, Color fg = null, int size = 15, Color hover = null)
	{
		string text = label;
		ImageWidget b;
		if (ROOT_AE3_Theme.IsTactical())
		{
			text.ToUpper();
			float hair = Hair();
			Rect(parent, x, y, width, height, ROOT_AE3_Theme.Border());
			b = Hit(parent, x + hair, y + hair, width - hair * 2, height - hair * 2, id, listener, bg, hover);
		}
		else
		{
			b = Hit(parent, x, y, width, height, id, listener, bg, hover);
		}

		ref Color col = fg;
		if (!col)
			col = ROOT_AE3_Theme.Text();

		TextWidget t = Text(parent, x + 4, y, width - 8, height, text, size, col, false, false, 1);
		t.SetTextOffset(0, (height - size) * 0.5 - 2);
		return b;
	}

	//------------------------------------------------------------------------------------------------
	//! Button with a centered icon only.
	static ImageWidget IconButton(Widget parent, float x, float y, float width, float height, string icon, int id, ROOT_AE3_UiListener listener, float iconSize = 16, Color tint = null, Color bg = null, Color hover = null)
	{
		ref Color back = bg;
		if (!back)
			back = Rgb(0, 0, 0, 0);

		ImageWidget b = Hit(parent, x, y, width, height, id, listener, back, hover);
		ref Color col = tint;
		if (!col && icon.StartsWith("g_"))
			col = ROOT_AE3_Theme.Text();

		Icon(parent, x + (width - iconSize) * 0.5, y + (height - iconSize) * 0.5, iconSize, icon, col);
		return b;
	}

	//------------------------------------------------------------------------------------------------
	//! Button with an icon and a left aligned label (menus, toolbars, navigation pane).
	static ImageWidget ItemButton(Widget parent, float x, float y, float width, float height, string icon, string label, int id, ROOT_AE3_UiListener listener, int size = 14, Color fg = null, Color bg = null, Color hover = null)
	{
		ref Color back = bg;
		if (!back)
			back = Rgb(0, 0, 0, 0);

		ImageWidget b = Hit(parent, x, y, width, height, id, listener, back, hover);
		ref Color col = fg;
		if (!col)
			col = ROOT_AE3_Theme.Text();

		float tx = x + 10;
		if (!icon.IsEmpty())
		{
			float s = Math.Min(18, height - 10);
			ref Color tint = null;
			if (icon.StartsWith("g_"))
				tint = col;

			Icon(parent, x + 10, y + (height - s) * 0.5, s, icon, tint);
			tx = x + 18 + s;
		}

		TextWidget t = Text(parent, tx, y, x + width - tx - 4, height, label, size, col);
		t.SetTextOffset(0, (height - size) * 0.5 - 2);
		return b;
	}

	//------------------------------------------------------------------------------------------------
	//! Filled rectangle with a 1 px border.
	static ImageWidget Box(Widget parent, float x, float y, float width, float height, Color fill, Color border = null)
	{
		ref Color edge = border;
		if (!edge)
			edge = ROOT_AE3_Theme.Border();

		float hair = Hair();
		Rect(parent, x, y, width, height, edge);
		return Rect(parent, x + hair, y + hair, width - hair * 2, height - hair * 2, fill);
	}

	//------------------------------------------------------------------------------------------------
	//! Width in reference units of at least one screen pixel (1-unit lines vanish when the UI is scaled below 1).
	static float Hair()
	{
		float scale = GetGame().GetWorkspace().DPIScale(1);
		if (scale <= 0 || scale >= 1)
			return 1;

		return Math.Ceil(1 / scale);
	}

	//------------------------------------------------------------------------------------------------
	//! Soft drop shadow around a rectangle; draw before it.
	static void Shadow(Widget parent, float x, float y, float width, float height, float spread = 8)
	{
		if (ROOT_AE3_Theme.IsTactical())
			return;

		for (int i = 1; i <= 4; i++)
		{
			float d = spread * i / 4;
			Rect(parent, x - d, y - d + spread * 0.4, width + d * 2, height + d * 2, Rgb(0, 0, 0, 0.07));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Hint text shown while the edit box is empty (UpdatePlaceholders, called every frame by the desktop).
	static void Placeholder(EditBoxWidget edit, Widget parent, float x, float y, float width, string text, Color color = null)
	{
		if (!edit)
			return;

		TextWidget t = Text(parent, x, y, width, 20, text, 14, color);
		s_aPlaceEdits.Insert(edit);
		s_aPlaceTexts.Insert(t);
	}

	//------------------------------------------------------------------------------------------------
	static void UpdatePlaceholders()
	{
		for (int i = s_aPlaceEdits.Count() - 1; i >= 0; i--)
		{
			EditBoxWidget e = s_aPlaceEdits[i];
			TextWidget t = s_aPlaceTexts[i];
			if (!e || !t)
			{
				s_aPlaceEdits.Remove(i);
				s_aPlaceTexts.Remove(i);
				continue;
			}

			t.SetVisible(e.GetText().IsEmpty());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! True while an edit box has keyboard focus in write mode.
	static bool Typing()
	{
		Widget f = Ws().GetFocusedWidget();
		EditBoxWidget e = EditBoxWidget.Cast(f);
		if (e && e.IsInWriteMode())
			return true;

		MultilineEditBoxWidget m = MultilineEditBoxWidget.Cast(f);
		return m && m.IsInWriteMode();
	}

	//------------------------------------------------------------------------------------------------
	static EditBoxWidget Edit(Widget parent, float x, float y, float width, float height, string text, int id, ROOT_AE3_UiListener listener, bool password = false, bool mono = false)
	{
		ResourceName layout = LAYOUT_EDIT;
		if (mono || ROOT_AE3_Theme.IsTactical())
			layout = LAYOUT_EDIT_MONO;

		Widget root = Ws().CreateWidgets(layout, parent);
		if (!root)
			return null;

		Place(root, x, y, width, height);
		EditBoxWidget e = EditBoxWidget.Cast(root.FindAnyWidget("Edit"));
		if (!e)
			return null;

		e.SetText(text);
		if (password)
			e.SetObfuscationChar("*");

		Listen(e, id, listener);
		return e;
	}

	//------------------------------------------------------------------------------------------------
	static MultilineEditBoxWidget MultiEdit(Widget parent, float x, float y, float width, float height, string text, int id, ROOT_AE3_UiListener listener)
	{
		Widget root = Ws().CreateWidgets(LAYOUT_MULTI, parent);
		if (!root)
			return null;

		Place(root, x, y, width, height);
		MultilineEditBoxWidget e = MultilineEditBoxWidget.Cast(root.FindAnyWidget("Edit"));
		if (!e)
			return null;

		e.SetText(text);
		Listen(e, id, listener);
		return e;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the text of an EditBoxWidget or MultilineEditBoxWidget without raising OnUiChange.
	static void SetEdit(Widget w, string text)
	{
		ROOT_AE3_WidgetHandler.s_bQuiet = true;
		EditBoxWidget e = EditBoxWidget.Cast(w);
		if (e)
			e.SetText(text);

		MultilineEditBoxWidget m = MultilineEditBoxWidget.Cast(w);
		if (m)
			m.SetText(text);

		ROOT_AE3_WidgetHandler.s_bQuiet = false;
	}

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_WidgetHandler Listen(Widget w, int id, ROOT_AE3_UiListener listener)
	{
		if (!w || !listener)
			return null;

		w.SetUserID(id);
		ROOT_AE3_WidgetHandler h = new ROOT_AE3_WidgetHandler();
		h.m_Listener = listener;
		h.m_wSelf = w;
		w.AddHandler(h);
		listener.KeepHandler(h);
		return h;
	}

	//------------------------------------------------------------------------------------------------
	static void Clear(Widget w)
	{
		if (!w)
			return;

		Widget c = w.GetChildren();
		while (c)
		{
			Widget next = c.GetSibling();
			c.RemoveFromHierarchy();
			c = next;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! CanvasWidget vertices are physical pixels; widget positions are reference units.
	//! Returns a copy of v (reference units) in pixels.
	static array<float> ToPixels(notnull array<float> v)
	{
		float k = GetGame().GetWorkspace().DPIScale(1000) / 1000;
		array<float> res = {};
		foreach (float f : v)
		{
			res.Insert(f * k);
		}

		return res;
	}

	//------------------------------------------------------------------------------------------------
	static float ToPixel(float value)
	{
		return GetGame().GetWorkspace().DPIScale(value);
	}

	//------------------------------------------------------------------------------------------------
	//! Mouse position in reference units.
	static void Mouse(out float x, out float y)
	{
		int px, py;
		WidgetManager.GetMousePos(px, py);
		x = Ws().DPIUnscale(px);
		y = Ws().DPIUnscale(py);
	}

	//------------------------------------------------------------------------------------------------
	//! Screen size in reference units.
	static void ScreenSize(out float w, out float h)
	{
		w = Ws().DPIUnscale(Ws().GetWidth());
		h = Ws().DPIUnscale(Ws().GetHeight());
	}

	//------------------------------------------------------------------------------------------------
	static Color Rgb(float r, float g, float b, float a = 1)
	{
		return new Color(r, g, b, a);
	}
}
