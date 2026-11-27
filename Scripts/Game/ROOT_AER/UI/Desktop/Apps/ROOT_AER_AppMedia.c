//! Image and video viewer. Media files (AER_MEDIA|<kind>|<source>|<caption>, kind image, url, base64 or video),
//! web addresses and pasted base64 text. Open... and Save as... use the file dialog; Set as background changes
//! the wallpaper (images).
class ROOT_AER_AppMedia : ROOT_AER_App
{
	protected static const int ID_OPEN = 1;
	protected static const int ID_URL = 2;
	protected static const int ID_BASE64 = 3;
	protected static const int ID_SAVE = 4;
	protected static const int ID_WALL = 5;
	protected static const int ID_PLAY = 6;
	protected static const int ID_STOP = 7;
	protected static const int ID_REWIND = 8;

	protected FrameWidget m_wView;
	protected VideoWidget m_wVideo;
	protected string m_sPath;		//!< file shown, empty for an unsaved web image
	protected string m_sKind;		//!< image, url, base64
	protected string m_sSource;		//!< texture, address or base64 text
	protected string m_sCaption;
	protected string m_sLocal;		//!< downloaded or decoded file
	protected string m_sMessage;
	protected float m_fViewW;
	protected float m_fViewH;

	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Photos";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "photos";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(820, 580, 0);
	}

	//------------------------------------------------------------------------------------------------
	override bool NeedsFileBrowsing()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Build()
	{
		float w = Width();
		float h = Height();
		Color clear = ROOT_AER_UI.Rgb(0, 0, 0, 0);
		ROOT_AER_UI.Rect(GetContent(), 0, 0, w, 40, ROOT_AER_Theme.Bar());
		ROOT_AER_UI.ItemButton(GetContent(), 6, 6, 90, 28, "g_open", "Open...", ID_OPEN, this, 14, ROOT_AER_Theme.Text(), clear, ROOT_AER_Theme.Hover());
		ROOT_AER_UI.ItemButton(GetContent(), 100, 6, 130, 28, "browser", "Web address...", ID_URL, this, 14, ROOT_AER_Theme.Text(), clear, ROOT_AER_Theme.Hover());
		ROOT_AER_UI.ItemButton(GetContent(), 234, 6, 130, 28, "file_text", "Paste base64...", ID_BASE64, this, 14, ROOT_AER_Theme.Text(), clear, ROOT_AER_Theme.Hover());
		ROOT_AER_UI.ItemButton(GetContent(), 368, 6, 110, 28, "g_newfile", "Save as...", ID_SAVE, this, 14, ROOT_AER_Theme.Text(), clear, ROOT_AER_Theme.Hover());
		ROOT_AER_UI.ItemButton(GetContent(), 482, 6, 170, 28, "settings", "Set as background", ID_WALL, this, 14, ROOT_AER_Theme.Text(), clear, ROOT_AER_Theme.Hover());
		ROOT_AER_UI.Rect(GetContent(), 0, 40, w, 1, ROOT_AER_Theme.Border());
		m_fViewW = w;
		m_fViewH = h - 41;
		m_wView = ROOT_AER_UI.Frame(GetContent(), 0, 41, m_fViewW, m_fViewH);
		if (m_sPath.IsEmpty() && m_sSource.IsEmpty() && !m_sArg.IsEmpty())
		{
			m_sPath = m_sArg;
			Request1("fs_media", m_sPath);
		}

		Show();
	}

	//------------------------------------------------------------------------------------------------
	protected void Show()
	{
		ROOT_AER_UI.Clear(m_wView);
		m_wVideo = null;
		float w = m_fViewW;
		float h = m_fViewH;
		ROOT_AER_UI.Rect(m_wView, 0, 0, w, h, ROOT_AER_UI.Rgb(0.03, 0.03, 0.03));
		bool web = ROOT_AER_WebMedia.IsWeb(m_sKind);
		if (m_sSource.IsEmpty() || (web && m_sLocal.IsEmpty()))
		{
			string msg = m_sMessage;
			if (msg.IsEmpty() && m_sSource.IsEmpty())
				msg = "Open a media file, a web address (image or video) or base64 image text.";
			else if (msg.IsEmpty())
				msg = "Loading...";

			ROOT_AER_UI.Icon(m_wView, (w - 64) * 0.5, h * 0.5 - 70, 64, "photos");
			ROOT_AER_UI.Text(m_wView, 20, h * 0.5 + 4, w - 40, 44, msg, 15, ROOT_AER_UI.Rgb(0.8, 0.8, 0.8), false, true, 1);
			return;
		}

		float capH = 0;
		if (!m_sCaption.IsEmpty())
			capH = 30;

		if (m_sKind == "video")
		{
			float barH = 36;
			m_wVideo = ROOT_AER_WebMedia.CreateVideo(m_wView, 0, 0, w, h - capH - barH, m_sLocal, false);
			if (!m_wVideo)
				ROOT_AER_UI.Text(m_wView, 20, h * 0.4, w - 40, 66, "The game cannot play this video file. The video player of the engine takes few formats and none is documented.", 15, ROOT_AER_UI.Rgb(0.8, 0.8, 0.8), false, true, 1);

			float by = h - capH - barH + 4;
			Color clear = ROOT_AER_UI.Rgb(0, 0, 0, 0);
			ROOT_AER_UI.ItemButton(m_wView, 8, by, 80, 28, "g_forward", "Play", ID_PLAY, this, 14, ROOT_AER_UI.Rgb(1, 1, 1), clear, ROOT_AER_Theme.Hover());
			ROOT_AER_UI.ItemButton(m_wView, 92, by, 80, 28, "g_close", "Stop", ID_STOP, this, 14, ROOT_AER_UI.Rgb(1, 1, 1), clear, ROOT_AER_Theme.Hover());
			ROOT_AER_UI.ItemButton(m_wView, 176, by, 100, 28, "g_back", "Restart", ID_REWIND, this, 14, ROOT_AER_UI.Rgb(1, 1, 1), clear, ROOT_AER_Theme.Hover());
			if (capH > 0)
			{
				ROOT_AER_UI.Rect(m_wView, 0, h - capH, w, capH, ROOT_AER_UI.Rgb(0, 0, 0, 0.7));
				ROOT_AER_UI.Text(m_wView, 12, h - capH + 6, w - 24, 20, m_sCaption, 14, ROOT_AER_UI.Rgb(1, 1, 1));
			}

			return;
		}

		ImageWidget img = ROOT_AER_UI.Picture(m_wView, 0, 0, w, h - capH, "");
		bool loaded;
		if (web)
			loaded = img.LoadImageTexture(0, m_sLocal, true, true);
		else
			loaded = img.LoadImageTexture(0, m_sSource);

		if (!loaded)
		{
			img.SetVisible(false);
			ROOT_AER_UI.Text(m_wView, 20, h * 0.5, w - 40, 44, "This image could not be displayed.", 15, ROOT_AER_UI.Rgb(0.8, 0.8, 0.8), false, true, 1);
		}

		int iw, ih;
		img.GetImageSize(0, iw, ih);
		if (loaded && iw > 0 && ih > 0)
		{
			float fw = iw;
			float fh = ih;
			float scale = Math.Min(w / fw, (h - capH) / fh);
			FrameSlot.SetSize(img, fw * scale, fh * scale);
			FrameSlot.SetPos(img, (w - fw * scale) * 0.5, (h - capH - fh * scale) * 0.5);
		}

		if (capH > 0)
		{
			ROOT_AER_UI.Rect(m_wView, 0, h - capH, w, capH, ROOT_AER_UI.Rgb(0, 0, 0, 0.7));
			ROOT_AER_UI.Text(m_wView, 12, h - capH + 6, w - 24, 20, m_sCaption, 14, ROOT_AER_UI.Rgb(1, 1, 1));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetImage(string kind, string source, string caption, string title)
	{
		m_sKind = kind;
		m_sSource = source;
		m_sCaption = caption;
		m_sLocal = "";
		m_sMessage = "";
		if (m_Window)
			m_Window.SetTitle(title + " - Photos");

		if (ROOT_AER_WebMedia.IsWeb(kind))
			ROOT_AER_WebMedia.Request(kind, source, this);

		Show();
	}

	//------------------------------------------------------------------------------------------------
	override void OnDialog(string tag, bool ok, string value)
	{
		if (tag == ROOT_AER_WebMedia.TAG)
		{
			if (ok)
				m_sLocal = value;
			else
				m_sMessage = value;

			Show();
			return;
		}

		string text = value;
		text.TrimInPlace();
		if (!ok || text.IsEmpty())
			return;

		switch (tag)
		{
			case "open":
			{
				m_sPath = value;
				Request1("fs_media", m_sPath);
				return;
			}

			case "url":
			{
				string url = text;
				string kind = ROOT_AER_WebMedia.KindOf(url);
				if (kind != "url" && kind != "video")
				{
					Status("Enter an address starting with http:// or https://.");
					return;
				}

				m_sPath = "";
				SetImage(kind, url, "", url);
				return;
			}

			case "base64":
			{
				m_sPath = "";
				SetImage("base64", text, "", "Pasted image");
				return;
			}

			case "save":
			{
				m_sPath = value;
				Request2("fs_save", value, ROOT_AER_Intel.Media(m_sKind, m_sSource, m_sCaption));
				if (m_Window)
					m_Window.SetTitle(ROOT_AER_Filesystem.BaseName(value) + " - Photos");

				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (!ok)
		{
			Status(Arg(payload, 0));
			return;
		}

		if (cmd == "fs_save")
		{
			Status(Arg(payload, 0));
			return;
		}

		if (cmd != "fs_media")
			return;

		string kind = Arg(payload, 3);
		if (kind.IsEmpty())
			kind = "image";

		SetImage(kind, Arg(payload, 1), Arg(payload, 2), ROOT_AER_Filesystem.BaseName(m_sPath));
		Status(m_sPath);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (!m_Desktop)
			return false;

		string folder = ROOT_AER_Filesystem.HomeOf(ROOT_AER_Client.Get().m_sUser);
		if (!m_sPath.IsEmpty())
			folder = ROOT_AER_Filesystem.Parent(m_sPath);

		switch (id)
		{
			case ID_PLAY:
			{
				if (m_wVideo)
					m_wVideo.Play(VideoCommand.PLAY);

				return true;
			}

			case ID_STOP:
			{
				if (m_wVideo)
					m_wVideo.Play(VideoCommand.STOP);

				return true;
			}

			case ID_REWIND:
			{
				if (m_wVideo)
				{
					m_wVideo.Play(VideoCommand.REWIND);
					m_wVideo.Play(VideoCommand.PLAY);
				}

				return true;
			}

			case ID_OPEN: m_Desktop.ShowFilePicker(this, "open", "Open image", folder, false, "", "Open"); return true;
			case ID_URL: m_Desktop.ShowDialog(this, "url", "Web address", "Address of an image (PNG, JPEG, GIF, base64 text) or a video (mp4, webm, ...):", true, "https://", "Open"); return true;
			case ID_BASE64: m_Desktop.ShowDialog(this, "base64", "Base64 image", "Paste base64 image text (a data:image/...;base64, prefix is allowed):", true, "", "Show"); return true;
			case ID_SAVE:
			{
				if (m_sSource.IsEmpty())
				{
					Status("Nothing to save.");
					return true;
				}

				m_Desktop.ShowFilePicker(this, "save", "Save image as", folder, true, "image.img", "Save");
				return true;
			}

			case ID_WALL:
			{
				string wall = m_sPath;
				if (wall.IsEmpty() && m_sKind == "url")
					wall = m_sSource;

				if (m_sKind == "video")
				{
					Status("Only images can be the background.");
					return true;
				}

				if (wall.IsEmpty())
				{
					Status("Save the image to a file first.");
					return true;
				}

				ROOT_AER_Client c = ROOT_AER_Client.Get();
				Request2("sys_set", "wallpaper", wall);
				m_Desktop.ApplyPrefs(c.m_sTheme, wall);
				return true;
			}
		}

		return false;
	}
}
