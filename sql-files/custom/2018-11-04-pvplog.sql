CREATE TABLE IF NOT EXISTS `pvplog` (
  `killer_id` int(11) NOT NULL,
  `victim_id` int(11) NOT NULL,
  `date_time` datetime NOT NULL,
  `map` varchar(11) NOT NULL,
  PRIMARY KEY (`killer_id`,`victim_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;