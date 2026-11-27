//! Classic ciphers and encodings for encrypted intel files, crypto and crack. Algorithm ids are stored with files
//! (ROOT_AE3_FileEntry.m_iEncryption). Keys by algorithm: Caesar shift, Columnar / Vigenere keyword, ROT variant
//! (rot5, rot13, rot18, rot47), Affine "a,b" (a coprime with 26), Rail fence rails, Bacon "extended" (26 letters,
//! else 24), Substitution 26-letter alphabet, Integer radix (2, 8, 10, 16). Text is treated as bytes.
class ROOT_AE3_Encryption
{
	static const int NONE = 0;
	static const int CAESAR = 1;
	static const int COLUMNAR = 2;
	static const int VIGENERE = 3;
	static const int ROT = 4;
	static const int AFFINE = 5;
	static const int RAILFENCE = 6;
	static const int MORSE = 7;
	static const int SPELLING = 8;
	static const int BACON = 9;
	static const int SUBSTITUTION = 10;
	static const int BASE32 = 11;
	static const int BASE64 = 12;
	static const int ASCII85 = 13;
	static const int UNICODE = 14;
	static const int INTEGER = 15;
	static const int COUNT = 16;

	protected static const string UPPER = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	protected static const string B64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	protected static const string B32 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
	protected static const string DIGITS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	protected static ref array<string> s_aMorse = {".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--",
		"-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..",
		"-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----."};
	protected static ref array<string> s_aNato = {"Alpha", "Bravo", "Charlie", "Delta", "Echo", "Foxtrot", "Golf", "Hotel", "India", "Juliett",
		"Kilo", "Lima", "Mike", "November", "Oscar", "Papa", "Quebec", "Romeo", "Sierra", "Tango", "Uniform", "Victor", "Whiskey", "Xray",
		"Yankee", "Zulu", "Zero", "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine"};

	//------------------------------------------------------------------------------------------------
	//! Command name of an algorithm (crypto -a NAME).
	static string Name(int algo)
	{
		array<string> names = {"none", "caesar", "columnar", "vigenere", "rot", "affine", "railfence", "morse", "spelling", "bacon", "substitution", "base32", "base64", "ascii85", "unicode", "integer"};
		if (algo < 0 || algo >= names.Count())
			return "none";

		return names[algo];
	}

	//------------------------------------------------------------------------------------------------
	//! Algorithm id of a name, -1 when unknown.
	static int FromName(string name)
	{
		string n = name;
		n.ToLower();
		for (int i = 0; i < COUNT; i++)
		{
			if (Name(i) == n)
				return i;
		}

		if (n == "alpha_sub")
			return SUBSTITUTION;

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Display name (Game Master, World Editor).
	static string Label(int algo)
	{
		array<string> labels = {"None", "Caesar (shift)", "Columnar transposition (keyword)", "Vigenere (keyword)", "ROT (rot5 / rot13 / rot18 / rot47)",
			"Affine (a,b)", "Rail fence (rails)", "Morse code", "Spelling alphabet (NATO)", "Bacon", "Alphabet substitution", "Base32", "Base64",
			"Ascii85", "Unicode notation (U+0041)", "Integer (radix)"};
		if (algo < 0 || algo >= labels.Count())
			return "None";

		return labels[algo];
	}

	//------------------------------------------------------------------------------------------------
	//! What the key of an algorithm is; empty when it takes none.
	static string KeyHint(int algo)
	{
		switch (algo)
		{
			case CAESAR: return "Shift, a whole number (e.g. 3)";
			case COLUMNAR: return "Key word, 2 letters or more (e.g. KEY)";
			case VIGENERE: return "Key word (e.g. LEMON)";
			case ROT: return "Variant: rot5, rot13, rot18 or rot47";
			case AFFINE: return "a,b with a coprime with 26 (e.g. 5,8)";
			case RAILFENCE: return "Number of rails, 2 or more (e.g. 3)";
			case BACON: return "Empty = 24 letters, extended = 26 letters";
			case SUBSTITUTION: return "26-letter alphabet (e.g. QWERTYUIOPASDFGHJKLZXCVBNM)";
			case INTEGER: return "Radix: 2, 8, 10 or 16";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	static string DefaultKey(int algo)
	{
		switch (algo)
		{
			case CAESAR: return "3";
			case COLUMNAR: return "KEY";
			case VIGENERE: return "LEMON";
			case ROT: return "rot13";
			case AFFINE: return "5,8";
			case RAILFENCE: return "3";
			case SUBSTITUTION: return "QWERTYUIOPASDFGHJKLZXCVBNM";
			case INTEGER: return "16";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Empty when key suits the algorithm, else the reason.
	static string CheckKey(int algo, string key)
	{
		switch (algo)
		{
			case CAESAR: if (!ROOT_AE3_Util.IsInt(key.Trim())) return "the Caesar key is a whole number"; break;
			case COLUMNAR: if (key.Length() < 2) return "the columnar key needs 2 characters or more"; break;
			case VIGENERE: if (Letters(key).IsEmpty()) return "the Vigenere key needs letters"; break;
			case ROT: if (key != "" && key != "rot5" && key != "rot13" && key != "rot18" && key != "rot47") return "the ROT variant is rot5, rot13, rot18 or rot47"; break;
			case AFFINE:
			{
				int a, b;
				if (!AffineKey(key, a, b) || Gcd(a, 26) != 1)
					return "the Affine key is a,b with a coprime with 26 (1 3 5 7 9 11 15 17 19 21 23 25)";

				break;
			}
			case RAILFENCE: if (!ROOT_AE3_Util.IsInt(key.Trim()) || key.Trim().ToInt() < 2) return "the rail fence key is a number of rails, 2 or more"; break;
			case SUBSTITUTION: if (Letters(key).Length() != 26) return "the substitution key is a 26-letter alphabet"; break;
			case INTEGER:
			{
				int r = key.Trim().ToInt();
				if (key.Trim() != "" && r != 2 && r != 8 && r != 10 && r != 16)
					return "the integer radix is 2, 8, 10 or 16";

				break;
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Encrypts (or decrypts) text with an algorithm and key.
	static string Apply(int algo, string text, string key, bool decrypt = false)
	{
		switch (algo)
		{
			case CAESAR:
			{
				int shift = key.Trim().ToInt();
				if (decrypt)
					shift = -shift;

				return Caesar(text, shift);
			}

			case COLUMNAR: return Columnar(text, key, decrypt);
			case VIGENERE: return Vigenere(text, key, decrypt);
			case ROT: return Rot(text, key, decrypt);
			case AFFINE:
			{
				int a, b;
				if (!AffineKey(key, a, b))
					return text;

				return Affine(text, a, b, decrypt);
			}

			case RAILFENCE: return RailFence(text, key.Trim().ToInt(), decrypt);
			case MORSE: return Morse(text, decrypt);
			case SPELLING: return Spelling(text, decrypt);
			case BACON: return Bacon(text, key.Trim() == "extended", decrypt);
			case SUBSTITUTION: return Substitution(text, key, decrypt);
			case BASE32: return Base32(text, decrypt);
			case BASE64: return Base64(text, decrypt);
			case ASCII85: return Ascii85(text, decrypt);
			case UNICODE: return Unicode(text, decrypt);
			case INTEGER:
			{
				int radix = 16;
				if (key.Trim() != "")
					radix = key.Trim().ToInt();

				return Integer(text, radix, decrypt);
			}
		}

		return text;
	}

	// Helpers ----------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	protected static int Mod(int v, int m)
	{
		int r = v % m;
		if (r < 0)
			r += m;

		return r;
	}

	//------------------------------------------------------------------------------------------------
	static int Gcd(int a, int b)
	{
		a = Math.AbsInt(a);
		b = Math.AbsInt(b);
		while (b != 0)
		{
			int t = a % b;
			a = b;
			b = t;
		}

		return a;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool AffineKey(string key, out int a, out int b)
	{
		array<string> parts = {};
		key.Split(",", parts, true);
		if (parts.Count() != 2 || !ROOT_AE3_Util.IsInt(parts[0].Trim()) || !ROOT_AE3_Util.IsInt(parts[1].Trim()))
			return false;

		a = parts[0].Trim().ToInt();
		b = parts[1].Trim().ToInt();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Upper case letters of text.
	protected static string Letters(string text)
	{
		string r;
		for (int i = 0; i < text.Length(); i++)
		{
			int c = text.ToAscii(i);
			if (c >= 97 && c <= 122)
				c -= 32;

			if (c >= 65 && c <= 90)
				r += c.AsciiToString();
		}

		return r;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Char(int code)
	{
		return code.AsciiToString();
	}

	//------------------------------------------------------------------------------------------------
	protected static string ToBase(int value, int radix, int width)
	{
		string r;
		int v = value;
		if (v == 0)
			r = "0";

		while (v > 0)
		{
			r = DIGITS.Get(v % radix) + r;
			v = v / radix;
		}

		while (r.Length() < width)
		{
			r = "0" + r;
		}

		return r;
	}

	//------------------------------------------------------------------------------------------------
	protected static int FromBase(string value, int radix)
	{
		int r;
		string v = value;
		v.ToUpper();
		for (int i = 0; i < v.Length(); i++)
		{
			int d = DIGITS.IndexOf(v.Get(i));
			if (d >= 0 && d < radix)
				r = r * radix + d;
		}

		return r;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Bytes(string text, notnull array<int> bytes)
	{
		for (int i = 0; i < text.Length(); i++)
		{
			bytes.Insert(text.ToAscii(i) & 255);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static string FromBytes(notnull array<int> bytes)
	{
		string r;
		foreach (int b : bytes)
		{
			r += b.AsciiToString();
		}

		return r;
	}

	//------------------------------------------------------------------------------------------------
	//! Words of text split at spaces.
	protected static void Words(string text, notnull array<string> words)
	{
		array<string> raw = {};
		text.Split(" ", raw, true);
		foreach (string w : raw)
		{
			string t = w.Trim();
			if (!t.IsEmpty())
				words.Insert(t);
		}
	}

	// Ciphers ----------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Caesar shift of letters; decrypt with a negative key.
	static string Caesar(string text, int key)
	{
		string r;
		key = Mod(key, 26);
		for (int i = 0; i < text.Length(); i++)
		{
			int c = text.ToAscii(i);
			if (c >= 65 && c <= 90)
				c = 65 + (c - 65 + key) % 26;
			else if (c >= 97 && c <= 122)
				c = 97 + (c - 97 + key) % 26;

			r += c.AsciiToString();
		}

		return r;
	}

	//------------------------------------------------------------------------------------------------
	//! Columnar transposition with a keyword. Columns are read in the alphabetical order of the key letters.
	static string Columnar(string text, string key, bool decrypt = false)
	{
		int cols = key.Length();
		if (cols < 2)
			return text;

		array<int> order = ColumnOrder(key);
		int len = text.Length();
		int rows = Math.Ceil(len / (float)cols);
		if (!decrypt)
		{
			string r;
			foreach (int col : order)
			{
				for (int row = 0; row < rows; row++)
				{
					int idx = row * cols + col;
					if (idx < len)
						r += text.Get(idx);
				}
			}

			return r;
		}

		array<string> grid = {};
		grid.Resize(rows * cols);
		int pos;
		foreach (int col : order)
		{
			for (int row = 0; row < rows; row++)
			{
				int idx = row * cols + col;
				if (idx < len)
				{
					grid[idx] = text.Get(pos);
					pos++;
				}
			}
		}

		string plain;
		for (int i = 0; i < len; i++)
		{
			plain += grid[i];
		}

		return plain;
	}

	//------------------------------------------------------------------------------------------------
	protected static array<int> ColumnOrder(string key)
	{
		array<int> order = {};
		array<bool> used = {};
		for (int i = 0; i < key.Length(); i++)
		{
			used.Insert(false);
		}

		for (int n = 0; n < key.Length(); n++)
		{
			int best = -1;
			for (int i = 0; i < key.Length(); i++)
			{
				if (used[i])
					continue;

				if (best < 0 || key.ToAscii(i) < key.ToAscii(best))
					best = i;
			}

			used[best] = true;
			order.Insert(best);
		}

		return order;
	}

	//------------------------------------------------------------------------------------------------
	static string Vigenere(string text, string key, bool decrypt)
	{
		string k = Letters(key);
		if (k.IsEmpty())
			return text;

		string r;
		int index;
		for (int i = 0; i < text.Length(); i++)
		{
			int c = text.ToAscii(i);
			int base = -1;
			if (c >= 65 && c <= 90)
				base = 65;
			else if (c >= 97 && c <= 122)
				base = 97;

			if (base < 0)
			{
				r += c.AsciiToString();
				continue;
			}

			int shift = k.ToAscii(index % k.Length()) - 65;
			if (decrypt)
				shift = -shift;

			r += Char(base + Mod(c - base + shift, 26));
			index++;
		}

		return r;
	}

	//------------------------------------------------------------------------------------------------
	//! rot5 digits, rot13 letters, rot18 both, rot47 printable ASCII.
	static string Rot(string text, string variant, bool decrypt)
	{
		string v = variant.Trim();
		if (v.IsEmpty())
			v = "rot13";

		int sign = 1;
		if (decrypt)
			sign = -1;

		string r;
		for (int i = 0; i < text.Length(); i++)
		{
			int c = text.ToAscii(i);
			if (v == "rot47" && c >= 33 && c <= 126)
				c = 33 + Mod(c - 33 + sign * 47, 94);
			else if ((v == "rot5" || v == "rot18") && c >= 48 && c <= 57)
				c = 48 + Mod(c - 48 + sign * 5, 10);
			else if ((v == "rot13" || v == "rot18") && c >= 65 && c <= 90)
				c = 65 + Mod(c - 65 + sign * 13, 26);
			else if ((v == "rot13" || v == "rot18") && c >= 97 && c <= 122)
				c = 97 + Mod(c - 97 + sign * 13, 26);

			r += c.AsciiToString();
		}

		return r;
	}

	//------------------------------------------------------------------------------------------------
	static string Affine(string text, int a, int b, bool decrypt)
	{
		if (Gcd(a, 26) != 1)
			return text;

		int inv = 1;
		for (int i = 1; i < 26; i++)
		{
			if ((a * i) % 26 == 1)
				inv = i;
		}

		string r;
		for (int i = 0; i < text.Length(); i++)
		{
			int c = text.ToAscii(i);
			int base = -1;
			if (c >= 65 && c <= 90)
				base = 65;
			else if (c >= 97 && c <= 122)
				base = 97;

			if (base < 0)
			{
				r += c.AsciiToString();
				continue;
			}

			int x = c - base;
			int y;
			if (decrypt)
				y = Mod(inv * (x - b), 26);
			else
				y = Mod(a * x + b, 26);

			r += Char(base + y);
		}

		return r;
	}

	//------------------------------------------------------------------------------------------------
	static string RailFence(string text, int rails, bool decrypt)
	{
		int len = text.Length();
		if (rails < 2 || rails >= len)
			return text;

		array<int> pattern = {};
		int rail;
		int dir = 1;
		for (int i = 0; i < len; i++)
		{
			pattern.Insert(rail);
			if (rail == 0)
				dir = 1;

			if (rail == rails - 1)
				dir = -1;

			rail += dir;
		}

		if (!decrypt)
		{
			string r;
			for (int row = 0; row < rails; row++)
			{
				for (int i = 0; i < len; i++)
				{
					if (pattern[i] == row)
						r += text.Get(i);
				}
			}

			return r;
		}

		array<string> plain = {};
		plain.Resize(len);
		int pos;
		for (int row = 0; row < rails; row++)
		{
			for (int i = 0; i < len; i++)
			{
				if (pattern[i] == row)
				{
					plain[i] = text.Get(pos);
					pos++;
				}
			}
		}

		string joined;
		foreach (string s : plain)
		{
			joined += s;
		}

		return joined;
	}

	//------------------------------------------------------------------------------------------------
	//! Index of a letter or digit in the Morse / spelling tables (A-Z then 0-9), -1 otherwise.
	protected static int TableIndex(int c)
	{
		if (c >= 97 && c <= 122)
			c -= 32;

		if (c >= 65 && c <= 90)
			return c - 65;

		if (c >= 48 && c <= 57)
			return 26 + c - 48;

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	protected static string TableChar(int index)
	{
		if (index < 26)
			return Char(65 + index);

		return Char(48 + index - 26);
	}

	//------------------------------------------------------------------------------------------------
	//! Letters and digits as Morse code separated by spaces; word gaps are "/".
	static string Morse(string text, bool decrypt)
	{
		return Table(text, decrypt, s_aMorse, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Letters and digits as NATO spelling words separated by spaces; word gaps are "/".
	static string Spelling(string text, bool decrypt)
	{
		return Table(text, decrypt, s_aNato, true);
	}

	//------------------------------------------------------------------------------------------------
	protected static string Table(string text, bool decrypt, notnull array<string> table, bool ignoreCase)
	{
		if (!decrypt)
		{
			array<string> parts = {};
			for (int i = 0; i < text.Length(); i++)
			{
				int c = text.ToAscii(i);
				if (c == 32)
				{
					parts.Insert("/");
					continue;
				}

				int idx = TableIndex(c);
				if (idx >= 0)
					parts.Insert(table[idx]);
				else
					parts.Insert(c.AsciiToString());
			}

			return ROOT_AE3_Util.JoinFrom(parts, 0, " ");
		}

		array<string> words = {};
		Words(text, words);
		string r;
		foreach (string w : words)
		{
			if (w == "/" || w == "|")
			{
				r += " ";
				continue;
			}

			string probe = w;
			if (ignoreCase)
				probe.ToLower();

			for (int i = 0; i < table.Count(); i++)
			{
				string entry = table[i];
				if (ignoreCase)
					entry.ToLower();

				if (entry == probe)
				{
					r += TableChar(i);
					break;
				}
			}
		}

		return r;
	}

	//------------------------------------------------------------------------------------------------
	//! Letters as 5-letter A/B groups. Standard alphabet: 24 letters (I = J, U = V); extended: 26.
	static string Bacon(string text, bool extended, bool decrypt)
	{
		string alphabet = "ABCDEFGHIKLMNOPQRSTUWXYZ";
		if (extended)
			alphabet = UPPER;

		if (!decrypt)
		{
			array<string> parts = {};
			for (int i = 0; i < text.Length(); i++)
			{
				string u = text.Get(i);
				u.ToUpper();
				if (!extended && u == "J")
					u = "I";

				if (!extended && u == "V")
					u = "U";

				int idx = alphabet.IndexOf(u);
				if (idx < 0 || u.IsEmpty())
				{
					parts.Insert(text.Get(i));
					continue;
				}

				string bits = ToBase(idx, 2, 5);
				bits.Replace("0", "A");
				bits.Replace("1", "B");
				parts.Insert(bits);
			}

			return ROOT_AE3_Util.JoinFrom(parts, 0, " ");
		}

		string ab;
		for (int i = 0; i < text.Length(); i++)
		{
			string ch = text.Get(i);
			ch.ToUpper();
			if (ch == "A")
				ab += "0";
			else if (ch == "B")
				ab += "1";
		}

		string r;
		for (int i = 0; i + 5 <= ab.Length(); i += 5)
		{
			int idx = FromBase(ab.Substring(i, 5), 2);
			if (idx < alphabet.Length())
				r += alphabet.Get(idx);
		}

		return r;
	}

	//------------------------------------------------------------------------------------------------
	//! Letter i of the alphabet becomes letter i of key (26 letters); case is kept.
	static string Substitution(string text, string key, bool decrypt)
	{
		string subst = Letters(key);
		if (subst.Length() != 26)
			return text;

		string from = UPPER;
		string to = subst;
		if (decrypt)
		{
			from = subst;
			to = UPPER;
		}

		string r;
		for (int i = 0; i < text.Length(); i++)
		{
			int c = text.ToAscii(i);
			bool lower = c >= 97 && c <= 122;
			string u = text.Get(i);
			u.ToUpper();
			int idx = from.IndexOf(u);
			if (idx < 0 || u.IsEmpty() || (c < 65 || (c > 90 && !lower)))
			{
				r += text.Get(i);
				continue;
			}

			string o = to.Get(idx);
			if (lower)
				o.ToLower();

			r += o;
		}

		return r;
	}

	//------------------------------------------------------------------------------------------------
	static string Base64(string text, bool decrypt)
	{
		array<int> bytes = {};
		if (!decrypt)
		{
			Bytes(text, bytes);
			string r;
			for (int i = 0; i < bytes.Count(); i += 3)
			{
				int b1 = bytes[i];
				int b2;
				int b3;
				if (i + 1 < bytes.Count())
					b2 = bytes[i + 1];

				if (i + 2 < bytes.Count())
					b3 = bytes[i + 2];

				int n = (b1 << 16) | (b2 << 8) | b3;
				r += B64.Get((n >> 18) & 63);
				r += B64.Get((n >> 12) & 63);
				if (i + 1 < bytes.Count())
					r += B64.Get((n >> 6) & 63);
				else
					r += "=";

				if (i + 2 < bytes.Count())
					r += B64.Get(n & 63);
				else
					r += "=";
			}

			return r;
		}

		array<int> vals = {};
		for (int i = 0; i < text.Length(); i++)
		{
			int v = B64.IndexOf(text.Get(i));
			if (v >= 0)
				vals.Insert(v);
		}

		for (int i = 0; i + 1 < vals.Count(); i += 4)
		{
			int n = vals[i] << 18 | vals[i + 1] << 12;
			int have = 1;
			if (i + 2 < vals.Count())
			{
				n |= vals[i + 2] << 6;
				have = 2;
			}

			if (i + 3 < vals.Count())
			{
				n |= vals[i + 3];
				have = 3;
			}

			bytes.Insert((n >> 16) & 255);
			if (have >= 2)
				bytes.Insert((n >> 8) & 255);

			if (have >= 3)
				bytes.Insert(n & 255);
		}

		return FromBytes(bytes);
	}

	//------------------------------------------------------------------------------------------------
	static string Base32(string text, bool decrypt)
	{
		array<int> bytes = {};
		if (!decrypt)
		{
			Bytes(text, bytes);
			string bits;
			foreach (int b : bytes)
			{
				bits += ToBase(b, 2, 8);
			}

			string r;
			for (int i = 0; i < bits.Length(); i += 5)
			{
				int take = Math.Min(5, bits.Length() - i);
				string chunk = bits.Substring(i, take);
				while (chunk.Length() < 5)
				{
					chunk += "0";
				}

				r += B32.Get(FromBase(chunk, 2));
			}

			while (r.Length() % 8 != 0)
			{
				r += "=";
			}

			return r;
		}

		string bits2;
		for (int i = 0; i < text.Length(); i++)
		{
			string ch = text.Get(i);
			ch.ToUpper();
			int idx = B32.IndexOf(ch);
			if (idx >= 0 && !ch.IsEmpty())
				bits2 += ToBase(idx, 2, 5);
		}

		for (int i = 0; i + 8 <= bits2.Length(); i += 8)
		{
			bytes.Insert(FromBase(bits2.Substring(i, 8), 2));
		}

		return FromBytes(bytes);
	}

	//------------------------------------------------------------------------------------------------
	//! Divides a big-endian byte number by d in place; returns the remainder.
	protected static int DivBytes(notnull array<int> num, int d)
	{
		int rem;
		for (int i = 0; i < num.Count(); i++)
		{
			int cur = rem * 256 + num[i];
			num[i] = cur / d;
			rem = cur % d;
		}

		return rem;
	}

	//------------------------------------------------------------------------------------------------
	//! num = num * m + add for a big-endian byte number of fixed size.
	protected static void MulAddBytes(notnull array<int> num, int m, int add)
	{
		int carry = add;
		for (int i = num.Count() - 1; i >= 0; i--)
		{
			int cur = num[i] * m + carry;
			num[i] = cur & 255;
			carry = cur >> 8;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Ascii85 (Adobe), framed by <~ ~>.
	static string Ascii85(string text, bool decrypt)
	{
		array<int> bytes = {};
		if (!decrypt)
		{
			Bytes(text, bytes);
			string r = "<~";
			for (int i = 0; i < bytes.Count(); i += 4)
			{
				int remain = Math.Min(4, bytes.Count() - i);
				array<int> chunk = {0, 0, 0, 0};
				for (int j = 0; j < remain; j++)
				{
					chunk[j] = bytes[i + j];
				}

				array<string> digits = {"", "", "", "", ""};
				for (int j = 4; j >= 0; j--)
				{
					digits[j] = Char(33 + DivBytes(chunk, 85));
				}

				for (int j = 0; j < remain + 1; j++)
				{
					r += digits[j];
				}
			}

			return r + "~>";
		}

		string clean = text;
		clean.Replace("<~", "");
		clean.Replace("~>", "");
		array<int> group = {};
		for (int i = 0; i <= clean.Length(); i++)
		{
			bool end = i == clean.Length();
			int c;
			if (!end)
				c = clean.ToAscii(i);

			if (!end && c == 122 && group.IsEmpty())
			{
				bytes.Insert(0);
				bytes.Insert(0);
				bytes.Insert(0);
				bytes.Insert(0);
				continue;
			}

			if (!end && (c < 33 || c > 117))
				continue;

			if (!end)
				group.Insert(c - 33);

			if (group.Count() == 5 || (end && !group.IsEmpty()))
			{
				int have = group.Count();
				while (group.Count() < 5)
				{
					group.Insert(84);
				}

				array<int> num = {0, 0, 0, 0};
				foreach (int v : group)
				{
					MulAddBytes(num, 85, v);
				}

				for (int j = 0; j < have - 1; j++)
				{
					bytes.Insert(num[j]);
				}

				group.Clear();
			}
		}

		return FromBytes(bytes);
	}

	//------------------------------------------------------------------------------------------------
	//! Characters as U+XXXX code points separated by spaces.
	static string Unicode(string text, bool decrypt)
	{
		if (!decrypt)
		{
			array<string> parts = {};
			for (int i = 0; i < text.Length(); i++)
			{
				parts.Insert("U+" + ToBase(text.ToAscii(i) & 255, 16, 4));
			}

			return ROOT_AE3_Util.JoinFrom(parts, 0, " ");
		}

		array<string> words = {};
		Words(text, words);
		string r;
		foreach (string w : words)
		{
			string t = w;
			t.Replace("U+", "");
			t.Replace("u+", "");
			t.Replace("\\u", "");
			if (!t.IsEmpty())
				r += Char(FromBase(t, 16) & 255);
		}

		return r;
	}

	//------------------------------------------------------------------------------------------------
	//! Bytes as numbers in a radix (2, 8, 10, 16) separated by spaces.
	static string Integer(string text, int radix, bool decrypt)
	{
		if (radix != 2 && radix != 8 && radix != 10 && radix != 16)
			radix = 16;

		int width;
		switch (radix)
		{
			case 2: width = 8; break;
			case 8: width = 3; break;
			case 16: width = 2; break;
		}

		array<int> bytes = {};
		if (!decrypt)
		{
			Bytes(text, bytes);
			array<string> parts = {};
			foreach (int b : bytes)
			{
				parts.Insert(ToBase(b, radix, width));
			}

			return ROOT_AE3_Util.JoinFrom(parts, 0, " ");
		}

		string t = text;
		t.Replace(",", " ");
		t.Replace(";", " ");
		array<string> words = {};
		Words(t, words);
		foreach (string w : words)
		{
			bytes.Insert(FromBase(w, radix) & 255);
		}

		return FromBytes(bytes);
	}
}
