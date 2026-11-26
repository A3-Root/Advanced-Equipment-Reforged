//! HTML page to rich text for the Browser: title, headings, paragraphs, line breaks, lists, bold, links and
//! images as links (absolute addresses). Scripts, styles and other markup are dropped.
class ROOT_AE3_HtmlText
{
	static const int MAX_IN = 400000;
	static const int MAX_OUT = 60000;

	//------------------------------------------------------------------------------------------------
	//! Absolute address of href on page base.
	static string Resolve(string base, string href)
	{
		string h = href.Trim();
		string low = h;
		low.ToLower();
		if (low.StartsWith("http://") || low.StartsWith("https://"))
			return h;

		int scheme = base.IndexOf("://");
		if (scheme < 0)
			return h;

		if (h.StartsWith("//"))
			return base.Substring(0, scheme + 1) + h;

		int hostEnd = base.IndexOfFrom(scheme + 3, "/");
		string origin = base;
		if (hostEnd > 0)
			origin = base.Substring(0, hostEnd);

		if (h.StartsWith("/"))
			return origin + h;

		string dir = origin + "/";
		if (hostEnd > 0)
		{
			string path = base;
			int q = path.IndexOf("?");
			if (q > 0)
				path = path.Substring(0, q);

			int slash = path.LastIndexOf("/");
			if (slash >= hostEnd)
				dir = path.Substring(0, slash + 1);
		}

		return dir + h;
	}

	//------------------------------------------------------------------------------------------------
	//! Value of attribute name in a tag body (quoted or bare), empty when missing.
	static string Attr(string tag, string name)
	{
		string low = tag;
		low.ToLower();
		int at = low.IndexOf(name + "=");
		if (at < 0)
			return "";

		int start = at + name.Length() + 1;
		if (start >= tag.Length())
			return "";

		string q = tag.Get(start);
		if (q == "\"" || q == "'")
		{
			int end = tag.IndexOfFrom(start + 1, q);
			if (end < 0)
				return "";

			return tag.Substring(start + 1, end - start - 1);
		}

		int stop = start;
		while (stop < tag.Length() && tag.Get(stop) != " " && tag.Get(stop) != ">")
		{
			stop++;
		}

		return tag.Substring(start, stop - start);
	}

	//------------------------------------------------------------------------------------------------
	protected static string Entities(string s)
	{
		string r = s;
		r.Replace("&nbsp;", " ");
		r.Replace("&quot;", "\"");
		r.Replace("&#39;", "'");
		r.Replace("&apos;", "'");
		r.Replace("&lt;", "(");
		r.Replace("&gt;", ")");
		r.Replace("&amp;", "&");
		r.Replace("\t", " ");
		r.Replace("\r", " ");
		r.Replace("\n", " ");
		while (r.Contains("  "))
		{
			r.Replace("  ", " ");
		}

		return r;
	}

	//------------------------------------------------------------------------------------------------
	//! Converts html; title receives the page title.
	static string Convert(string html, string base, out string title)
	{
		title = "";
		string src = html;
		if (src.Length() > MAX_IN)
			src = src.Substring(0, MAX_IN);

		string outText;
		int pos;
		int n = src.Length();
		bool inTitle;
		string skipUntil;
		bool inLink;
		while (pos < n && outText.Length() < MAX_OUT)
		{
			int lt = src.IndexOfFrom(pos, "<");
			if (lt < 0)
				lt = n;

			if (lt > pos && skipUntil.IsEmpty())
			{
				string text = Entities(src.Substring(pos, lt - pos));
				if (inTitle)
					title += text;
				else if (!text.Trim().IsEmpty() || (!outText.IsEmpty() && !outText.EndsWith(">")))
					outText += text;
			}

			if (lt >= n)
				break;

			if (src.ContainsAt("<!--", lt))
			{
				int endComment = src.IndexOfFrom(lt + 4, "-->");
				if (endComment < 0)
					break;

				pos = endComment + 3;
				continue;
			}

			int gt = src.IndexOfFrom(lt, ">");
			if (gt < 0)
				break;

			string tag = src.Substring(lt + 1, gt - lt - 1);
			pos = gt + 1;
			string name = tag;
			int space = name.IndexOf(" ");
			if (space > 0)
				name = name.Substring(0, space);

			name.ToLower();
			name.Replace("/", "");
			bool closing = tag.StartsWith("/");
			if (!skipUntil.IsEmpty())
			{
				if (closing && name == skipUntil)
					skipUntil = "";

				continue;
			}

			switch (name)
			{
				case "script":
				case "style":
				case "noscript":
				case "svg":
				{
					if (!closing && !tag.EndsWith("/"))
						skipUntil = name;

					break;
				}

				case "title": inTitle = !closing; break;
				case "br": outText += "<br/>"; break;
				case "p":
				case "div":
				case "tr":
				case "section":
				case "article":
				case "ul":
				case "ol":
				case "table":
				{
					if (!outText.EndsWith("<br/>"))
						outText += "<br/>";

					break;
				}

				case "li":
				{
					if (!closing)
						outText += "<br/>  - ";

					break;
				}

				case "h1":
				case "h2":
				{
					if (closing)
						outText += "</h2><br/>";
					else
						outText += "<br/><h2>";

					break;
				}

				case "h3":
				case "h4":
				case "b":
				case "strong":
				{
					if (closing)
						outText += "</b>";
					else
						outText += "<b>";

					break;
				}

				case "a":
				{
					if (closing)
					{
						if (inLink)
							outText += "</a>";

						inLink = false;
						break;
					}

					string href = Attr(tag, "href");
					if (href.IsEmpty() || href.StartsWith("#") || href.StartsWith("javascript:") || inLink)
						break;

					outText += "<a href=\"" + Resolve(base, href) + "\">";
					inLink = true;
					break;
				}

				case "img":
				case "video":
				case "source":
				{
					string media = Attr(tag, "src");
					if (media.IsEmpty() || inLink)
						break;

					string alt = Attr(tag, "alt");
					string label = "image";
					if (name != "img")
						label = "video";

					if (!alt.IsEmpty())
						label += ": " + Entities(alt);

					outText += " <a href=\"" + Resolve(base, media) + "\">[" + label + "]</a> ";
					break;
				}
			}
		}

		if (inLink)
			outText += "</a>";

		title = title.Trim();
		return outText;
	}
}
