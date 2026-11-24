*This project has been created as part of the 42 curriculum by agense, tle-rhun and meaba.*

# ft_irc

## Description

ft_irc is our own IRC server, written in C++98. The idea is simple on paper: a real IRC client should be able to connect to our server, authenticate, join channels and talk with other people, just like on any other IRC server.

In practice, it means getting your hands dirty with quite a few things:

- TCP sockets and handling many clients at the same time, without `fork()` and without threads, using a single `poll()`;
- non-blocking I/O, with commands that can arrive in pieces (or several at once);
- the IRC protocol itself: message format, numeric replies, and everything the client expects to keep it happy.

The server handles password authentication, nicknames, private messages and channels, with channel operators and the modes required by the subject.

### Supported commands

| Command | What it does |
|---|---|
| `PASS` | Sends the server password (required before anything else) |
| `NICK` | Sets or changes your nickname |
| `USER` | Completes the registration |
| `JOIN` | Joins a channel (and creates it if it doesn't exist) |
| `PART` | Leaves a channel |
| `PRIVMSG` | Sends a message to a user or a channel |
| `NOTICE` | Same thing, but never triggers an automatic reply |
| `TOPIC` | Shows or changes a channel's topic |
| `INVITE` | Invites someone to a channel |
| `KICK` | Kicks someone out of a channel |
| `MODE` | Changes a channel's modes |
| `PING` / `PONG` | Checks that the connection is still alive |
| `QUIT` | Disconnects cleanly |

### Channel modes

| Mode | Effect |
|---|---|
| `i` | Invite-only channel |
| `t` | Only operators can change the topic |
| `k` | Password required to join the channel |
| `o` | Gives or takes operator status |
| `l` | Limits the number of members |

`+` turns a mode on, `-` turns it off. For example: `MODE #channel +k secret` or `MODE #channel -l`.

## Instructions

### Build

```bash
make
```

This builds the `ircserv` executable. The usual rules are there too: `clean`, `fclean` and `re`.

### Run

```bash
./ircserv <port> <password>
```

- `port`: the listening port (between 1 and 65535, e.g. `6667`);
- `password`: the password every client has to give to connect.

```bash
./ircserv 6667 mypassword
```

### Connecting with an IRC client

With irssi, for example:

```text
/connect localhost 6667 mypassword
```

### Testing by hand with `nc`

To see what's really going on, `nc` is still the handiest tool:

```bash
nc -C localhost 6667
```

Then type the commands one by one:

```text
PASS mypassword
NICK alice
USER alice 0 * :Alice
JOIN #test
PRIVMSG #test :hi everyone
```

`-C` makes `nc` end each line with `\r\n`, like a real client. To check that the server handles commands split into pieces, type part of a command, hit `Ctrl+D`, then type the rest: the server should wait for the end of the line before processing it.

### Going further

The internals (TCP layer, `poll()` loop, buffers, parser, command dispatch) are described in detail in [ARCHITECTURE.md](ARCHITECTURE.md).

## Resources

What helped us the most during the project:

- [C++ Socket Programming (TutorialsPoint)](https://www.tutorialspoint.com/cplusplus/cpp_socket_programming.htm): a good starting point to understand `socket()`, `bind()`, `listen()` and `accept()` before tackling `poll()`.
- [RFC 2812 – Internet Relay Chat: Client Protocol](https://www.rfc-editor.org/info/rfc2812/#section-3.2.1): the reference for command format and numeric replies. Section 3.2 on channel operations (`JOIN`, `PART`, `MODE`, `TOPIC`, `INVITE`, `KICK`) was what we leaned on for all the channel handling.
- [IRC Commands Cheat Sheet (ircbits)](https://ircbits.com/articles/irc-commands-cheat-sheet): a quick summary of user-side commands, handy to know what to test.
- [IRCnet Webchat](https://webchat.ircnet.net/): a web client connected to a real IRC network. We used it to compare with how a "real" server behaves whenever we weren't sure about a reply.

### Use of AI

We mostly used AI as a support tool: to rephrase parts of the RFC, help us understand some behaviors IRC clients expect, and proofread the documentation. The code was written, tested and understood by us, and anything that came from a suggestion was checked before we kept it.
