//! Calculator with + - * / and decimal point.
class ROOT_AE3_AppCalculator : ROOT_AE3_App
{
	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Calculator";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "plus";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(300, 380, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected TextWidget m_wDisplay;
	protected float m_fAcc;
	protected string m_sOp;
	protected string m_sEntry = "0";
	protected bool m_bFresh = true;

	override void Build()
	{
		float w = Width();
		ROOT_AE3_UI.Rect(GetContent(), 8, 8, w - 16, 44, ROOT_AE3_Theme.Field());
		m_wDisplay = Label(16, 16, w - 32, 30, m_sEntry, 24, true);
		array<string> keys = {"7", "8", "9", "/", "4", "5", "6", "*", "1", "2", "3", "-", "0", ".", "=", "+", "C"};
		float bw = (w - 16 - 18) / 4;
		foreach (int i, string k : keys)
		{
			int col = i % 4;
			int row = i / 4;
			Btn(8 + col * (bw + 6), 60 + row * 52, bw, 46, k, 100 + i);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Apply()
	{
		float v = m_sEntry.ToFloat();
		switch (m_sOp)
		{
			case "+": m_fAcc = m_fAcc + v; break;
			case "-": m_fAcc = m_fAcc - v; break;
			case "*": m_fAcc = m_fAcc * v; break;
			case "/":
			{
				if (v == 0)
				{
					m_sEntry = "Error";
					m_sOp = "";
					return;
				}

				m_fAcc = m_fAcc / v;
				break;
			}

			default: m_fAcc = v;
		}

		m_sEntry = m_fAcc.ToString();
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		array<string> keys = {"7", "8", "9", "/", "4", "5", "6", "*", "1", "2", "3", "-", "0", ".", "=", "+", "C"};
		if (id < 100 || id >= 100 + keys.Count())
			return false;

		string k = keys[id - 100];
		if (k == "C")
		{
			m_fAcc = 0;
			m_sOp = "";
			m_sEntry = "0";
			m_bFresh = true;
		}
		else if (k == "=")
		{
			Apply();
			m_sOp = "";
			m_bFresh = true;
		}
		else if (k == "+" || k == "-" || k == "*" || k == "/")
		{
			Apply();
			m_sOp = k;
			m_bFresh = true;
		}
		else
		{
			if (m_bFresh)
			{
				m_sEntry = "";
				m_bFresh = false;
			}

			if (k != "." || !m_sEntry.Contains("."))
				m_sEntry += k;
		}

		m_wDisplay.SetText(m_sEntry);
		return true;
	}
}
