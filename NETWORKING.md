OpenOMF network support
=======================

The original game used IPX/SPX network with "blocking" game logic, each side
had to acknowledge the opponents input before the move could continue. This was
clearly unsuitable in several ways, so networking is probably where OpenOMF
deviates most from the original engine. This document will try to describe the
general approach and implementation of OpenOMF network support.

Protocols and libraries
-----------------------

OpenOMF uses [ENet](http://enet.bespin.org/) for all its core networking. ENet
provides reliable and unreliable channelized networking over UDP. The main fork
of ENet does not support IPv6, and thus neither does OpenOMF at this time.

NAT traversal is attempted via several approaches:

* NAT-PMP support is provided by
  [libnatpmp](http://miniupnp.free.fr/libnatpmp.html)
* UPnP support is provided by [miniupnpc](https://miniupnp.tuxfamily.org/)

When using the network lobby, 2 more approaches are used:

* [UDP hole punching is
  attempted](https://en.wikipedia.org/wiki/UDP_hole_punching)
* If all else fails, the lobby server will relay the game packets between the 2
  parties itself

The ENet packets themselves consist of 3 kinds of packets, each on their own
channel:

* Channel 0: Lobby communication packets (yells/whispers/challenges, etc). These
  are sent reliably
* Channel 1: Peer to peer packets (clock alignment, MELEE/VS inputs, etc). These
  are sent in a mix of reliable and unreliable.
* Channel 2: Fight event packets (What input was made on which game tick). These
  are sent unreliably.

The details of these protocols is not currently documented here. Please refer to
the source code via the following links:

* [OpenOMF lobby
  server](https://github.com/omf2097/openomf_lobby/blob/main/src/openomf_lobby_client.erl)
* [OpenOMF lobby
  client](https://github.com/omf2097/openomf/blob/master/src/game/scenes/lobby.c)
* [OpenOMF network
  controller](https://github.com/omf2097/openomf/blob/master/src/controller/net_controller.c)

The lobby
---------

OpenOMF's network lobby provides several features:

* Tracks user presence (available/challenging/fighting, etc) and win/loss count
* Provides public and private chat features (via yell and whisper)
* Allows users to challenge each other and to accept/reject the challenge
* Provides assistance with NAT/firewall traversal

Planned features include:

* Specator mode, to allow other users to watch an ongoing or previous Fight
* Persistent user accounts to track long term wins/losses, etc
* Tournament bracket mode, where a group of players systematically play to find
  a single winner

The lobby is implemented in Erlang, using a [Erlang port of the ENet
library](https://github.com/flambard/enet).

Peer to peer mode
-----------------

OpenOMF also provides for a way for 2 players to play against each other without
using the public lobby, via the 'start server' and 'connect to server' options
under the network menu. The 'start server' side will open a port (and, if
configured, use NAT-PMP/UPnP to try to open an external port) and display the
open port(s) and public IP to the user. That user can then provide that
information to the other party for use in the 'connect to server menu'.

Starting a match
----------------

Regardless of whether the lobby or a peer to peer connection is used to
establish a connection between 2 OpenOMF instances, the games will now proceed
to connect to each other, check compatability, and align the in-game 'clock'.

Aligning the clocks
-------------------

It is critical that both sides agree on when a given game tick occurs. A simple
protocol is used to try to determine the round trip time (RTT) between both
sides, such that when A send a packet at time T, it can predict that B will see
that packet at T+RTT/2.

Pilot/HAR/Arena selection
-------------------------

In these screens, each user's key events are simply sent reliably over the wire,
it is assumed that they arrive and are processed correctly by both sides.

The Arena
---------

During the arena, rollback networking is used, essentially a custom
implementation of [GGPO](http://ggpo.net). Each side sends all its un-confirmed
inputs, along with the tick the input was made to the other side, while
receiving the same. Once one side has seen inputs from the far side past a
certain tick, they know that they know everything they need to rewind the local
game state and replay it with the new inputs to get the agreed on state. Both
sides are constantly exchanging their inputs along with a hash of the game state
at certain ticks. When replaying, if a hash mismatch is detected, the match
ends.

During the arena, the "frame advantage" is also tracked and attempted to be
controlled. Frame advantage is if one side is consistently "ahead" of the other
side when inputs come in. This is likely caused by changing latency conditions.
This can lead to the "faster" side not seeing their opponent move until the move
has been executing for several frames, or even at all. The engine will attempt
to slow the faster side down to mitigate this.

Rollback networking is considered the best approach for fighting games, but it
can only handle so much network latency and packet loss. In cases with high
latency or loss, significant popping and teleporting will still be observed.
Some additional work can be done to OpenOMF to mitigate this, but this is
ongoing.

When rolling back and replaying the game state, special attention has to be
applied to the sounds. Sometimes after a rollback a sound that WAS playing is
no longer playing, or a sound may have been introduced during the replay that
was not playing before. OpenOMF tracks the active sounds and will trigger a fade
out on old sounds and a fade in on new sounds to try to minimize popping and
other audio artifacts.

Refer to the resources on the GGPO website for further details.

Configuration
--------------

There are several networking options available in the config file that are not
currently available in the menus.

* `net_lobby_address` The address of the OpenOMF lobby to connect to, defaults
   to lobby.openomf.org
* `net_listen_port_start` When trying to bind to a port, start at this number, 0
  means any port
* `net_listen_port_end` When trying to bind to a port, don't go past this number,
  0 means don't stop
* `net_ext_port_start` When using PMP or UPnP, only use external ports at or
  above this number, defaults to 0 for any external port
* `net_ext_port_end` When using PMP or UPnP, don't use an external port above
  this number, 0 means don't have an upper limit
* `net_use_pmp` Whether to attempt using NAT-PMP, if you are having trouble with
  a broken or misconfigured PMP server, disabling it here may help
* `net_use_upnp` Whether to attempt using UPnP, if you are having trouble with
  a broken or misconfigured UPnP server, disabling it here may help

Command line switches
---------------------

It can be convienent to start the game and jump straight into a match, so the
following command line switches are provided

* `--connect <IP or Host>` Connect directly to the given peer
* `--port <port>` Use a specific port when connecting to the peer provided by
  `--connect`
* `--lobby` Jump straight into the lobby
* `--lobby-addr <IP or Host>` Instead of using the lobby address from the
  config file, use address for the lobby


