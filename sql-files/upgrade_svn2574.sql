ALTER TABLE friends ADD `friend_account` int(11) NOT NULL default 0 AFTER `char_id`;
UPDATE friends, `char` SET friends.friend_account=`char`.account_id where friends.friend_id=`char`.char_id;
