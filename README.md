# totty

`totty` unclogs pipes and gets them flowing again.

When using the standard I/O functions provided by libc the program
will automatically detect when the output is not written a
terminal (also called a "tty") and will buffer that output.
While this is good for performance it can break the interactivity
of a piped command that you're running at the terminal.

Add the word `totty` (as in, "to a terminal") before any command
in your pipe and that command will believe it is writing to a
terminal.

### An example

A common command is:

```
tail -f /var/log/apache2/access.log | grep 404
```

but if a second grep is added, the output stops until the buffer fills:

```
tail -f /var/log/apache2/access.log | grep 404 | grep favicon.ico
(output stops here until 4kb of "404" data is written)
```

To fix this, use `totty`!

```
tail -f /var/log/apache2/access.log | totty grep 404 | grep favicon.ico
(the data flows again)
```

No need to figure out which component is the source of the
clog, if you're unsure then it's safe to add it everywhere:

```
totty tail -f /var/log/apache2/access.log | totty grep 404 | totty grep favicon.ico
(works fine)
```

### Alternatives

You may see suggestions to use the `script` command with particular
flags for this purpose, but it has several downsides:

- it can be hard to remember: `script --return -c "command to execute" /dev/null`
- it requires you to escape the command to invoke, and to add text before and after the existing command
- it is not portable (none of these flags are supported on macOS)

By comparison, `totty` works anywhere that supports POSIX
pseudo-terminals, is easy to remember, is easy to introduce to a
working pipe, and requires no escaping or modification of the pipe
component that you're unclogging.
