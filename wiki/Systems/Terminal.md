# Terminal

armaOS terminal: login (`LOGIN>` / `PASSWORD>`), prompt `user@host:/path/>` with the typed text and cursor after the
last line, quotes, `>` / `>>` redirection, Tab completion, history (Up / Down), Ctrl+C button, 20 designs, font size
(A- / A+), mouse wheel scrolling, Desktop button (computers with both interfaces).

## Programs

| Program | Usage |
| --- | --- |
| `ls [-l] [-a] [path...]` | list a directory |
| `cd [path]` | change directory, `~` is the home directory |
| `cat file` | print a file |
| `cp src dst`, `mv src dst`, `rm [-r] path`, `mkdir [-p] path`, `touch file` | file operations |
| `find name`, `grep [-i] [-v] pattern file...` | search names and contents (regular expressions) |
| `echo text` | print text (`> file` writes, `>> file` appends) |
| `chown path user` | change owner |
| `unlock [-p] file password` | read or permanently unlock a locked file |
| `mount usbN`, `umount usbN`, `lsusb` | flash drives (plugging a drive mounts it) |
| `sudo cmd`, `su [user]`, `exit` | privileges; `exit` also logs out |
| `ip [r]`, `ifconfig` | address and gateway |
| `wifi scan`, `wifi connect ssid [password]`, `wifi disconnect` | wireless networks |
| `ping address`, `ssh user@host password`, `sshd [on\|off\|status]`, `msg ip text` | network (host: IP address or hostname) |
| `date`, `history`, `clear`, `whoami`, `man cmd`, `help` | misc |
| `shutdown`, `standby`, `desktop` | device |
| `crypto -m encrypt\|decrypt [-a cipher] [-k key] [-o file] text\|file` | cipher, see [Encryption](Encryption.md) (installed when enabled) |
| `crack -m bruteforce\|statistics\|key [-a cipher] [-o file] text\|file` | attack ciphertext without the key (installed when enabled) |
| `snake [--big]`, `chess`, `checkers`, `uno`, `ludo`, `snakesladders`, `blackjack`, `rps`, `battleship` | games, see [Games](Games.md) |

## Programs are files

Programs are files (`#!ae3 <name>`) in `/bin`, `/sbin`, `/usr/bin`, `/games`. A program runs only when its root-owned
file exists and it was installed on that computer; deleting the file removes it. Program files on flash drives or
outside these folders do not run. Other executable files run as scripts (one command per line, at most 50 lines).

## Accounts

Root login is off by default (`m_bAllowRootLogin`); users in `/etc/sudoers` use `sudo` / `su`. While
`m_bCliElevateSudoers` is on (default), sudoers act as root on the filesystem; the account `admin` always does and
counts as a sudoer. Default root password `toor` (`m_sDefaultRootPassword`). Failed logins and unlocks go to
`/var/log/auth.log`; each failure delays the player's next attempt by 1 s, up to 5 s.

## Screen on the model

Within `m_fUiPlayerRange` (2 m) nearby players see the terminal text on the laptop screen, up to
`m_iUiMaxConcurrentViewers` (3) per device. Password arguments of `ssh`, `unlock`, `wifi connect` and `crypto -k` are
masked.
