//! Image viewer for media files (AE3_MEDIA|image|<texture>|<caption>).
class ROOT_AE3_AppMedia : ROOT_AE3_App
{
	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Image Viewer";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "camera";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(720, 520, 0);
	}

	//------------------------------------------------------------------------------------------------
	override bool NeedsFileBrowsing()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected EditBoxWidget m_wPath;
	protected FrameWidget m_wView;

	override void Build()
	{
		float w = Width();
		m_wPath = Field(6, 6, w - 100, m_sArg, 1);
		Btn(w - 88, 6, 82, 26, "Open", 2);
		m_wView = ROOT_AE3_UI.Frame(GetContent(), 6, 38, w - 12, Height() - 44);
		if (!m_sArg.IsEmpty())
			Request1("fs_media", m_sArg);
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (!ok)
		{
			Status(Arg(payload, 0));
			return;
		}

		ROOT_AE3_UI.Clear(m_wView);
		float w = FrameSlot.GetSizeX(m_wView);
		float h = FrameSlot.GetSizeY(m_wView);
		ROOT_AE3_UI.Rect(m_wView, 0, 0, w, h, ROOT_AE3_UI.Rgb(0, 0, 0));
		ImageWidget img = ROOT_AE3_UI.Picture(m_wView, 0, 0, w, h - 26, Arg(payload, 1));
		int iw, ih;
		img.GetImageSize(0, iw, ih);
		if (iw > 0 && ih > 0)
		{
			float scale = Math.Min(w / iw, (h - 26) / ih);
			FrameSlot.SetSize(img, iw * scale, ih * scale);
			FrameSlot.SetPos(img, (w - iw * scale) * 0.5, (h - 26 - ih * scale) * 0.5);
		}

		ROOT_AE3_UI.Text(m_wView, 6, h - 24, w - 12, 20, Arg(payload, 2), 14);
		Status(Arg(payload, 0));
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (id == 2)
		{
			Request1("fs_media", m_wPath.GetText());
			return true;
		}

		return false;
	}
}
