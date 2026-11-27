//! armaOS program 'crypto': Encrypts or decrypts text or a file with any ROOT_AER_Encryption algorithm.
class ROOT_AER_Cmd_Crypto : ROOT_AER_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "crypto";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/crypto";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Encrypts or decrypts text or a file.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		string algos;
		for (int i = 1; i < ROOT_AER_Encryption.COUNT; i++)
		{
			if (i > 1)
				algos += "|";

			algos += ROOT_AER_Encryption.Name(i);
		}

		return "Usage crypto: 'crypto -m encrypt|decrypt [-a " + algos + "] [-k KEY] [-o OUTFILE] TEXT|FILE'. Keys: caesar shift (3), columnar and vigenere key word, rot variant (rot5 rot13 rot18 rot47), affine a,b (5,8), railfence rails (3), bacon extended (26 letters), substitution 26-letter alphabet, integer radix (2 8 10 16). morse, spelling, base32, base64, ascii85 and unicode take no key. Columnar encryption turns spaces into '_'.";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AER_ShellContext ctx, array<string> args)
	{
		map<string, string> opt = new map<string, string>();
		string input;
		if (!ROOT_AER_CryptoArgs.Parse(args, opt, input) || !opt.Contains("m") || input.IsEmpty())
		{
			ctx.Err(GetManual());
			return;
		}

		string mode = opt.Get("m");
		if (mode != "encrypt" && mode != "decrypt")
		{
			ctx.Err("crypto: mode is encrypt or decrypt");
			return;
		}

		string algo = "caesar";
		if (opt.Contains("a"))
			algo = opt.Get("a");

		string text;
		if (!ROOT_AER_CryptoArgs.ReadInput(ctx, input, text))
			return;

		int id = ROOT_AER_Encryption.FromName(algo);
		if (id <= 0)
		{
			ctx.Err("crypto: unknown algorithm " + algo);
			return;
		}

		string key = ROOT_AER_Encryption.DefaultKey(id);
		if (opt.Contains("k"))
			key = opt.Get("k");
		else if (!ROOT_AER_Encryption.KeyHint(id).IsEmpty() && id != ROOT_AER_Encryption.ROT && id != ROOT_AER_Encryption.BACON && id != ROOT_AER_Encryption.INTEGER)
		{
			ctx.Err("crypto: " + algo + " needs a key (-k): " + ROOT_AER_Encryption.KeyHint(id));
			return;
		}

		string bad = ROOT_AER_Encryption.CheckKey(id, key);
		if (!bad.IsEmpty())
		{
			ctx.Err("crypto: " + bad);
			return;
		}

		if (id == ROOT_AER_Encryption.COLUMNAR && mode == "encrypt")
			text.Replace(" ", "_");

		string result = ROOT_AER_Encryption.Apply(id, text, key, mode == "decrypt");
		if (!opt.Contains("o"))
		{
			ctx.Out(result);
			return;
		}

		string err;
		if (!ctx.Fs().WriteFile(ctx.Abs(opt.Get("o")), result, ctx.FsUser(), false, err))
			ctx.Err(string.Format("crypto: %1: %2", opt.Get("o"), err));
		else
			ctx.Out("Written to " + opt.Get("o"));
	}
}

//! Argument parsing shared by crypto and crack: '-x value' pairs plus one positional input.
class ROOT_AER_CryptoArgs
{
	//------------------------------------------------------------------------------------------------
	static bool Parse(notnull array<string> args, notnull map<string, string> opt, out string input)
	{
		for (int i = 0; i < args.Count(); i++)
		{
			string a = args[i];
			if (a.Length() == 2 && a.StartsWith("-") && i + 1 < args.Count())
			{
				opt.Set(a.Substring(1, 1), args[i + 1]);
				i++;
				continue;
			}

			if (!input.IsEmpty())
				input += " ";

			input += a;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Input as file content when it names a readable file, else the text itself.
	static bool ReadInput(ROOT_AER_ShellContext ctx, string input, out string text)
	{
		string err;
		if (ctx.Fs().Resolve(ctx.Abs(input), ctx.FsUser(), err))
		{
			if (!ctx.Fs().ReadFile(ctx.Abs(input), ctx.FsUser(), text, err))
			{
				ctx.Err(string.Format("%1: %2", input, err));
				return false;
			}

			if (ROOT_AER_Intel.IsLocked(text) || ROOT_AER_Intel.IsMedia(text))
			{
				ctx.Err(input + ": file cannot be processed");
				return false;
			}

			text = text.Trim();
			return true;
		}

		text = input;
		return true;
	}
}
