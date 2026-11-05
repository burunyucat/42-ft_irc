# Guide d'implémentation IRC

Le travail restant comprend le parser, le dispatch des commandes,
**l'implémentation des handlers nécessaires au sujet**, et les notifications
de départ. Des handlers supplémentaires sont recommandés pour la compatibilité
avec le client IRC de référence (voir ci-dessous). Les fonctions
`parseCommand()`, `executeCommand()` et
`notifyClientQuit()` se trouvent dans
[`src/ServerCommands.cpp`](src/ServerCommands.cpp), avec leurs déclarations dans
[`include/Server.hpp`](include/Server.hpp).

Les autres handlers sont seulement déclarés dans `include/Server.hpp` : pour ceux
à implémenter, adapter leurs signatures, écrire leurs définitions et les appeler
depuis `executeCommand()`. `handleQuit()` et son dispatch sont déjà faits : ne pas
les réimplémenter.

> **Toute réponse ou notification IRC doit être ajoutée à l'`outputBuffer` du
> client destinataire. Les handlers ne doivent jamais appeler `send()` directement.**
> Cela concerne les erreurs, les réponses de succès et les messages destinés aux
> autres clients. Voir les exemples et le circuit d'envoi en section 4.

## 1. `parseCommand(line, newCommand)`

Transformer une ligne sans `\r\n` en `Command` :

```text
PRIVMSG Bob :Bonjour à tous
→ name = "PRIVMSG", params = ["Bob", "Bonjour à tous"]
```

- Le `:` introduit le dernier paramètre : conserver les espaces qui suivent,
  mais retirer ce `:` introducteur.
- `QUIT` donne zéro paramètre ; `QUIT :` donne un paramètre vide (`""`).
- Convertir uniquement le nom de commande en majuscules : `quit` devient `QUIT`,
  sans modifier le texte des paramètres.
- Gérer les lignes vides sans accès invalide.
- Retourner `true` si l'analyse réussit, `false` sinon.

Le parser traite la syntaxe, pas les permissions. L'identité de l'émetteur vient
du `Client`, jamais d'un préfixe fourni par l'utilisateur.

## 2. `executeCommand(client, command)`

Compléter le choix des handlers selon `command.name`, en conservant les cas déjà
implémentés. Chaque handler doit, selon la commande :

- vérifier les paramètres, l'enregistrement et les permissions ;
- mettre à jour l'état des clients et des salons ;
- ajouter les réponses, erreurs numériques et notifications nécessaires aux
  `outputBuffer` des clients destinataires (voir section 4).

Pour vérifier l'accès et l'enregistrement, les handlers utilisent deux types
d'informations : la configuration du serveur et l'état du client qui a envoyé
la commande (`client`, passé par référence à `executeCommand()`).

- `m_password` est un membre privé de la classe `Server`, déclarée dans
  [`include/Server.hpp`](include/Server.hpp). Il contient le mot de passe commun
  d'accès au serveur, fourni au lancement de `ircserv`. Les handlers étant des
  méthodes de `Server`, ils peuvent y accéder directement pour vérifier `PASS`.
- `client.authenticated` est un booléen de la structure `Client`, définie dans
  [`include/Client.hpp`](include/Client.hpp). Il indique si ce client a fourni
  le bon mot de passe avec `PASS` ; il vaut initialement `false`.
- `client.registered` est un autre booléen de la structure `Client`. Il indique
  si ce client a terminé son enregistrement IRC (`PASS` accepté, `NICK` et `USER`
  valides) ; il vaut initialement `false`. Les handlers doivent le vérifier avant
  d'autoriser les commandes nécessitant un enregistrement.

Le mot de passe attendu appartient donc au serveur, tandis que les états
d'authentification et d'enregistrement sont conservés séparément pour chaque client.

`executeCommand()` retourne `void` : aucun booléen de succès ou d'échec n'est
attendu. Si une commande est refusée, le handler ajoute lui-même l'erreur IRC
dans le `client.outputBuffer`, puis termine avec `return;`, sans appliquer
l'action refusée. La couche réseau transmet ensuite cette réponse au client.
Un refus de commande ne demande pas à lui seul la déconnexion et n'empêche pas
le traitement des commandes suivantes. Celui-ci s'arrête uniquement si
`client.disconnectRequested` est positionné.

Les signatures des `handlePass()`, `handleNick()`, etc., sont à adapter pour
recevoir les données nécessaires, par exemple :

```cpp
void handlePass(Client& client, const Command& command);
```

### Handlers à implémenter

Ajouter uniquement leurs appels dans le dispatch ne suffit pas : leur logique
doit aussi être écrite.

#### Nécessaires aux fonctionnalités obligatoires du sujet

| Handler | Responsabilité |
|---|---|
| `handlePass` | Comparer le mot de passe reçu à `m_password` et mettre à jour `authenticated`. |
| `handleNick` | Valider le pseudonyme, vérifier sa disponibilité et gérer ses changements. |
| `handleUser` | Valider et enregistrer les informations utilisateur. |
| `handleJoin` | Rejoindre ou créer un salon et vérifier ses restrictions d'accès. |
| `handlePrivmsg` | Transmettre un message à un utilisateur ou à un salon. |
| `handleInvite` | Vérifier les droits et inviter un utilisateur dans un salon. |
| `handleMode` | Consulter ou modifier les modes du salon (`i`, `t`, `k`, `o`, `l`). |
| `handleKick` | Vérifier les droits, notifier l'expulsion et retirer le membre du salon. |
| `handleTopic` | Consulter ou modifier le sujet selon les permissions. |

#### Recommandés, mais non explicitement imposés par le sujet

| Handler | Responsabilité |
|---|---|
| `handlePart` | Notifier le départ et retirer le membre du salon. |
| `handlePing` | Préparer la réponse `PONG`. |
| `handlePong` | Traiter la réponse à un éventuel `PING` du serveur. |
| `handleNames` | Préparer la liste des membres visibles d'un salon. |

Ces quatre commandes ne sont pas explicitement exigées dans `en.subject.pdf`.
Elles peuvent néanmoins devenir nécessaires au bon fonctionnement du client de
référence, qui doit se connecter et fonctionner sans erreur. En particulier,
`PING`/`PONG` servent à vérifier que la connexion est toujours active.
L'absence d'un handler `NAMES` n'exonère pas de préparer les réponses de liste
des membres attendues lors d'un `JOIN`.

L'enregistrement ne doit être finalisé (`registered = true`) qu'après un `PASS`
accepté, un pseudonyme valide et des informations `USER` valides. Réévaluer cette
condition après les commandes concernées. Avant l'enregistrement, refuser les
commandes qui le nécessitent, sans empêcher celles qui permettent de s'enregistrer.

## 3. `notifyClientQuit(client)`

Préparer une notification de départ lorsque cette fonction est appelée :

1. Parcourir les salons dont le partant est membre.
2. Réunir les autres membres, sans doublons (`std::set<int>`, par exemple).
3. Ajouter **un seul `QUIT` par destinataire** dans son `outputBuffer`, avec
   `client.quitReason` et un `\r\n` final.

Exemple simplifié, avec un préfixe limité au pseudonyme :

```text
:Alice QUIT :À bientôt\r\n
```

Exclure le partant et les clients sans salon commun. Vérifier que chaque
destinataire existe encore dans `m_clientRegistry`. Ne pas modifier les salons ni
effacer le client : son identité et ses appartenances sont encore disponibles.
Ne pas produire de notification avec une identité invalide.

Cette fonction est aussi appelée lors d'un départ réseau sans commande `QUIT`,
avec la raison par défaut `"Connection closed"`.

## 4. Règles communes aux handlers

### Notifier un client : toujours passer par son `outputBuffer`

**Modifier l'état du serveur ne prévient pas les clients.** Pour qu'un client
apprenne une arrivée dans un salon, un changement de pseudo, une expulsion ou
une erreur, le handler doit préparer le message IRC correspondant et l'ajouter
à son buffer de sortie.

`outputBuffer` est un membre `std::string` de la structure `Client`, définie dans
[`include/Client.hpp`](include/Client.hpp). Chaque client possède son propre
buffer : **utiliser celui du destinataire, pas systématiquement celui de
l'émetteur de la commande**.

```cpp
// Exemple : répondre à un PING simple du client courant.
client.outputBuffer += ":ircserv PONG ircserv :token\r\n";

// Exemple : notifier un autre client, retrouvé dans le registre.
std::map<int, Client>::iterator recipient = m_clientRegistry.find(recipientFd);
if (recipient != m_clientRegistry.end())
    recipient->second.outputBuffer += ":Alice PRIVMSG Bob :Bonjour\r\n";
```

Ces messages illustratifs doivent être construits avec les identités et
paramètres réels de la commande.

- **Ajouter avec `+=` ou `append()`, jamais avec `=`**, qui effacerait les messages
  déjà en attente.
- **Terminer chaque message par `\r\n`**, sinon le client ne reçoit pas une ligne
  IRC complète.
- Pour une notification à plusieurs clients, ajouter le message au buffer de
  **chaque destinataire concerné**.
- **Ne pas appeler `send()` depuis un handler.** La boucle réseau active
  `POLLOUT` quand un buffer n'est pas vide, puis `processClientOutput()` effectue
  l'envoi et conserve les octets qui n'ont pas encore pu être transmis.
- `Util::sendErrorMsg()` affiche une erreur côté serveur : **ce n'est pas une
  réponse IRC envoyée au client**.

```text
Handler → outputBuffer du destinataire → poll/POLLOUT → processClientOutput/send
```

### État et destinataires

- Une modification d'état doit être accompagnée des notifications IRC prévues.
- `KICK` et `PART` concernent un salon : ne pas mettre à jour `disconnectRequested`.
  Mettre à jour les appartenances et supprimer le salon s'il devient vide.
- Pour `KICK`, notifier aussi le client expulsé avant de le retirer du salon.
- Utiliser `m_clientRegistry.find(fd)` pour consulter un client : `operator[]`
  créerait une entrée si le FD est absent.
