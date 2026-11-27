//! armaOS program 'crack': Attacks ciphertext without the key (caesar, columnar, rot, affine, railfence, encodings).
class ROOT_AER_Cmd_Crack : ROOT_AER_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "crack";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/crack";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Attacks crypto output without the key.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage crack: 'crack -m bruteforce|statistics|key [-a caesar|columnar|rot|affine|railfence|morse|spelling|bacon|base32|base64|ascii85|unicode|integer] [-o OUTFILE] TEXT|FILE'. caesar: bruteforce prints all 26 shifts, statistics guesses the shift from letter frequency. columnar: key lists candidate key lengths, bruteforce prints the grid for each length. rot: all four variants. affine: all 312 keys. railfence: 2 to 12 rails. Encodings (morse to integer) decode directly.";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AER_ShellContext ctx, array<string> args)
	{
		map<string, string> opt = new map<string, string>();
		string input;
		ROOT_AER_CryptoArgs.Parse(args, opt, input);
		if (!opt.Contains("m") || input.IsEmpty())
		{
			ctx.Err(GetManual());
			return;
		}

		string text;
		if (!ROOT_AER_CryptoArgs.ReadInput(ctx, input, text))
			return;

		if (text.Length() > 4096)
			text = text.Substring(0, 4096);

		string mode = opt.Get("m");
		string algo = "caesar";
		if (opt.Contains("a"))
			algo = opt.Get("a");

		array<string> lines = {};
		if (algo == "caesar" && mode == "bruteforce")
		{
			for (int k = 1; k < 26; k++)
			{
				lines.Insert(string.Format("%1: %2", ROOT_AER_Util.PadLeft(k.ToString(), 2), ROOT_AER_Encryption.Caesar(text, -k)));
			}
		}
		else if (algo == "caesar" && mode == "statistics")
		{
			int shift = GuessShift(text);
			lines.Insert(string.Format("Most frequent letter taken as E: shift %1", shift));
			lines.Insert(ROOT_AER_Encryption.Caesar(text, -shift));
		}
		else if (algo == "rot" && mode == "bruteforce")
		{
			array<string> variants = {"rot5", "rot13", "rot18", "rot47"};
			foreach (string variant : variants)
			{
				lines.Insert(variant + ": " + ROOT_AER_Encryption.Rot(text, variant, true));
			}
		}
		else if (algo == "affine" && mode == "bruteforce")
		{
			for (int a = 1; a < 26; a += 2)
			{
				if (a == 13)
					continue;

				for (int b = 0; b < 26; b++)
				{
					lines.Insert(string.Format("%1,%2: %3", a, b, ROOT_AER_Encryption.Affine(text, a, b, true)));
				}
			}
		}
		else if (algo == "railfence" && mode == "bruteforce")
		{
			for (int rails = 2; rails <= Math.Min(12, text.Length() - 1); rails++)
			{
				lines.Insert(string.Format("%1 rails: %2", rails, ROOT_AER_Encryption.RailFence(text, rails, true)));
			}
		}
		else if (mode == "bruteforce" && ROOT_AER_Encryption.FromName(algo) >= ROOT_AER_Encryption.MORSE && ROOT_AER_Encryption.FromName(algo) != ROOT_AER_Encryption.SUBSTITUTION)
		{
			lines.Insert(ROOT_AER_Encryption.Apply(ROOT_AER_Encryption.FromName(algo), text, "", true));
		}
		else if (algo == "columnar" && mode == "key")
		{
			int len = text.Length();
			string lens;
			for (int n = 2; n <= Math.Min(len, 20); n++)
			{
				if (len % n == 0)
					lens += n.ToString() + " ";
			}

			lines.Insert(string.Format("Length %1. Candidate key lengths (divisors): %2", len, lens.Trim()));
		}
		else if (algo == "columnar" && mode == "bruteforce")
		{
			int len = text.Length();
			for (int cols = 2; cols <= Math.Min(len, 12); cols++)
			{
				int rows = Math.Ceil(len / (float)cols);
				lines.Insert(string.Format("Key length %1:", cols));
				for (int r = 0; r < rows; r++)
				{
					string row;
					for (int c = 0; c < cols; c++)
					{
						int idx = c * rows + r;
						if (idx < len)
							row += text.Get(idx) + " ";
					}

					lines.Insert("  " + row);
				}
			}
		}
		else
		{
			ctx.Err(string.Format("crack: mode '%1' is not available for %2", mode, algo));
			return;
		}

		if (!opt.Contains("o"))
		{
			foreach (string l : lines)
			{
				ctx.Out(l);
			}

			return;
		}

		string data;
		foreach (string l : lines)
		{
			data += l + "\n";
		}

		string err;
		if (!ctx.Fs().WriteFile(ctx.Abs(opt.Get("o")), data, ctx.FsUser(), false, err))
			ctx.Err(string.Format("crack: %1: %2", opt.Get("o"), err));
		else
			ctx.Out("Written to " + opt.Get("o"));
	}

	//------------------------------------------------------------------------------------------------
	protected int GuessShift(string text)
	{
		array<int> counts = {};
		counts.Resize(26);
		string up = text;
		up.ToUpper();
		for (int i = 0; i < up.Length(); i++)
		{
			int c = up.ToAscii(i);
			if (c >= 65 && c <= 90)
				counts[c - 65] = counts[c - 65] + 1;
		}

		int best = 0;
		for (int j = 1; j < 26; j++)
		{
			if (counts[j] > counts[best])
				best = j;
		}

		int shift = best - 4;
		if (shift < 0)
			shift += 26;

		return shift;
	}
}
