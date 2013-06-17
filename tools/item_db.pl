#!/usr/bin/perl

# To run this file:
#   item_db.pl --i=../db/pre-re/item_db.txt --o=../sql-files/item_db.sql --t=pre
#   item_db.pl --i=../db/re/item_db.txt --o=../sql-files/item_db_re.sql --t=re
# For a list of options:
#   item_db.pl --help

use strict;
use Getopt::Long;

my $sFilein = "";
my $sFileout = "";
my $sTarget = "";
my $sHelp = 0;
my $stable = "";

my $db;
my $nb_columns;
my @str_col = ();
my $line_format;
my $create_table;
my @defaults = ();

Main();

sub GetArgs {
	GetOptions(
	'i=s' => \$sFilein, #output file
	'o=s' => \$sFileout, #input file
	't=s'	=> \$sTarget, # re/pre-re
	'table=s' => \$stable,
	'help!' => \$sHelp,
	) or $sHelp=1; #Display help if invalid options are supplied.
	my $sValidTarget = "Re|Pre";

	if( $sHelp ) {
		print "Incorrect option specified. Available options:\n"
			."\t --o=filename => output filename \n"
			."\t --i=filename => intput filename \n"
			."\t --table=tablename => tablename to create \n"
			."\t --t=type => specify target type ([$sValidTarget]) \n";
		exit;
	}
	unless($sFilein or $sFileout){
		print "ERROR: Filename_in and Filename_out are required to continue.\n";
		exit;
	}
	unless($sTarget =~ /$sValidTarget/i){
		print "ERROR: Incorrect target specified. Available targets:\n"
			."\t --target => target (specify which setup to run [$sValidTarget])\n";
		exit;
	}
}

sub Main {
	GetArgs();
	BuildDataForType($sTarget);
	ConvertFile($sFilein,$sFileout);
	print "Conversion ended.\n";
}

sub ConvertFile { my($sFilein,$sFileout)=@_;
	my $sFHout;
	print "Starting ConvertFile with: \n\t filein=$sFilein \n\t fileout=$sFileout \n";
	open FHIN,"$sFilein" or die "ERROR: Can't read or locate $sFilein.\n";
	open $sFHout,">$sFileout" or die "ERROR: Can't write $sFileout.\n";
	
	printf $sFHout ("%s\n",$create_table);
	while(my $ligne=<FHIN>)
	{
		if ($ligne =~ /[^\r\n]+/)
		{
			$ligne = $&;
			if ($ligne =~ /^\/\//)
			{
				printf $sFHout ("#");
				$ligne = substr($ligne, 2);
			}
			my @champ = ();
			if ($ligne =~ $line_format) {
				@champ = ($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13,$14,$15,$16,$17,$18,$19,$20,$21,$22);
			} 
			if ($#champ != $nb_columns - 1)
			{
				# Can't parse, it's a real comment
				printf $sFHout ("%s\n", $ligne);
			} else {
				printf $sFHout ("REPLACE INTO `%s` VALUES (", $db);
				for (my $i=0; $i<$#champ; $i++)
				{
					printField($sFHout,$champ[$i],",",$i);
				}
				printField($sFHout,$champ[$#champ],");\n",$#champ);
			}
		}
	}
	print $sFHout "\n";
}

sub printField { my ($sFHout,$str, $suffix, $idCol) = @_;
	# Remove first { and last }
	if ($str =~ /{.*}/)
	{
		$str = substr($&,1,-1);
	}
	# If nothing, put NULL
	if ($str eq "") {
		my $sDef;
		if(scalar(@defaults)) { $sDef = $defaults[$idCol]; } #Use default in array.
		else { $sDef = "NULL" unless($sDef); } #Let SQL handle the default.
		print $sFHout "$sDef$suffix";
	} else {
		my $flag = 0;
		# Search if it's a string column?
		foreach my $col (@str_col)
		{
			if ($col == $idCol)
			{
				$flag |= 1;
				last;
			}
		}
		$flag |= 2 if($idCol >= $nb_columns-3);

		if ($flag & 3)
		{
			# String column, so escape , remove trailing and add ''
			my $string;
			$string = escape($str,"'","\\'") if($flag & 1) ;
			$string =~ s/\s+$//; #Remove trailing spaces.
			$string =~ s/^\s+//; #Remove leading spaces.
			$string = escape($string,'\\\"','\\\\\"') if($flag & 2) ;
			printf $sFHout ("'%s'%s", $string, $suffix);
		} else {
			# Not a string column.
			printf $sFHout ("%s%s", $str,$suffix);
		}
	}
}

sub escape { my ($str,$sregex,$sreplace) = @_;
	my @str_splitted = split($sregex, $str);
	my $result = "";
	for (my $i=0; $i<=$#str_splitted; $i++)
	{
		if ($i == 0) {
			$result = @str_splitted[0];
		} else {
			$result = $result.$sreplace.@str_splitted[$i];
		}
	}
	return $result
}

sub BuildDataForType{ my($sType) = @_;
	print "Starting BuildDataForType with: \n\t type=$sTarget \n";
	
	if($sType =~ /Pre/i){
		$db = $stable;
		$db = "item_db" unless($db);
		$nb_columns = 22;
		@str_col = (1,2,19,20,21);
		$line_format = "([^\,]*),"x($nb_columns-3)."(\{.*\}),"x(2)."(\{.*\})"; #Last 3 columns are scripts.
		$create_table = "
#
# Table structure for table `$db`
#
DROP TABLE IF EXISTS `$db`;
CREATE TABLE `$db` (
  `id` smallint(5) unsigned NOT NULL default '0',
  `name_english` varchar(50) NOT NULL default '',
  `name_japanese` varchar(50) NOT NULL default '',
  `type` tinyint(2) unsigned NOT NULL default '0',
  `price_buy` mediumint(10) unsigned default NULL,
  `price_sell` mediumint(10) unsigned default NULL,
  `weight` smallint(5) unsigned NOT NULL default '0',
  `attack` smallint(3) unsigned default NULL,
  `defence` smallint(5) NULL default NULL,
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
	#NOTE: These do not match the table struct defaults.
	@defaults = ('0','\'\'','\'\'','0','NULL','NULL','NULL','NULL','NULL','NULL','NULL','NULL','NULL','NULL','NULL','NULL','NULL','NULL','NULL','NULL','NULL','NULL');
	}
	elsif($sType =~ /Re/i){
		$db = $stable;
		$db = "item_db_re" unless($db);
		$nb_columns = 22;
		@str_col = (1,2,7,16,19,20,21);
		$line_format = "([^\,]*),"x($nb_columns-3)."(\{.*\}),"x(2)."(\{.*\})"; #Last 3 columns are scripts.
		$create_table = "
#
# Table structure for table `$db`
#

DROP TABLE IF EXISTS `$db`;
CREATE TABLE `$db` (
 `id` smallint(5) unsigned NOT NULL default '0',
 `name_english` varchar(50) NOT NULL default '',
 `name_japanese` varchar(50) NOT NULL default '',
 `type` tinyint(2) unsigned NOT NULL default '0',
 `price_buy` mediumint(10) unsigned default NULL,
 `price_sell` mediumint(10) unsigned default NULL,
 `weight` smallint(5) unsigned NOT NULL default '0',
 `atk:matk` varchar(11) default '',
 `defence` smallint(5) NULL default NULL,
 `range` tinyint(2) unsigned default NULL,
 `slots` tinyint(2) unsigned default NULL,
 `equip_jobs` int(12) unsigned default NULL,
 `equip_upper` tinyint(8) unsigned default NULL,
 `equip_genders` tinyint(2) unsigned default NULL,
 `equip_locations` smallint(4) unsigned default NULL,
 `weapon_level` tinyint(2) unsigned default NULL,
 `equip_level` varchar(10) default '',
 `refineable` tinyint(1) unsigned default NULL,
 `view` smallint(3) unsigned default NULL,
 `script` text,
 `equip_script` text,
 `unequip_script` text,
 PRIMARY KEY (`id`)
) ENGINE=MyISAM;
";
	#NOTE: These do not match the table struct defaults.
	@defaults = ('0','\'\'','\'\'','0','NULL','NULL','NULL','NULL','NULL','NULL','NULL','NULL','NULL','NULL','NULL','NULL','NULL','NULL','NULL','NULL','NULL','NULL');
	}
}