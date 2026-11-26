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
	static TextWidget Text(Widget parent, float x, float y, float width, float height, string text, int size = 16, Color color = null, bool mono = false, bool wrap = false)
	{
		ref Color col = color;
		if (!col)
			col = Color.FromInt(Color.WHITE);

		TextWidget t = TextWidget.Cast(Ws().CreateWidget(WidgetType.TextWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.BLEND, col, 0, parent));
		Place(t, x, y, width, height);
		if (mono)
			t.SetFont(FONT_MONO);
		else
			t.SetFont(FONT);

		t.SetExactFontSize(size);
		t.SetTextWrapping(wrap);
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
	static ImageWidget Icon(Widget parent, float x, float y, float size, string name, Color color = null)
	{
		ref Color col = color;
		if (!col)
			col = Color.FromInt(Color.WHITE);

		ImageWidget img = ImageWidget.Cast(Ws().CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.BLEND | WidgetFlags.STRETCH | WidgetFlags.IGNORE_CURSOR, col, 0, parent));
		Place(img, x, y, size, size);
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
	//! Clickable button: background rectangle plus centered label; highlights on hover and press.
	static ImageWidget Button(Widget parent, float x, float y, float width, float height, string label, int id, ROOT_AE3_UiListener listener, Color bg = null, Color fg = null, int size = 15)
	{
		ref Color back = bg;
		if (!back)
			back = new Color(0.25, 0.27, 0.3, 1);

		ImageWidget b = Rect(parent, x, y, width, height, back);
		b.ClearFlags(WidgetFlags.IGNORE_CURSOR);
		ROOT_AE3_WidgetHandler h = Listen(b, id, listener);
		if (h)
			h.SetButton(back);

		TextWidget t = Text(parent, x + 4, y, width - 8, height, label, size, fg);
		t.SetTextOffset(0, (height - size) * 0.5 - 2);
		return b;
	}

	//------------------------------------------------------------------------------------------------
	static EditBoxWidget Edit(Widget parent, float x, float y, float width, float height, string text, int id, ROOT_AE3_UiListener listener, bool password = false, bool mono = false)
	{
		ResourceName layout = LAYOUT_EDIT;
		if (mono)
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
