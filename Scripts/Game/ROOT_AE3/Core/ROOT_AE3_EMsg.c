//! Message ids between client and server (SCR_PlayerController RPCs).
enum ROOT_AE3_EMsg
{
	CHUNK,			//!< Part of a long payload; the next non-chunk message completes it.
	// client -> server
	OPEN,			//!< data: requested mode (0 terminal, 1 desktop)
	CLOSE,
	TERM_LINE,		//!< data: entered line
	TERM_TAB,		//!< data: current line, reply TERM_SET_LINE
	TERM_KEY,		//!< data: key name (CTRL_C)
	GUI,			//!< data: rid, cmd, args...
	GAME_INPUT,		//!< data: table id, move
	// server -> client
	SESSION,		//!< data: device name, mode, settings, design, theme, user, history...
	TERM_OUT,		//!< data: text to append
	TERM_CLEAR,
	TERM_PROMPT,	//!< data: prompt, obfuscate (0/1)
	TERM_SET_LINE,	//!< data: line
	GUI_REPLY,		//!< data: rid, ok, payload...
	ENDED,			//!< data: reason
	MODE,			//!< data: 0 terminal, 1 desktop
	NOTIFY,			//!< data: title, text
	GAME_STATE,		//!< data: table view
	SNAKE,			//!< data: big (0/1)
	SETTINGS,		//!< data: ROOT_AE3_Settings.PackClient
	SCREEN,			//!< data: device RplId, screen text, input line
	SCREEN_INPUT,	//!< data: device RplId, input line
	SOUND,			//!< data: sound id (ROOT_AE3_Sound), UI sound for the operator
	// client -> server
	OPEN_CARRIED,	//!< data: requested mode (-1 auto, 0 terminal, 1 desktop); device in the player's inventory
	RENAME,			//!< data: new device name
	EWO,			//!< data: EWO request (ROOT_AE3_EwoSystem.OnRequest)
	PLUG_NET,		//!< data: router RplId, cable length; device = the device to plug
	// server -> client
	EWO_STATE		//!< data: EWO pack state (ROOT_AE3_EwoPanel)
}
