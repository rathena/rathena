# Packet Structure Notation

This document specifies how packets are and should be documented, to
keep packet structure comments consistent in the entire codebase. It
also serves as a guide to those, who are unfamiliar with the general
packet layout.

All mentioned data types are assumed to be little-endian (least-
significant byte first, least significant bit last) and of same size
regardless of architecture.

### Typical description of a packet

```
Notifies the client about entering a chatroom.  
00db <packet len>.W <chat id>.L { <role>.L <name>.24B }* (ZC_ENTER_ROOM)
role:  
  0 = owner (menu)  
  1 = normal  
```

The first line contains a brief description of what the packet does,
or what it is good for, followed by it's `AEGIS` name in parentheses;
first two letters of the `AEGIS` name specify origin (first letter)
and destination (second letter) of the packet. If the packet's name
is not known or is not applicable (rAthena server-server packets),
specify at least these two letters to indicate the direction of the
packet. Do not use `S(end)/R(ecv)` for this, as it is inaccurate and
location dependent (if the description is copied to different server
or other RO-related projects, it might change it's meaning).

If there are multiple versions of the packet, the `AEGIS` name is
appended to the end of the packet's structure instead. If the name
did not change between versions, a `PACKETVER` expression is appended,
such as `(PACKETVER >= 20111111)`.

Second line describes the packet's field structure, beginning with a
`%04x` formatted packet type, followed by the individual fields and
their types. Each field begins with it's name enclosed in angle
brackets ( `<field name>` ) followed by a dot and the data size type.
Field names should be lower-case and without underscores. If other
packets already have a field in common, use that name, rather than
inventing your own (ex. "packet len" and "account id"). Repeated and
optional fields are designated with curly and square brackets
respectively, padded with a single space at each side.

Further lines are optional and either include details about the
the packet's mechanics or further explanation on the packet fields'
values.

### Packet field data size type

 |Field name|Field description|Field size|
 |---|---|---|
 |B|byte|1 byte|
 |W|word|2 bytes|
 |L|long, dword|4 bytes|
 |F|float|4 bytes|
 |Q|quad|8 bytes|

### Variable cases
 
 |Field name|Field description|
 |---|---|
 |nB|n bytes|
 |?B|variable/unknown amount of bytes|
 |nS|n bytes, zero-terminated|
 |?S|variable/unknown amount of bytes, zero-terminated|

### Repetition of packet fields
 
 |Field name|Field description|
 |---|---|
 |{}|repeated block|
 |{}*|variable/unknown amount of consecutive blocks|
 |{}*n|n times repeated block|
 |[]|optional fields|

### Packet origin and destination letters
 
 |Origin|Destination|
 |---|---|
 |A|Account (Login)|
 |C|Client|
 |H|Character|
 |I|Inter|
 |S|Server (any type of server)|
 |Z|Zone (Map)|

### Examples

- Packet with nested repetition blocks:
 
```
/// Presents a textual list of producable items.
/// 018d <packet len>.W { <name id>.W { <material id>.W }*3 }* (ZC_MAKABLEITEMLIST)
/// material id:
///     unused by the client
```

- Packet with multiple versions identified with different AEGIS names:

```
/// Request for server's tick.
/// 007e <client tick>.L (CZ_REQUEST_TIME)
/// 0360 <client tick>.L (CZ_REQUEST_TIME2)
```

- Packet with multiple versions identified with same AEGIS name:

```
/// Cashshop Buy Ack.
/// 0289 <cash point>.L <error>.W (ZC_PC_CASH_POINT_UPDATE)
/// 0289 <cash point>.L <kafra point>.L <error>.W (PACKETVER >= 20070711) (ZC_PC_CASH_POINT_UPDATE)
```

- Packet with combination of both different AEGIS names and different versions with same name:

```
/// Sends hotkey bar.
/// 02b9 { <is skill>.B <id>.L <count>.W }*27 (ZC_SHORTCUT_KEY_LIST)
/// 07d9 { <is skill>.B <id>.L <count>.W }*36 (ZC_SHORTCUT_KEY_LIST_V2, PACKETVER >= 20090603)
/// 07d9 { <is skill>.B <id>.L <count>.W }*38 (ZC_SHORTCUT_KEY_LIST_V2, PACKETVER >= 20090617)
```
 
- Packet for a client command:

```
/// /item /monster.
/// Request to make items or spawn monsters.
/// 013f <item/mob name>.24B (CZ_ITEM_CREATE)
```
