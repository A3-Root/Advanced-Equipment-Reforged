//! armaOS program 'crypto': Encrypts or decrypts text or a file with caesar or columnar.
class ROOT_AE3_Cmd_Crypto : ROOT_AE3_Command
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
		return "Usage crypto: 'crypto -m encrypt|decrypt [-a caesar|columnar] -k KEY [-o OUTFILE] TEXT|FILE'. Caesar keys are integer shifts, columnar keys are words (column count = key length, spaces become '_').";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AE3_ShellContext ctx, array<string> args)
	{
		map<string, string> opt = new map<string, string>();
		string input;
		if (!ROOT_AE3_CryptoArgs.Parse(args, opt, input) || !opt.Contains("m") || !opt.Contains("k") || input.IsEmpty())
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
		if (!ROOT_AE3_CryptoArgs.ReadInput(ctx, input, text))
			return;

		string key = opt.Get("k");
		string result;
		if (algo == "caesar")
		{
			if (!ROOT_AE3_Util.IsInt(key) || key.ToInt() <= 0)
			{
				ctx.Err("crypto: caesar keys are positive integers");
				return;
			}

			int shift = key.ToInt();
			if (mode == "decrypt")
				shift = -shift;

			result = ROOT_AE3_Encryption.Caesar(text, shift);
		}
		else if (algo == "columnar")
		{
			if (key.Length() < 2)
			{
				ctx.Err("crypto: columnar keys need at least 2 characters");
				return;
			}

			if (mode == "encrypt")
			{
				text.Replace(" ", "_");
				result = ROOT_AE3_Encryption.Columnar(text, key);
			}
			else
			{
				result = ROOT_AE3_Encryption.Columnar(text, key, true);
			}
		}
		else
		{
			ctx.Err("crypto: unknown algorithm " + algo);
			return;
		}

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
class ROOT_AE3_CryptoArgs
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
	static bool ReadInput(ROOT_AE3_ShellContext ctx, string input, out string text)
	{
		string err;
		if (ctx.Fs().Resolve(ctx.Abs(input), ctx.FsUser(), err))
		{
			if (!ctx.Fs().ReadFile(ctx.Abs(input), ctx.FsUser(), text, err))
			{
				ctx.Err(string.Format("%1: %2", input, err));
				return false;
			}

			if (ROOT_AE3_Intel.IsLocked(text) || ROOT_AE3_Intel.IsMedia(text))
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
