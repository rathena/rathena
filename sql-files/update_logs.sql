#ZenyLog types (M)onsters,(T)rade,(V)ending Sell/Buy,(S)hop Sell/Buy,(N)PC Change amount,(A)dministrators
#Database: log
#Table: zenylog
CREATE TABLE `zenylog` (
  `id` int(11) NOT NULL auto_increment,
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `char_id` int(11) NOT NULL default '0',
  `src_id` int(11) NOT NULL default '0',
  `type` set('M','T','V','S','N','A') NOT NULL default 'S',
  `amount` int(11) NOT NULL default '0',
  `map` varchar(20) NOT NULL default 'prontera.gat',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
