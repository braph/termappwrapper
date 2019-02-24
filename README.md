# termappwrapper

TermAppWrapper is a C program which allows you to change the keybindings of a terminal application.
It provides different keymodes, just like vi.

## Configuration commands
* mode
`mode <mode>` begins a new mode

* bind
`bind <key> <command>` binds a command

* repeat
`repeat <on|off>` enables/disables vi-like command repetition

* ignore\_unmapped
`ignore\_unmapped [<all|char|...>]` discard this input

## Commands
* mask
Mask the next charachter

* key
`key <KEY>...` types KEY

* write
`write <STR>...` writes strings

* goto
`goto <mode>` switches mode

* readline
`readline [OPTIONS]` use readline to read a line

**NOTE:** This command only works right in programs that can react to the SIGWINCH signal

* ignore
do nothing

* signal
`signal <SIGNAL>` send signal to the program

## BUGS / LIMITATIONS
`goto` does not check if the mode exists
