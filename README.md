*This project has been created as part of the 42 curriculum by burunyucat, Menil-A.*

# ft_irc

## Description

ft_irc, c'est notre propre serveur IRC, écrit en C++98. L'idée du sujet est simple sur le papier : un vrai client IRC doit pouvoir se connecter à notre serveur, s'authentifier, rejoindre des salons et discuter avec les autres, comme sur n'importe quel serveur IRC.

En pratique, ça oblige à mettre les mains dans pas mal de choses :

- les sockets TCP et la gestion de plusieurs clients en même temps, sans `fork()` et sans threads, avec un seul `poll()` ;
- des I/O non bloquantes, avec des commandes qui peuvent arriver en morceaux (ou plusieurs d'un coup) ;
- le protocole IRC lui-même : le format des messages, les réponses numériques, et tout ce que le client attend pour être content.

Le serveur gère l'authentification par mot de passe, les pseudos, les messages privés et les salons, avec des opérateurs de salon et les modes demandés par le sujet.

### Commandes supportées

| Commande | Ce qu'elle fait |
|---|---|
| `PASS` | Donne le mot de passe du serveur (obligatoire avant le reste) |
| `NICK` | Choisit ou change de pseudo |
| `USER` | Termine l'enregistrement |
| `JOIN` | Rejoint un salon (et le crée s'il n'existe pas) |
| `PART` | Quitte un salon |
| `PRIVMSG` | Envoie un message à quelqu'un ou à un salon |
| `NOTICE` | Pareil, mais sans réponse automatique en retour |
| `TOPIC` | Affiche ou modifie le sujet d'un salon |
| `INVITE` | Invite quelqu'un dans un salon |
| `KICK` | Éjecte quelqu'un d'un salon |
| `MODE` | Change les modes d'un salon |
| `PING` / `PONG` | Vérifie que la connexion est toujours en vie |
| `QUIT` | Se déconnecte proprement |

### Modes de salon

| Mode | Effet |
|---|---|
| `i` | Salon sur invitation uniquement |
| `t` | Seuls les opérateurs peuvent changer le topic |
| `k` | Mot de passe pour rejoindre le salon |
| `o` | Donne ou retire le statut d'opérateur |
| `l` | Limite le nombre de membres |

`+` active un mode, `-` le retire. Par exemple : `MODE #salon +k secret` ou `MODE #salon -l`.

## Instructions

### Compilation

```bash
make
```

Ça produit l'exécutable `ircserv`. Les règles habituelles sont là aussi : `clean`, `fclean` et `re`.

### Lancement

```bash
./ircserv <port> <password>
```

- `port` : le port d'écoute (entre 1 et 65535, par exemple `6667`) ;
- `password` : le mot de passe que chaque client devra donner pour se connecter.

```bash
./ircserv 6667 motdepasse
```

### Se connecter avec un client IRC

Avec irssi par exemple :

```text
/connect localhost 6667 motdepasse
```

### Tester à la main avec `nc`

Pour voir ce qui se passe vraiment, `nc` reste le plus pratique :

```bash
nc -C localhost 6667
```

Puis on tape les commandes une par une :

```text
PASS motdepasse
NICK alice
USER alice 0 * :Alice
JOIN #test
PRIVMSG #test :salut tout le monde
```

Le `-C` sert à envoyer `\r\n` en fin de ligne, comme un vrai client. Pour vérifier que le serveur gère bien les commandes coupées en morceaux, on peut taper une partie de la commande, faire `Ctrl+D`, puis la suite : le serveur doit attendre la fin de la ligne avant de la traiter.

### Pour aller plus loin

Le fonctionnement interne (couche TCP, boucle `poll()`, buffers, parser, dispatch des commandes) est détaillé dans [ARCHITECTURE.md](ARCHITECTURE.md).

## Resources

Ce qui nous a le plus servi pendant le projet :

- [C++ Socket Programming (TutorialsPoint)](https://www.tutorialspoint.com/cplusplus/cpp_socket_programming.htm) : un bon point de départ pour comprendre `socket()`, `bind()`, `listen()` et `accept()` avant d'attaquer `poll()`.
- [RFC 2812 – Internet Relay Chat: Client Protocol](https://www.rfc-editor.org/info/rfc2812/#section-3.2.1) : la référence pour le format des commandes et les réponses numériques. La section 3.2 sur les opérations de salon (`JOIN`, `PART`, `MODE`, `TOPIC`, `INVITE`, `KICK`) a été notre point d'appui pour toute la gestion des salons.
- [IRC Commands Cheat Sheet (ircbits)](https://ircbits.com/articles/irc-commands-cheat-sheet) : un résumé rapide des commandes côté utilisateur, pratique pour savoir quoi tester.
- [IRCnet Webchat](https://webchat.ircnet.net/) : un client web branché sur un vrai réseau IRC. On s'en est servi pour comparer avec le comportement d'un « vrai » serveur quand on avait un doute sur une réponse.

### Utilisation de l'IA

On a utilisé l'IA surtout comme support : pour reformuler des passages de la RFC, nous aider à comprendre certains comportements attendus par les clients IRC, et relire la documentation. Le code a été écrit, testé et compris par nous, et tout ce qui venait d'une suggestion a été vérifié avant d'être gardé.
