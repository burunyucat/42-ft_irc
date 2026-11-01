## Couche TCP

La couche TCP est la partie du programme gerant la communication reseau entre les clients IRC et le serveur.

Ecoute de POLLOUT d'un client seulement quand son outputbuffer n'est pas vide.

Ecoute de POLLIN de tous les clients a tout moment puisqu'ils peuvent envoyer des messages n'importe quand.

## Couche IRC

### Parser

### Gestion des commandes

Commande a gérer:

- `PASS` : Vérifier le mot de passe du serveur |
- `NICK` : Définir ou changer le pseudo ; vérifier sa validité et sa disponibilité |
- `USER` : Enregistrer les informations utilisateur nécessaires à l’inscription |
- `JOIN` : Rejoindre un salon, le créer si nécessaire, vérifier les restrictions d’entrée |
- `PRIVMSG` : Envoyer un message à un utilisateur ou aux autres membres d’un salon |
- `KICK` : Expulser un membre d’un salon après vérification des droits |
- `INVITE` : Inviter un utilisateur dans un salon |
- `TOPIC` : Consulter ou modifier le sujet d’un salon |
- `MODE` : Consulter ou modifier les modes d’un salon et les droits de ses membres |

Pour MODE, gerer:
- `i` : Entree sur invitation
- `t` : Modification du topic reservee aux operateurs
- `k` : Cle d'entree du salon
- `o` : Statut d'operateur d'un membre
- `l` : Nombre maximal de membres

Conseiller :

- `PART` : Quitter un salon en restant connecté
- `QUIT` : Quitter le serveur proprement, avec nettoyage et notifications
- `PING` : Répondre aux vérifications de connexion envoyées par le client
- `PONG` : Traiter les réponses si votre serveur envoie lui-même des PING
