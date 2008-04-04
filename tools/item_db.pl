#!/usr/bin/perl
$db = "item_db";
$nb_columns = 22;
@str_col = (1,2,19,20,21);
$line_format = "([^\,]*),([^\,]*),([^\,]*),([^\,]*),([^\,]*),([^\,]*),([^\,]*),([^\,]*),([^\,]*),([^\,]*),([^\,]*),([^\,]*),([^\,]*),([^\,]*),([^\,]*),([^\,]*),([^\,]*),([^\,]*),([^\,]*),(\{.*\}),(\{.*\}),(\{.*\})";
#$line_format = ;
$create_table = "#
# Table structure for table `item_db`
#

DROP TABLE IF EXISTS `item_db`;
CREATE TABLE `item_db` (
  `id` smallint(5) unsigned NOT NULL default '0',
  `name_english` varchar(50) NOT NULL default '',
  `name_japanese` varchar(50) NOT NULL default '',
  `type` tinyint(2) unsigned NOT NULL default '0',
  `price_buy` mediumint(10) unsigned default NULL,
  `price_sell` mediumint(10) unsigned default NULL,
  `weight` smallint(5) unsigned NOT NULL default '0',
  `attack` smallint(3) unsigned default NULL,
  `defence` tinyint(3) unsigned default NULL,
  `range` tinyint(2) unsigned default NULL,
  `slots` tinyint(2) unsigned default NULL,
  `equip_jobs` int(12) unsigned default NULL,
  `equip_upper` tinyint(8) unsigned default NULL,
  `equip_genders` tinyint(2) unsigned default NULL,
  `equip_locations` smallint(4) unsigned default NULL,
  `weapon_level` tinyint(2) unsigned default NULL,
  `equip_level` tinyint(3) unsigned default NULL,
  `refineable` tinyint(1) unsigned default NULL,
  `view` smallint(3) unsigned default NULL,
  `script` text,
  `equip_script` text,
  `unequip_script` text,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM;
";
printf("%s\n",$create_table);
while ($ligne=<STDIN>)
{
	if ($ligne =~ /[^\r\n]+/)
	{
		$ligne = $&;
		if ($ligne =~ /^\/\//)
		{
			printf("# ");
			$ligne = substr($ligne, 2);
		}
		if ($ligne =~ $line_format) {
			@champ = ($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13,$14,$15,$16,$17,$18,$19,$20,$21,$22);
		} else {
			@champ = ();
		}
		if ($#champ != $nb_columns - 1)
		{
			# Can't parse, it's a real comment
			printf ("%s\n", $ligne);
		} else {
			printf("REPLACE INTO `%s` VALUES (", $db);
			for ($i=0; $i<$#champ; $i++)
			{
				printField($champ[$i],",",$i);
			}
			printField($champ[$#champ],");\n",$#champ);
		}
	}
}


sub printField {
	my ($str, $suffix, $idCol) = @_;
	# Remove first { and last }
	if ($str =~ /{.*}/)
	{
		$str = substr($&,1,-1);
	}
	# If nothing, put NULL
	if ($str eq "") {
		printf("NULL%s", $suffix);
	} else {
		my $flag = 0;
		# Search if it's a string column ?
		foreach $col (@str_col)
		{
			if ($col == $idCol)
			{
				$flag = 1;
				break;
			}
		}
		if ($flag == 1)
		{
			# String column, so escape and add ''
			printf("'%s'%s", escape($str), $suffix);
		} else {
			# Not a string column
			printf("%s%s", $str,$suffix);
		}
	}
}

sub escape {
	my ($str) = @_;
	my @str_splitted = split("'", $str);
	my $result = "";
	for (my $i=0; $i<=$#str_splitted; $i++)
	{
		if ($i == 0) {
			$result = @str_splitted[0];
		} else {
			$result = $result."\\'".@str_splitted[$i];
		}
	}
	return $result
}
