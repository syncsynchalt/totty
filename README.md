# totty

`totty` unclogs pipes and gets them flowing again.

The standard I/O functions in libc will detect when a program is
not writing to a terminal (also called a tty) and will automatically
buffer output.  While this is good for performance it can break the
interactivity of a piped command that you're watching at the terminal.

Add the word `totty` (as in, "to a terminal") before any command
in your pipe and that command will believe it is writing to a
terminal, disabling this automatic buffering feature.

### An example

A common command is:

```
tail -f /var/log/apache2/access.log | grep 404
```

but if a second grep is added, the output stops until the buffer fills:

```
tail -f /var/log/apache2/access.log | grep 404 | grep favicon.ico
```

To fix this, use `totty` and the pipe flows again!

```
tail -f /var/log/apache2/access.log | totty grep 404 | grep favicon.ico
```

No need to figure out which component is the source of the
clog!  It's safe to add it everywhere:

```
totty tail -f /var/log/apache2/access.log | totty grep 404 | totty grep favicon.ico
```

### Alternatives

You may see suggestions to use the `script` command with particular
flags for this purpose: `script --return -c "command to execute" /dev/null`.
This has several downsides:

- it can be hard to remember the exact invocation
- it requires you to escape the command to invoke
- it is not portable (none of these flags are supported on macOS)
- it's long and ugly

By comparison, `totty` works anywhere that supports POSIX
pseudo-terminals, is easy to remember, is easy to introduce to a
working pipe, and requires no escaping or modification of the pipe
component that you're unclogging.
