//! Calculator with + - * / %, sign change, backspace and decimal point. Buttons or keyboard / numpad
//! (digits, + - * / . , Enter or = for the result, Backspace, Delete or C to clear).
class ROOT_AE3_AppCalculator : ROOT_AE3_App
{
	protected static ref array<string> KEYS = {"C", "<-", "%", "/", "7", "8", "9", "*", "4", "5", "6", "-", "1", "2", "3", "+", "+/-", "0", ".", "="};

	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Calculator";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "calculator";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(320, 460, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected TextWidget m_wDisplay;
	protected TextWidget m_wExpr;
	protected float m_fAcc;
	protected string m_sOp;
	protected string m_sEntry = "0";
	protected bool m_bFresh = true;

	override void Build()
	{
		float w = Width();
		float h = Height();
		m_wExpr = ROOT_AE3_UI.Text(GetContent(), 14, 10, w - 28, 20, "", 14, ROOT_AE3_Theme.Muted(), false, false, 2);
		m_wDisplay = ROOT_AE3_UI.Text(GetContent(), 14, 30, w - 28, 44, m_sEntry, 34, ROOT_AE3_Theme.Text(), false, false, 2);
		float top = 86;
		float gap = 4;
		float bw = (w - 16 - gap * 3) / 4;
		float bh = (h - top - 8 - gap * 4) / 5;
		foreach (int i, string k : KEYS)
		{
			int col = i % 4;
			int row = i / 4;
			float x = 8 + col * (bw + gap);
			float y = top + row * (bh + gap);
			if (k == "=")
				MainBtn(x, y, bw, bh, k, 100 + i);
			else
				Btn(x, y, bw, bh, k, 100 + i);
		}

		Show();
	}

	//------------------------------------------------------------------------------------------------
	protected void Show()
	{
		if (m_wDisplay)
			m_wDisplay.SetText(m_sEntry);

		if (!m_wExpr)
			return;

		if (m_sOp.IsEmpty())
			m_wExpr.SetText("");
		else
			m_wExpr.SetText(Format(m_fAcc) + " " + m_sOp);
	}

	//------------------------------------------------------------------------------------------------
	protected static string Format(float v)
	{
		if (Math.AbsFloat(v - Math.Round(v)) < 0.000001 && Math.AbsFloat(v) < 1000000000)
		{
			int iv = Math.Round(v);
			return iv.ToString();
		}

		return v.ToString();
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
					m_fAcc = 0;
					return;
				}

				m_fAcc = m_fAcc / v;
				break;
			}

			default: m_fAcc = v;
		}

		m_sEntry = Format(m_fAcc);
	}

	//------------------------------------------------------------------------------------------------
	//! Handles one key of KEYS.
	protected void Press(string k)
	{
		if (m_sEntry == "Error" && k != "C")
		{
			m_sEntry = "0";
			m_bFresh = true;
		}

		switch (k)
		{
			case "C":
			{
				m_fAcc = 0;
				m_sOp = "";
				m_sEntry = "0";
				m_bFresh = true;
				break;
			}

			case "<-":
			{
				if (m_bFresh || m_sEntry.Length() <= 1 || (m_sEntry.Length() == 2 && m_sEntry.StartsWith("-")))
				{
					m_sEntry = "0";
					m_bFresh = true;
				}
				else
				{
					m_sEntry = m_sEntry.Substring(0, m_sEntry.Length() - 1);
				}

				break;
			}

			case "=":
			{
				Apply();
				m_sOp = "";
				m_bFresh = true;
				break;
			}

			case "%":
			{
				float v = m_sEntry.ToFloat();
				if (m_sOp == "+" || m_sOp == "-")
					v = m_fAcc * v / 100;
				else
					v = v / 100;

				m_sEntry = Format(v);
				m_bFresh = true;
				break;
			}

			case "+/-":
			{
				if (m_sEntry.StartsWith("-"))
					m_sEntry = m_sEntry.Substring(1, m_sEntry.Length() - 1);
				else if (m_sEntry != "0")
					m_sEntry = "-" + m_sEntry;

				break;
			}

			case "+":
			case "-":
			case "*":
			case "/":
			{
				if (!m_bFresh || m_sOp.IsEmpty())
					Apply();

				m_sOp = k;
				m_bFresh = true;
				break;
			}

			default:
			{
				if (m_bFresh)
				{
					m_sEntry = "0";
					m_bFresh = false;
				}

				if (k == ".")
				{
					if (!m_sEntry.Contains("."))
						m_sEntry += ".";
				}
				else if (m_sEntry == "0")
				{
					m_sEntry = k;
				}
				else if (m_sEntry == "-0")
				{
					m_sEntry = "-" + k;
				}
				else if (m_sEntry.Length() < 16)
				{
					m_sEntry += k;
				}
			}
		}

		Show();
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (id < 100 || id >= 100 + KEYS.Count())
			return false;

		Press(KEYS[id - 100]);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnKey(KeyCode key, bool ctrl)
	{
		if (ctrl)
			return false;

		bool shift = Debug.KeyState(KeyCode.KC_LSHIFT) != 0 || Debug.KeyState(KeyCode.KC_RSHIFT) != 0;
		string k;
		switch (key)
		{
			case KeyCode.KC_NUMPAD0: k = "0"; break;
			case KeyCode.KC_NUMPAD1: k = "1"; break;
			case KeyCode.KC_NUMPAD2: k = "2"; break;
			case KeyCode.KC_NUMPAD3: k = "3"; break;
			case KeyCode.KC_NUMPAD4: k = "4"; break;
			case KeyCode.KC_NUMPAD5: k = "5"; break;
			case KeyCode.KC_NUMPAD6: k = "6"; break;
			case KeyCode.KC_NUMPAD7: k = "7"; break;
			case KeyCode.KC_NUMPAD8: k = "8"; break;
			case KeyCode.KC_NUMPAD9: k = "9"; break;
			case KeyCode.KC_0: k = "0"; break;
			case KeyCode.KC_1: k = "1"; break;
			case KeyCode.KC_2: k = "2"; break;
			case KeyCode.KC_3: k = "3"; break;
			case KeyCode.KC_4: k = "4"; break;
			case KeyCode.KC_5:
			{
				k = "5";
				if (shift)
					k = "%";

				break;
			}

			case KeyCode.KC_6: k = "6"; break;
			case KeyCode.KC_7: k = "7"; break;
			case KeyCode.KC_8:
			{
				k = "8";
				if (shift)
					k = "*";

				break;
			}

			case KeyCode.KC_9: k = "9"; break;
			case KeyCode.KC_ADD: k = "+"; break;
			case KeyCode.KC_SUBTRACT: k = "-"; break;
			case KeyCode.KC_MINUS: k = "-"; break;
			case KeyCode.KC_MULTIPLY: k = "*"; break;
			case KeyCode.KC_DIVIDE: k = "/"; break;
			case KeyCode.KC_SLASH: k = "/"; break;
			case KeyCode.KC_DECIMAL: k = "."; break;
			case KeyCode.KC_PERIOD: k = "."; break;
			case KeyCode.KC_COMMA: k = "."; break;
			case KeyCode.KC_EQUALS:
			{
				k = "=";
				if (shift)
					k = "+";

				break;
			}

			case KeyCode.KC_RETURN: k = "="; break;
			case KeyCode.KC_NUMPADENTER: k = "="; break;
			case KeyCode.KC_BACK: k = "<-"; break;
			case KeyCode.KC_DELETE: k = "C"; break;
			case KeyCode.KC_C: k = "C"; break;
		}

		if (k.IsEmpty())
			return false;

		Press(k);
		return true;
	}
}
