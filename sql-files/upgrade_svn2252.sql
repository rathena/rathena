################################
# NEW friend table             #
# NOTE -> if you dont write a  #
# converter, all your friends  #
# will be deleted              #
################################

DROP TABLE `friends`;

CREATE TABLE `friends` (
  `char_id` int(11) NOT NULL default '0',
  `friend_id` int(11) NOT NULL default '0'
) TYPE=MyISAM;

