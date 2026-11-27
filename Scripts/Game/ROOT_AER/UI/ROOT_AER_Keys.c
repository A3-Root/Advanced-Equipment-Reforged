//! Keyboard keys read directly (Debug.KeyState), also while an edit box is in write mode.
//! Pressed() is true once per key press; call it every frame for the keys in use.
class ROOT_AER_Keys
{
	protected static ref map<KeyCode, bool> s_mDown = new map<KeyCode, bool>();

	//------------------------------------------------------------------------------------------------
	static bool Pressed(KeyCode key)
	{
		bool down = Debug.KeyState(key) != 0;
		bool was = s_mDown.Get(key);
		s_mDown.Set(key, down);
		return down && !was;
	}
}
