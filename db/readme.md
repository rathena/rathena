# Import Directories

## What is the import directory for?

The `import/` directory provides a way for you to change your config settings without the need to even touch the main `/conf/` and `/db/` files.

By placing your custom entries into the `import/` directory within these two locations, your core files will not need to have any conflicts resolved when you update your server. You store your changes, and the rest is updated with rAthena.

## How does this work?

Think of "import" as in "override". Place only the settings you have changed in the import files, or settings you are "overriding".

For example, when setting up a server there are always a few config settings that users would like to change in order for rAthena to suit their needs. The following example will show you how to use the `/db/import/` directory correctly. (for `/conf/import/` examples, see [/conf/readme.md](/conf/readme.md))

### Achievements
---
We want to add our own custom achievement that can be given to a player via an NPC Script and another that we can give to our GMs.

#### /db/import/achievement_db.yml

```yml
    - Id: 280000
      Group: None
      Name: Emperio
      Reward:
        TitleId: 1035
      Score: 50
    - Id: 280001
      Group: None
      Name: Staff
      Reward:
        TitleId: 1036
      Score: 50
```


### Instances
---
We want to add our own customized Housing Instance.

#### /db/import/instance_db.yml

```yml
    - Id: 35
      Name: Home
      IdleTimeOut: 900
      Enter:
        Map: 1@home
        X: 24
        Y: 6
      AdditionalMaps:
        - Map: 2@home
        - Map: 3@home
```


### Mob Alias
---
We want to make Porings look like Baphomet.

#### /db/import/mob_avail.yml

```yml
    - Mob: PORING
      Sprite: BAPHOMET
```


### Custom Maps
---
We want to add our own custom maps. For this we need to add our map names to `import/map_index.txt` and then to the `import/map_cache.dat` file for the Map Server to load.

#### /db/import/map_index.txt

```
    1@home	1250
    2@home
    3@home
    ev_has
    shops
    prt_pvp
```


### Item Trade Restrictions
---
We want to ensure that specific items cannot be traded, sold, dropped, placed in storage, etc.

#### /db/import/item_db.yml

```yml
    - Id: 34000 # Old Green Box
      Trade:
        NoDrop: true
        NoTrade: true
        TradePartner: true
        NoSell: true
        NoCart: true
        NoStorage: true
        NoGuildStorage: true
        NoMail: true
        NoAuction: true
    - Id: 34001 # House Keys
      Trade:
        NoDrop: true
        NoTrade: true
        TradePartner: true
        NoSell: true
        NoCart: true
        NoStorage: true
        NoGuildStorage: true
        NoMail: true
        NoAuction: true
    - Id: 34002 # Reputation Journal
      Trade:
        NoDrop: true
        NoTrade: true
        TradePartner: true
        NoSell: true
        NoCart: true
        NoStorage: true
        NoGuildStorage: true
        NoMail: true
        NoAuction: true
```


### Custom Quests
---
We want to add our own custom quests to the quest_db.

#### /db/import/quest_db.yml

```yml
    - Id: 89001
      Title: "Reputation Quest"
    - Id: 89002
      Title: "Reputation Quest"
```



We cannot stress enough how helpful this system is for everyone. The majority of git conflicts will simply go away if users make use of the `import/` system.
