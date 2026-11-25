# Terminal

| Program | Usage |
| --- | --- |
| `ls [-l] [-a] [path...]` | list a directory |
| `cd [path]` | change directory, `~` is the home directory |
| `cat file` | print a file |
| `cp src dst`, `mv src dst`, `rm [-r] path`, `mkdir [-p] path`, `touch file` | file operations |
| `find name`, `grep [-i] [-v] pattern file...` | search names and contents (regular expressions) |
| `chown path user` | change owner |
| `unlock [-p] file password` | read or permanently unlock a locked file |
| `mount usbN`, `umount usbN`, `lsusb` | flash drives |
| `sudo cmd`, `su [user]`, `exit` | privileges |
| `ip [r]`, `ifconfig`, `wifi scan`, `wifi connect ssid [password]`, `wifi disconnect`, `ping ip`, `ssh user@ip password`, `msg ip text` | network |
| `date`, `history`, `clear`, `whoami`, `echo text`, `man cmd`, `help` | misc |
| `shutdown`, `standby`, `desktop` | device |
| `crypto -m encrypt\|decrypt [-a caesar\|columnar] -k key [-o file] text\|file` | cipher (installed when enabled) |
| `crack -m bruteforce\|statistics\|key [-a caesar\|columnar] [-o file] text\|file` | attack ciphertext without the key (installed when enabled) |
| `snake [--big]`, `chess` ... `battleship` | games |

`> file` writes and `>> file` appends the output of a command.
