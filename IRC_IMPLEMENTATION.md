# Guide d'implémentation IRC

Les fonctions restant à compléter (`parseCommand()`, `executeCommand()` et
`notifyClientQuit()`) se trouvent dans
[`src/ServerCommands.cpp`](src/ServerCommands.cpp), avec leurs déclarations dans
[`include/Server.hpp`](include/Server.hpp).

Le handler `handleQuit()` et son appel depuis `executeCommand()` sont déjà
implémentés. Le parser et les notifications de départ restent à réaliser pour
que le traitement d'un `QUIT` reçu du réseau soit complet.

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

Le dispatch de `QUIT` vers `handleQuit(client, command)` est déjà en place.
Compléter le choix des handlers pour les autres commandes selon `command.name`,
en conservant ce cas. Chaque handler doit, selon la commande :

- vérifier les paramètres, l'enregistrement et les permissions ;
- mettre à jour l'état des clients et des salons ;
- préparer les réponses, erreurs numériques et notifications nécessaires.

`m_password` contient le mot de passe attendu. `authenticated` indique son
acceptation ; `registered` indique que l'enregistrement IRC est terminé.

Retourner `true` en cas de succès, `false` pour un échec récupérable.
**Retourner `false` n'envoie pas de réponse IRC et ne déconnecte pas le client.**
Le handler doit lui-même préparer l'erreur destinée à l'utilisateur.

Les signatures des `handlePass()`, `handleNick()`, etc., sont à adapter pour
recevoir les données nécessaires, par exemple :

```cpp
void handlePass(Client& client, const Command& command);
```

## 3. Handler `QUIT` — déjà implémenté

Sa déclaration est déjà adaptée dans `include/Server.hpp`. Son implémentation
conserve seulement la raison et demande le départ :

```cpp
void Server::handleQuit(Client& client, const Command& command)
{
    if (!command.params.empty())
        client.quitReason = command.params[0];
    client.disconnectRequested = true;
}
```

Sans paramètre, la raison reste `"Connection closed"` ; avec `QUIT :`, elle devient
vide. Le parser doit avoir retiré le `:` introducteur.

**Ne pas appeler `disconnectClient()` ni `notifyClientQuit()`, et ne pas retirer
le client des salons dans ce handler.** L'appel de notification et le nettoyage
sont déjà pris en charge ; le contenu de `notifyClientQuit()` reste à implémenter
(section 4). Les commandes suivantes de ce client ne seront plus
exécutées. Un dernier message ajouté au buffer du partant n'est pas garanti envoyé.

## 4. `notifyClientQuit(client)`

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

## 5. Règles communes aux handlers

- Ajouter les messages à l'`outputBuffer` du destinataire, sans écraser son contenu,
  et terminer chaque message par `\r\n`. Ne pas envoyer directement.
- Une modification d'état doit être accompagnée des notifications IRC prévues.
- `KICK` et `PART` concernent un salon : ne pas utiliser `disconnectRequested`.
  Mettre à jour les appartenances et supprimer le salon s'il devient vide.
- Pour `KICK`, notifier aussi le client expulsé avant de le retirer du salon.
- Utiliser `m_clientRegistry.find(fd)` pour consulter un client : `operator[]`
  créerait une entrée si le FD est absent.
