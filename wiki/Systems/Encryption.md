# Encryption

Files can hold text encrypted with one of 15 ciphers (`ROOT_AER_Encryption`). Players decrypt it with `crypto` when they
know the key, or attack it with `crack`. Both programs are installed on computers with
`ROOT_AER_ComputerComponent.m_bInstallSecurity` or `ROOT_AER_API.InstallCommand(device, "crypto")` / `"crack"`.

| Id | Name (`-a`) | Key (`-k`) | Default key |
| --- | --- | --- | --- |
| 1 | `caesar` | Shift, a whole number | 3 |
| 2 | `columnar` | Key word, 2 letters or more (encryption turns spaces into `_`) | KEY |
| 3 | `vigenere` | Key word | LEMON |
| 4 | `rot` | `rot5` (digits), `rot13` (letters), `rot18` (both), `rot47` (printable characters) | rot13 |
| 5 | `affine` | `a,b`, a coprime with 26 (1 3 5 7 9 11 15 17 19 21 23 25), b 0-25 | 5,8 |
| 6 | `railfence` | Number of rails, 2 or more | 3 |
| 7 | `morse` | none; words separated by `/` | |
| 8 | `spelling` | none; NATO spelling alphabet, words separated by `/` | |
| 9 | `bacon` | empty = 24 letters (I = J, U = V), `extended` = 26 letters | |
| 10 | `substitution` | 26-letter alphabet | QWERTYUIOPASDFGHJKLZXCVBNM |
| 11 | `base32` | none | |
| 12 | `base64` | none | |
| 13 | `ascii85` | none; framed by `<~ ~>` | |
| 14 | `unicode` | none; `U+0041` per character | |
| 15 | `integer` | Number base 2, 8, 10 or 16 | 16 |

Text is treated as bytes; non-letters pass through the letter ciphers unchanged.

## crypto

`crypto -m encrypt|decrypt [-a cipher] [-k key] [-o file] text|file`. Without `-a` the cipher is `caesar`. `-k` is
needed for caesar, columnar, vigenere, affine, railfence and substitution. A file path as input reads the file;
`-o` writes the result to a file.

## crack

`crack -m mode [-a cipher] [-o file] text|file`:

| Cipher | Mode | Output |
| --- | --- | --- |
| caesar | bruteforce, statistics | all 25 shifts; or the shift guessed from letter frequency |
| columnar | key, bruteforce | candidate key lengths; the grid for each length 2-12 |
| rot | bruteforce | the four variants |
| affine | bruteforce | all 312 keys |
| railfence | bruteforce | 2 to 12 rails |
| morse, spelling, bacon, base32, base64, ascii85, unicode, integer | bruteforce | the decoded text |

## Setting up an encrypted file

- Game Master: "Add file", Protection "Encrypted text (cipher)" or "Password and cipher", then the cipher and its key.
- World Editor: `ROOT_AER_FileEntry` Encryption and Key (empty key = default key).
- Script: `ROOT_AER_API.AddFile(device, path, content, owner, perms, ROOT_AER_Encryption.VIGENERE, "LEMON")`.
