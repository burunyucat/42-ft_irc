# Architecture du serveur ft_irc

Le serveur IRC est divisé en deux grandes parties :

- **La couche réseau (TCP)** : gère les connexions et le transport des données entre les clients IRC et le serveur.
- **La couche IRC** : interprète les commandes reçues, applique les règles du protocole IRC et gère l'état du serveur (utilisateurs, salons, permissions, etc.).

Cette séparation permet de distinguer la communication réseau de la logique applicative.

## 1. Couche TCP

La couche TCP est responsable de la communication réseau entre les clients IRC et le serveur.

Elle s'occupe notamment de :

- Créer et configurer les sockets.
- Accepter les nouvelles connexions TCP.
- Surveiller les sockets grâce à `poll()`.
- Recevoir les octets avec `recv()` et les stocker dans les buffers d'entrée.
- Extraire les commandes IRC complètes pour les transmettre au parser.
- Envoyer les réponses préparées par la couche IRC avec `send()`.
- Gérer les déconnexions et les erreurs réseau.

### 1.1 Les sockets

Le serveur possède deux types de sockets :

**Socket d'écoute (listening socket)**

Créée au démarrage du serveur, elle est associée à une adresse IP et à un port grâce à `bind()`, puis placée en écoute avec `listen()`.

Elle sert uniquement à recevoir les nouvelles demandes de connexion TCP.

Lorsqu'une connexion est disponible, `accept()` retourne une nouvelle socket dédiée au client. La socket d'écoute reste disponible pour les prochaines connexions.

**Sockets clients (connected sockets)**

Chaque client connecté possède une socket TCP dédiée côté serveur.

Ces sockets servent à recevoir les données envoyées par les clients et à leur transmettre des réponses.

Sous Linux, chaque socket est identifiée dans le programme par un **file descriptor (FD)**.

L'acceptation d'une connexion TCP ne signifie pas que l'utilisateur est authentifié auprès du serveur IRC. L'authentification intervient ensuite, dans la couche IRC.

### 1.2 La boucle événementielle et `poll()`

Le serveur doit gérer plusieurs clients simultanément, sans utiliser `fork()` et sans bloquer les opérations réseau.

Pour cela, il utilise une **boucle événementielle** reposant sur `poll()` et des sockets non bloquantes.

`poll()` permet de surveiller plusieurs file descriptors en un seul appel et d'identifier ceux sur lesquels des opérations sont possibles.

Chaque socket surveillée est représentée par une structure `pollfd` contenant :

- `fd` : le descripteur à surveiller.
- `events` : les événements que le programme souhaite surveiller.
- `revents` : les événements détectés par le noyau.

Principaux événements :

| Événement | Signification |
|---|---|
| `POLLIN` | Lecture possible ou connexion en attente d'acceptation |
| `POLLOUT` | Écriture possible sans blocage |
| `POLLHUP` | Condition de fermeture signalée |
| `POLLERR` | Erreur sur le descripteur |
| `POLLNVAL` | Descripteur invalide |

**Gestion de `POLLIN`**

Les sockets des clients actifs sont normalement surveillées en permanence avec `POLLIN`, puisqu'un client peut envoyer des données à tout moment.

Lorsqu'un `POLLIN` est détecté, le serveur utilise `recv()` pour récupérer les octets disponibles.

Pour la socket d'écoute, cet événement indique qu'une connexion peut être acceptée avec `accept()`.

**Gestion de `POLLOUT`**

`POLLOUT` est surveillé pour un client uniquement lorsque son `outputBuffer` contient des données en attente d'envoi.

Il indique que le buffer d'envoi TCP du noyau dispose de place pour accepter des données.

Surveiller constamment `POLLOUT` serait inefficace : une socket TCP est généralement prête en écriture, même lorsque le serveur n'a rien à envoyer. `poll()` retournerait donc immédiatement et provoquerait une consommation inutile de CPU (*busy waiting*).

**Fonctionnement de la boucle événementielle**

1. Le serveur prépare les événements à surveiller.
2. Il appelle `poll()`, qui attend qu'un événement soit disponible.
3. Le noyau renseigne les champs `revents` des descripteurs concernés.
4. Le serveur parcourt les événements et effectue les opérations appropriées (`accept()`, `recv()`, `send()`, etc.).
5. Il recommence la boucle.

Cette architecture permet de gérer de nombreux clients avec un seul thread, sans exécuter leurs opérations en parallèle.

`poll()` ne réalise pas lui-même les lectures et écritures : il signale uniquement leur disponibilité.

### 1.3 Les buffers

Chaque client possède deux buffers applicatifs, stockés côté serveur :

- **`inputBuffer`** : conserve les octets reçus du client qui n'ont pas encore été entièrement traités.
- **`outputBuffer`** : conserve les octets que le serveur doit transmettre au client.

Ces buffers sont distincts des buffers TCP internes gérés automatiquement par le noyau Linux.

**Réception (`inputBuffer`)**

TCP transporte un flux d'octets et ne garantit pas qu'un appel à `recv()` corresponde à une commande IRC complète.

Par exemple, le serveur peut recevoir :

```text
NICK Alice\r\nJOIN #general\r\nPRIVMSG Bob :Bonj
```

Dans cet exemple :

- `NICK Alice` est une commande complète.
- `JOIN #general` est une commande complète.
- `PRIVMSG Bob :Bonj` est une commande incomplète.

Le serveur recherche les délimiteurs `\r\n`, extrait chaque commande complète et la transmet individuellement au parser.

Les octets restants sont conservés dans l'`inputBuffer` jusqu'à la prochaine réception.

**Envoi (`outputBuffer`)**

Lorsqu'une commande IRC nécessite une réponse ou une notification, celle-ci est ajoutée à l'`outputBuffer` du client destinataire.

Le serveur surveille alors `POLLOUT` sur sa socket et utilise `send()` pour transférer les octets vers le buffer d'envoi TCP du noyau.

Un appel à `send()` peut n'accepter qu'une partie des données. Les octets restants doivent donc être conservés dans l'`outputBuffer` pour un prochain envoi.

Une fois le buffer vidé, la surveillance de `POLLOUT` peut être désactivée pour ce client.

## 2. Couche IRC

La couche IRC est responsable de l'interprétation des commandes et de l'application des règles du protocole.

Elle comprend principalement le parser, le gestionnaire de commandes et les structures représentant l'état du serveur.

### 2.1 Parser

Le parser reçoit les commandes IRC complètes extraites des `inputBuffer` par la couche réseau.

Son rôle consiste à analyser leur syntaxe et à les transformer en données structurées, représentées par exemple par une structure `Command`.

```cpp
struct Command
{
    std::string name;
    std::vector<std::string> params;
};
```

- `name` : nom de la commande.
- `params` : paramètres de la commande.

Par exemple, pour la ligne :

```text
PRIVMSG Bob :Bonjour Alice
```

Le parser produit :

```text
name = "PRIVMSG"
params[0] = "Bob"
params[1] = "Bonjour Alice"
```

Le parser ne réalise aucune opération sur les salons ou les utilisateurs. Il se limite à l'analyse syntaxique.

La commande structurée est ensuite transmise au gestionnaire de commandes, accompagnée de l'identité du client qui l'a envoyée.

### 2.2 Gestionnaire de commandes (CommandHandler)

Le gestionnaire de commandes est responsable de l'exécution des commandes IRC après leur interprétation par le parser.

Il vérifie leur validité, l'état du client et ses permissions, puis applique les opérations demandées : création et gestion des salons, envoi de messages, modification des modes, expulsion de membres, etc.

Il peut ainsi :

- Consulter l'état du serveur.
- Modifier les informations des clients et des salons.
- Préparer des réponses ou notifications IRC.
- Ajouter ces réponses aux buffers de sortie des clients concernés.

La couche réseau se charge ensuite de leur transmission avec `poll()` et `send()`.

**Commandes à gérer**

| Commande | Responsabilité |
|---|---|
| `PASS` | Vérifier le mot de passe du serveur |
| `NICK` | Définir ou changer le pseudonyme |
| `USER` | Enregistrer les informations de l'utilisateur |
| `JOIN` | Rejoindre un salon, le créer si nécessaire et vérifier les restrictions |
| `PRIVMSG` | Envoyer un message à un utilisateur ou aux membres d'un salon |
| `KICK` | Expulser un membre d'un salon |
| `INVITE` | Inviter un utilisateur dans un salon |
| `TOPIC` | Consulter ou modifier le sujet d'un salon |
| `MODE` | Consulter ou modifier les modes d'un salon et les privilèges des membres |

**Modes de salon à gérer**

| Mode | Responsabilité |
|---|---|
| `i` | Entrée uniquement sur invitation |
| `t` | Modification du topic réservée aux opérateurs |
| `k` | Clé d'entrée du salon |
| `o` | Attribution ou retrait du statut d'opérateur |
| `l` | Limitation du nombre de membres |

Le signe `+` active ou ajoute un mode, tandis que le signe `-` le désactive ou le retire.

**Commandes supplémentaires recommandées**

- `PART` : quitter un salon sans se déconnecter du serveur.
- `QUIT` : quitter le serveur proprement.
- `PING` : vérifier la disponibilité d'une connexion.
- `PONG` : répondre à un `PING`.
- `NAMES` : obtenir la liste des membres d'un salon.

Certaines commandes supplémentaires peuvent être nécessaires pour assurer la compatibilité avec le client IRC de référence.

### 2.3 État interne du serveur

Le serveur conserve en mémoire les informations nécessaires au fonctionnement du protocole IRC.

Trois composants principaux peuvent être utilisés :

**`Server`**

Représente le serveur et coordonne son fonctionnement.

Il peut contenir :

- La socket d'écoute.
- La liste des file descriptors surveillés.
- La collection des clients connectés.
- La collection des salons existants.
- La boucle événementielle et les méthodes réseau.

**`Client`**

Représente un client connecté au serveur.

Il peut contenir :

- Son file descriptor.
- Son pseudonyme et son nom d'utilisateur.
- Son état d'authentification et d'enregistrement.
- Son `inputBuffer`.
- Son `outputBuffer`.

Un client TCP connecté n'est pas nécessairement un utilisateur IRC déjà enregistré.

**`Channel`**

Représente un salon IRC.

Il peut contenir :

- Son nom.
- Son topic.
- La liste de ses membres.
- La liste de ses opérateurs.
- La liste des utilisateurs invités.
- Ses modes et restrictions.

Un channel est uniquement une structure logique conservée en mémoire. Sa création ne nécessite aucune nouvelle socket, aucun thread et aucun processus.

### 2.4 Synchronisation entre le serveur et les clients

Le serveur détient l'état réel des utilisateurs et des salons.

Les clients IRC ne peuvent pas accéder directement aux structures `Client` ou `Channel` du serveur. Ils construisent leur représentation locale à partir des réponses et notifications IRC reçues.

Par exemple, lorsqu'Alice crée un salon avec :

```text
JOIN #gaming
```

Le serveur :

1. Vérifie si le salon existe et le crée si nécessaire.
2. Ajoute Alice à ses membres et lui attribue les droits d'opérateur si elle vient de créer le salon.
3. Prépare une notification `JOIN`.
4. Ajoute les réponses nécessaires dans `alice.outputBuffer`.
5. Transmet ces réponses via la couche TCP.

Le client d'Alice peut alors mettre à jour son interface.

Le même principe s'applique lors d'une expulsion, d'un changement de topic ou de la modification des modes.

**Modifier uniquement l'état interne du serveur ne suffit donc pas : les clients concernés doivent également recevoir les notifications prévues par le protocole IRC.**

Cela ne signifie pas que chaque modification interne doit être communiquée à tous les clients. Les destinataires et les réponses dépendent de la commande et des règles du protocole.

### 2.5 Stockage des messages

Le sujet ne demande pas d'implémenter un historique persistant des discussions.

Les messages sont généralement transmis aux destinataires concernés, puis supprimés des buffers applicatifs une fois acceptés par la couche TCP.

Il faut donc conserver temporairement les données en attente d'envoi, mais pas nécessairement enregistrer tous les messages échangés dans une base de données ou un fichier.

## 3. Cycle complet d'une commande IRC

Exemple : Alice envoie `PRIVMSG Bob :Salut !`.

1. Le client IRC d'Alice transmet la commande via sa connexion TCP.
2. `poll()` détecte `POLLIN` sur la socket d'Alice.
3. `recv()` récupère les octets et les ajoute à `alice.inputBuffer`.
4. Une fois `\r\n` détecté, la commande complète est extraite.
5. Le parser transforme la ligne en structure `Command`.
6. Le `CommandHandler` vérifie les informations et applique la logique IRC.
7. Le serveur construit le message destiné à Bob et l'ajoute à `bob.outputBuffer`.
8. La surveillance de `POLLOUT` est activée pour Bob.
9. `poll()` signale que la socket de Bob est prête en écriture.
10. `send()` transfère les octets vers le buffer d'envoi TCP du noyau, qui se charge de leur transmission.

Le processus fonctionne également pour les commandes qui modifient l'état du serveur, comme `JOIN`, `KICK`, `TOPIC` ou `MODE`.

Dans ces cas, l'exécution de la commande peut modifier les données internes et générer des notifications destinées à un ou plusieurs clients.

## 4. Contraintes techniques importantes

Le projet ft_irc impose notamment :

- L'utilisation du standard **C++98**.
- Les communications via TCP/IP (IPv4 ou IPv6).
- La gestion de plusieurs clients sans blocage.
- L'interdiction de `fork()`.
- L'utilisation d'un seul mécanisme de multiplexage `poll()` ou équivalent pour les opérations réseau.
- Des opérations d'entrée/sortie non bloquantes.
- Le déclenchement des lectures et écritures en fonction des événements remontés par `poll()`.
- La gestion des données reçues partiellement ou de plusieurs commandes reçues simultanément.
- La gestion correcte des erreurs et des déconnexions.

Il ne faut pas utiliser `errno`, après une lecture ou une écriture, pour décider de relancer de nouvelles opérations, conformément aux contraintes spécifiques du sujet.

L'objectif est de disposer d'un serveur capable de maintenir plusieurs connexions TCP, d'interpréter correctement les commandes IRC et de garder son état cohérent tout en communiquant les informations nécessaires aux clients.
