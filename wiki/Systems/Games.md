# Games

Network games run on the server as tables that computers join: chess, checkers, Uno, Ludo, Snakes and Ladders,
Blackjack, Rock Paper Scissors, Battleship. Snake runs locally.

- Terminal: `chess` ... `battleship` open the lobby (new, join <number>, resume <number>, refresh, quit); at a table:
  moves, board, status, seatname <text>, leave, quit. `snake [--big]`.
- Desktop Games app: game list, lobby (new table, join, play against a bot, own matches), waiting room (seats, Start,
  Add computer, Remove computer, Practice seat), table with the drawn board and a typed move field. Boards scale with
  the window.
- Boards: chess and checkers (click a piece, then a marked square; promotion choice), Uno (click a raised card or the
  draw pile; wilds ask for the colour), Ludo and Snakes and Ladders (Roll, click a glowing token), Blackjack (Deal,
  Hit, Stand), Rock Paper Scissors (click a throw), Battleship (place ships, Random, Clear, Ready; click enemy waters).
- Practice seats (`m_bGamesDebugSeats`): the host holds several seats and plays the one to move. Bots
  (`m_bGamesBots`): hosts add bot players; script `HostGameWithBots(device, game, bots)`.

Settings: `m_bGamesEnabled`, `m_bGamesAllowHosting`, `m_bGamesRequireSameNetwork`, `m_iGamesMaxTables` (16),
`m_fGamesTurnTimeout` (120 s), `m_fGamesAbandonTimeout` (600 s), extra power draw `m_fGamesPowerDraw` (15 W) and
`m_fGamesIdleDraw` (2 W). Games are installed per computer (`m_bInstallGames`, Game Master "Install games",
`AddGames`).
