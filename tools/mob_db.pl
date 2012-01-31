#!/usr/bin/perl
$db = "mob_db";
$nb_columns = 57;
@str_col = (1,2,3);
$create_table = "#
# Table structure for table `mob_db`
#

DROP TABLE IF EXISTS `mob_db`;
CREATE TABLE `mob_db` (
  `ID` mediumint(9) unsigned NOT NULL default '0',
  `Sprite` text NOT NULL,
  `kName` text NOT NULL,
  `iName` text NOT NULL,
  `LV` tinyint(6) unsigned NOT NULL default '0',
  `HP` int(9) unsigned NOT NULL default '0',
  `SP` mediumint(9) unsigned NOT NULL default '0',
  `EXP` mediumint(9) unsigned NOT NULL default '0',
  `JEXP` mediumint(9) unsigned NOT NULL default '0',
  `Range1` tinyint(4) unsigned NOT NULL default '0',
  `ATK1` smallint(6) unsigned NOT NULL default '0',
  `ATK2` smallint(6) unsigned NOT NULL default '0',
  `DEF` smallint(6) unsigned NOT NULL default '0',
  `MDEF` smallint(6) unsigned NOT NULL default '0',
  `STR` smallint(6) unsigned NOT NULL default '0',
  `AGI` smallint(6) unsigned NOT NULL default '0',
  `VIT` smallint(6) unsigned NOT NULL default '0',
  `INT` smallint(6) unsigned NOT NULL default '0',
  `DEX` smallint(6) unsigned NOT NULL default '0',
  `LUK` smallint(6) unsigned NOT NULL default '0',
  `Range2` tinyint(4) unsigned NOT NULL default '0',
  `Range3` tinyint(4) unsigned NOT NULL default '0',
  `Scale` tinyint(4) unsigned NOT NULL default '0',
  `Race` tinyint(4) unsigned NOT NULL default '0',
  `Element` tinyint(4) unsigned NOT NULL default '0',
  `Mode` smallint(6) unsigned NOT NULL default '0',
  `Speed` smallint(6) unsigned NOT NULL default '0',
  `aDelay` smallint(6) unsigned NOT NULL default '0',
  `aMotion` smallint(6) unsigned NOT NULL default '0',
  `dMotion` smallint(6) unsigned NOT NULL default '0',
  `MEXP` mediumint(9) unsigned NOT NULL default '0',
  `MVP1id` smallint(9) unsigned NOT NULL default '0',
  `MVP1per` smallint(9) unsigned NOT NULL default '0',
  `MVP2id` smallint(9) unsigned NOT NULL default '0',
  `MVP2per` smallint(9) unsigned NOT NULL default '0',
  `MVP3id` smallint(9) unsigned NOT NULL default '0',
  `MVP3per` smallint(9) unsigned NOT NULL default '0',
  `Drop1id` smallint(9) unsigned NOT NULL default '0',
  `Drop1per` smallint(9) unsigned NOT NULL default '0',
  `Drop2id` smallint(9) unsigned NOT NULL default '0',
  `Drop2per` smallint(9) unsigned NOT NULL default '0',
  `Drop3id` smallint(9) unsigned NOT NULL default '0',
  `Drop3per` smallint(9) unsigned NOT NULL default '0',
  `Drop4id` smallint(9) unsigned NOT NULL default '0',
  `Drop4per` smallint(9) unsigned NOT NULL default '0',
  `Drop5id` smallint(9) unsigned NOT NULL default '0',
  `Drop5per` smallint(9) unsigned NOT NULL default '0',
  `Drop6id` smallint(9) unsigned NOT NULL default '0',
  `Drop6per` smallint(9) unsigned NOT NULL default '0',
  `Drop7id` smallint(9) unsigned NOT NULL default '0',
  `Drop7per` smallint(9) unsigned NOT NULL default '0',
  `Drop8id` smallint(9) unsigned NOT NULL default '0',
  `Drop8per` smallint(9) unsigned NOT NULL default '0',
  `Drop9id` smallint(9) unsigned NOT NULL default '0',
  `Drop9per` smallint(9) unsigned NOT NULL default '0',
  `DropCardid` smallint(9) unsigned NOT NULL default '0',
  `DropCardper` smallint(9) unsigned NOT NULL default '0',
  PRIMARY KEY  (`ID`)
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
		@champ = split (",",$ligne);
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
print("\n");


sub printField {
	my ($str, $suffix, $idCol) = @_;
	# Remove first { and last }
	if ($str =~ /{.*}/)
	{
		$str = substr($&,1,-1);
	}
	# Remove comment at end of line
	if ($str =~ /[^\/]*\/\//)
	{
		$str = substr($&,0,-2);
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
