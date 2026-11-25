//! Classic ciphers for encrypted intel files.
class ROOT_AE3_Encryption
{
	//------------------------------------------------------------------------------------------------
	//! Caesar shift of letters; decrypt with a negative key.
	static string Caesar(string text, int key)
	{
		string r;
		key = key % 26;
		if (key < 0)
			key += 26;

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
}
