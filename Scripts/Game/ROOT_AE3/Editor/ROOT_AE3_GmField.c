enum ROOT_AE3_EGmField
{
	TEXT,
	MULTILINE,
	PASSWORD,
	TOGGLE,		//!< Value "1" or "0"
	CHOICE,		//!< Value one of m_aValues
	PERMS		//!< Value like "rw-r--": owner rwx, everyone rwx
}

//! One field of a Game Master form. Built on the server with the current values of the first selected device
//! and sent packed to the Game Master (ROOT_AE3_GmActions.Fields).
class ROOT_AE3_GmField
{
	string m_sLabel;
	ROOT_AE3_EGmField m_eKind;
	string m_sValue;
	ref array<string> m_aValues = {};	//!< CHOICE: values
	ref array<string> m_aLabels = {};	//!< CHOICE: labels shown for the values
	int m_iShowIf = -1;					//!< Field index that decides visibility, -1 = always shown
	ref array<string> m_aShowValues = {};	//!< Values of field m_iShowIf that show this field

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_GmField Make(ROOT_AE3_EGmField kind, string label, string value)
	{
		ROOT_AE3_GmField f = new ROOT_AE3_GmField();
		f.m_eKind = kind;
		f.m_sLabel = label;
		f.m_sValue = value;
		return f;
	}

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_GmField Toggle(string label, bool value)
	{
		return Make(ROOT_AE3_EGmField.TOGGLE, label, ROOT_AE3_Util.BoolStr(value));
	}

	//------------------------------------------------------------------------------------------------
	//! values and labels have the same length; value not in values selects the first one.
	static ROOT_AE3_GmField Choice(string label, notnull array<string> values, notnull array<string> labels, string value)
	{
		ROOT_AE3_GmField f = Make(ROOT_AE3_EGmField.CHOICE, label, value);
		f.m_aValues.Copy(values);
		f.m_aLabels.Copy(labels);
		if (!values.IsEmpty() && !values.Contains(value))
			f.m_sValue = values[0];

		return f;
	}

	//------------------------------------------------------------------------------------------------
	//! Shown only while field `field` has one of the space separated values.
	ROOT_AE3_GmField ShowIf(int field, string values)
	{
		m_iShowIf = field;
		m_aShowValues = ROOT_AE3_Util.Words(values);
		return this;
	}

	//------------------------------------------------------------------------------------------------
	static string Pack(notnull array<ref ROOT_AE3_GmField> fields)
	{
		array<string> packed = {};
		foreach (ROOT_AE3_GmField f : fields)
		{
			array<string> p = {f.m_sLabel, f.m_eKind.ToString(), f.m_sValue, f.m_iShowIf.ToString(), ROOT_AE3_Util.JoinFrom(f.m_aShowValues, 0, " ")};
			foreach (int i, string v : f.m_aValues)
			{
				p.Insert(v);
				if (i < f.m_aLabels.Count())
					p.Insert(f.m_aLabels[i]);
				else
					p.Insert(v);
			}

			packed.Insert(ROOT_AE3_Codec.JoinSub(p));
		}

		return ROOT_AE3_Codec.Join(packed);
	}

	//------------------------------------------------------------------------------------------------
	static void Unpack(string packed, notnull array<ref ROOT_AE3_GmField> fields)
	{
		fields.Clear();
		if (packed.IsEmpty())
			return;

		foreach (string s : ROOT_AE3_Codec.Split(packed))
		{
			array<string> p = ROOT_AE3_Codec.SplitSub(s);
			if (p.Count() < 5)
				continue;

			ROOT_AE3_GmField f = Make(p[1].ToInt(), p[0], p[2]);
			f.m_iShowIf = p[3].ToInt();
			f.m_aShowValues = ROOT_AE3_Util.Words(p[4]);
			for (int i = 5; i + 1 < p.Count(); i += 2)
			{
				f.m_aValues.Insert(p[i]);
				f.m_aLabels.Insert(p[i + 1]);
			}

			fields.Insert(f);
		}
	}
}
