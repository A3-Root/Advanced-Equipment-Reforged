//! Client: media from web addresses (http, https) and base64 text (optionally a data: URI).
//! Images (PNG, JPEG, GIF) and videos are saved under $profile:ROOT_AE3/media/ and shown from local storage
//! (ImageWidget.LoadImageTexture, VideoWidget.LoadVideo). Pages are returned as text.
//! Results go to target.OnDialog(TAG or TAG_PAGE, ok, file path / page text, or the error).
class ROOT_AE3_WebMedia
{
	static const string TAG = "webmedia";
	static const string TAG_PAGE = "webpage";
	static const string DIR = "$profile:ROOT_AE3/media";
	static const int MAX_BYTES = 8388608;
	static const int MAX_BASE64 = 2097152;

	protected static ref map<string, string> s_mCache;
	protected static ref array<ref ROOT_AE3_WebMediaJob> s_aJobs;
	protected static ref array<int> s_aDecode;
	protected static ref array<string> s_aVideoExt;
	protected static ref array<string> s_aImageExt;

	//------------------------------------------------------------------------------------------------
	//! Media kind of a source: "video" for addresses of video files, "url" for other http(s) addresses,
	//! "image" for texture resources, else "base64".
	static string KindOf(string source)
	{
		string s = source.Trim();
		string low = s;
		low.ToLower();
		if (low.StartsWith("http://") || low.StartsWith("https://"))
		{
			if (IsVideoAddress(low))
				return "video";

			return "url";
		}

		if (s.StartsWith("{") || s.EndsWith(".edds"))
			return "image";

		return "base64";
	}

	//------------------------------------------------------------------------------------------------
	//! Lowercase extension of the address path, without query or fragment.
	static string Extension(string url)
	{
		string path = url;
		path.ToLower();
		int cut = path.IndexOf("?");
		if (cut >= 0)
			path = path.Substring(0, cut);

		cut = path.IndexOf("#");
		if (cut >= 0)
			path = path.Substring(0, cut);

		int dot = path.LastIndexOf(".");
		int slash = path.LastIndexOf("/");
		if (dot < 0 || dot < slash)
			return "";

		return path.Substring(dot + 1, path.Length() - dot - 1);
	}

	//------------------------------------------------------------------------------------------------
	static bool IsVideoAddress(string url)
	{
		if (!s_aVideoExt)
			s_aVideoExt = {"mp4", "webm", "ogv", "mov", "m4v", "avi", "mkv", "bk2", "bik"};

		return s_aVideoExt.Contains(Extension(url));
	}

	//------------------------------------------------------------------------------------------------
	static bool IsImageAddress(string url)
	{
		if (!s_aImageExt)
			s_aImageExt = {"png", "jpg", "jpeg", "gif", "b64", "base64"};

		return s_aImageExt.Contains(Extension(url));
	}

	//------------------------------------------------------------------------------------------------
	static bool IsWeb(string kind)
	{
		return kind == "url" || kind == "base64" || kind == "video";
	}

	//------------------------------------------------------------------------------------------------
	//! Web access allowed by the server (setting m_bDesktopWebImages).
	static bool Allowed()
	{
		return ROOT_AE3_SettingsManager.Get().m_bDesktopWebImages;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Id(string kind, string source)
	{
		return string.Format("%1_%2_%3", kind, Math.AbsInt(source.Hash()), source.Length());
	}

	//------------------------------------------------------------------------------------------------
	protected static void Init()
	{
		if (!s_mCache)
			s_mCache = new map<string, string>();

		if (!s_aJobs)
			s_aJobs = {};
	}

	//------------------------------------------------------------------------------------------------
	//! Image (kind url or base64) or video (kind video) as a local file.
	static void Request(string kind, string source, ROOT_AE3_UiListener target)
	{
		Init();
		string id = Id(kind, source);
		string path = s_mCache.Get(id);
		if (!path.IsEmpty())
		{
			Reply(target, TAG, true, path);
			return;
		}

		if (kind == "base64")
		{
			string err;
			path = SaveBase64(id, source, err);
			if (path.IsEmpty())
			{
				Reply(target, TAG, false, err);
				return;
			}

			s_mCache.Set(id, path);
			Reply(target, TAG, true, path);
			return;
		}

		if (kind != "url" && kind != "video")
		{
			Reply(target, TAG, false, "Not a web address.");
			return;
		}

		Start(id, kind, source, target);
	}

	//------------------------------------------------------------------------------------------------
	//! Text of a web page (no cache).
	static void RequestPage(string url, ROOT_AE3_UiListener target)
	{
		Init();
		Start(Id("page", url) + "_" + System.GetTickCount().ToString(), "page", url, target);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Reply(ROOT_AE3_UiListener target, string tag, bool ok, string value)
	{
		if (target)
			target.OnDialog(tag, ok, value);
	}

	//------------------------------------------------------------------------------------------------
	protected static void Start(string id, string mode, string source, ROOT_AE3_UiListener target)
	{
		string tag = TAG;
		if (mode == "page")
			tag = TAG_PAGE;

		if (!Allowed())
		{
			Reply(target, tag, false, "Web access is switched off on this server.");
			return;
		}

		foreach (ROOT_AE3_WebMediaJob j : s_aJobs)
		{
			if (j.m_sId == id)
			{
				j.m_aTargets.Insert(target);
				return;
			}
		}

		string url = source.Trim();
		int scheme = url.IndexOf("://");
		if (scheme < 0)
		{
			Reply(target, tag, false, "Enter an address starting with http:// or https://.");
			return;
		}

		int slash = url.IndexOfFrom(scheme + 3, "/");
		string server = url + "/";
		string request = "";
		if (slash > 0)
		{
			server = url.Substring(0, slash + 1);
			request = url.Substring(slash + 1, url.Length() - slash - 1);
		}

		RestContext ctx = GetGame().GetRestApi().GetContext(server);
		if (!ctx)
		{
			Reply(target, tag, false, "That web address cannot be reached.");
			return;
		}

		ROOT_AE3_WebMediaJob job = new ROOT_AE3_WebMediaJob();
		job.m_sId = id;
		job.m_sMode = mode;
		job.m_sTag = tag;
		job.m_sExt = Extension(url);
		job.m_Ctx = ctx;
		job.m_aTargets.Insert(target);
		job.m_Cb = new RestCallback();
		job.m_Cb.SetOnSuccess(job.OnSuccess);
		job.m_Cb.SetOnError(job.OnError);
		s_aJobs.Insert(job);
		ctx.SetTimeout(30);
		ctx.GET(job.m_Cb, request);
	}

	//------------------------------------------------------------------------------------------------
	//! Download finished. Images: base64 text or the raw file. Videos: the raw file. Pages: the text.
	static void Finish(ROOT_AE3_WebMediaJob job, bool ok, string data, string error)
	{
		string result = error;
		bool success;
		if (ok && job.m_sMode == "page")
		{
			result = data;
			success = true;
		}
		else if (ok)
		{
			string path;
			string trimmed = data.Trim();
			if (job.m_sMode != "video" && (trimmed.StartsWith("data:") || LooksBase64(trimmed)))
			{
				path = SaveBase64(job.m_sId, trimmed, error);
			}
			else
			{
				array<int> bytes = {};
				int n = data.Length();
				if (n > MAX_BYTES)
					n = MAX_BYTES;

				for (int i = 0; i < n; i++)
				{
					bytes.Insert(data.ToAscii(i) & 0xFF);
				}

				string ext = "";
				if (job.m_sMode == "video")
				{
					ext = VideoFormat(bytes);
					if (ext.IsEmpty())
						ext = job.m_sExt;
				}

				path = SaveBytes(job.m_sId, bytes, ext, error);
			}

			if (!path.IsEmpty())
			{
				s_mCache.Set(job.m_sId, path);
				result = path;
				success = true;
			}
			else
			{
				result = error;
			}
		}

		foreach (ROOT_AE3_UiListener t : job.m_aTargets)
		{
			Reply(t, job.m_sTag, success, result);
		}

		// The job owns the callback that is running: removed on the next frame.
		GetGame().GetCallqueue().CallLater(RemoveJob, 0, false, job.m_sId);
	}

	//------------------------------------------------------------------------------------------------
	protected static void RemoveJob(string id)
	{
		for (int i = s_aJobs.Count() - 1; i >= 0; i--)
		{
			if (s_aJobs[i].m_sId == id)
				s_aJobs.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static bool LooksBase64(string s)
	{
		if (s.Length() < 16)
			return false;

		Table();
		int n = s.Length();
		if (n > 256)
			n = 256;

		for (int i = 0; i < n; i++)
		{
			int ch = s.ToAscii(i) & 0xFF;
			if (ch >= 128 || (s_aDecode[ch] < 0 && ch != 61 && ch != 10 && ch != 13))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Table()
	{
		if (s_aDecode)
			return;

		s_aDecode = {};
		s_aDecode.Resize(128);
		for (int i = 0; i < 128; i++)
		{
			s_aDecode[i] = -1;
		}

		string abc = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		for (int i = 0; i < 64; i++)
		{
			s_aDecode[abc.ToAscii(i)] = i;
		}

		s_aDecode[45] = 62;	// '-' url-safe
		s_aDecode[95] = 63;	// '_' url-safe
	}

	//------------------------------------------------------------------------------------------------
	//! Base64 (whitespace ignored, data: prefix removed) to bytes. False on an invalid character.
	static bool DecodeBase64(string text, notnull array<int> bytes)
	{
		Table();
		string s = text;
		int comma = s.IndexOf(",");
		if (s.StartsWith("data:") && comma > 0)
			s = s.Substring(comma + 1, s.Length() - comma - 1);

		int acc;
		int bits;
		int n = s.Length();
		for (int i = 0; i < n; i++)
		{
			int ch = s.ToAscii(i) & 0xFF;
			if (ch == 61)
				break;

			if (ch == 32 || ch == 9 || ch == 10 || ch == 13)
				continue;

			if (ch >= 128 || s_aDecode[ch] < 0)
				return false;

			acc = ((acc << 6) | s_aDecode[ch]) & 0xFFFFFF;
			bits += 6;
			if (bits >= 8)
			{
				bits -= 8;
				bytes.Insert((acc >> bits) & 0xFF);
				if (bytes.Count() > MAX_BASE64)
					return false;
			}
		}

		return !bytes.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	protected static string SaveBase64(string id, string text, out string error)
	{
		array<int> bytes = {};
		if (!DecodeBase64(text, bytes))
		{
			error = "The image text is not valid base64 or is larger than 2 MB.";
			return "";
		}

		return SaveBytes(id, bytes, "", error);
	}

	//------------------------------------------------------------------------------------------------
	//! Image file extension from the first bytes: png, jpg, gif; empty when unknown.
	static string Format(notnull array<int> b)
	{
		if (b.Count() >= 4 && b[0] == 0x89 && b[1] == 0x50 && b[2] == 0x4E && b[3] == 0x47)
			return "png";

		if (b.Count() >= 3 && b[0] == 0xFF && b[1] == 0xD8 && b[2] == 0xFF)
			return "jpg";

		if (b.Count() >= 3 && b[0] == 0x47 && b[1] == 0x49 && b[2] == 0x46)
			return "gif";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Video file extension from the first bytes: mp4 (ftyp box), webm (EBML), bk2 (Bink 2); empty when unknown.
	static string VideoFormat(notnull array<int> b)
	{
		if (b.Count() >= 8 && b[4] == 0x66 && b[5] == 0x74 && b[6] == 0x79 && b[7] == 0x70)
			return "mp4";

		if (b.Count() >= 4 && b[0] == 0x1A && b[1] == 0x45 && b[2] == 0xDF && b[3] == 0xA3)
			return "webm";

		if (b.Count() >= 3 && b[0] == 0x4B && b[1] == 0x42 && b[2] == 0x32)
			return "bk2";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! ext empty: image, checked by its first bytes.
	protected static string SaveBytes(string id, notnull array<int> bytes, string ext, out string error)
	{
		if (ext.IsEmpty())
		{
			ext = Format(bytes);
			if (ext.IsEmpty())
			{
				error = "Not a PNG, JPEG or GIF image.";
				return "";
			}
		}

		if (!ROOT_AE3_Filesystem.ValidName(ext))
		{
			error = "Unknown file type.";
			return "";
		}

		FileIO.MakeDirectory(DIR);
		string path = DIR + "/" + id + "." + ext;
		FileHandle fh = FileIO.OpenFile(path, FileMode.WRITE);
		if (!fh)
		{
			error = "The file could not be stored.";
			return "";
		}

		foreach (int v : bytes)
		{
			fh.Write(v, 1);
		}

		fh.Close();
		return path;
	}

	//------------------------------------------------------------------------------------------------
	//! Video player for a local file: VideoWidget.LoadVideo, then Play. Null when the engine refuses the file.
	static VideoWidget CreateVideo(Widget parent, float x, float y, float w, float h, string file, bool repeat)
	{
		VideoWidget v = VideoWidget.Cast(GetGame().GetWorkspace().CreateWidget(WidgetType.VideoWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.STRETCH | WidgetFlags.IGNORE_CURSOR, Color.FromInt(Color.WHITE), 0, parent));
		if (!v)
			return null;

		FrameSlot.SetPos(v, x, y);
		FrameSlot.SetSize(v, w, h);
		if (!v.LoadVideo(file, 0))
		{
			v.RemoveFromHierarchy();
			return null;
		}

		if (repeat)
			v.Play(VideoCommand.REPEAT);

		v.Play(VideoCommand.PLAY);
		return v;
	}
}

//! One running download of ROOT_AE3_WebMedia.
class ROOT_AE3_WebMediaJob
{
	string m_sId;
	string m_sMode;		//!< image (url), video, page
	string m_sTag;
	string m_sExt;
	RestContext m_Ctx;
	ref RestCallback m_Cb;
	ref array<ROOT_AE3_UiListener> m_aTargets = {};

	//------------------------------------------------------------------------------------------------
	void OnSuccess(RestCallback cb)
	{
		ROOT_AE3_WebMedia.Finish(this, true, cb.GetData(), "");
	}

	//------------------------------------------------------------------------------------------------
	void OnError(RestCallback cb)
	{
		int code = cb.GetHttpCode();
		ROOT_AE3_WebMedia.Finish(this, false, "", string.Format("The download failed (HTTP %1).", code));
	}
}
