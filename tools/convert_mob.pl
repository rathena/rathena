#!/usr/bin/perl

# Mob Database:
#     --i=../db/pre-re/mob_db.txt --o=../db/pre-re/mob_db_new.txt --o2=../db/pre-re/mob_drop_new.txt
#     --i=../db/re/mob_db.txt --o=../db/re/mob_db_new.txt --o2=../db/re/mob_drop_new.txt


# List of options:
#   convert_mob.pl --help

use strict;
use warnings;
use Getopt::Long;
use File::Basename;

my $sFilein = "";
my $sFileoutinfo = "";
my $sFileoutdrop = "";
my $sHelp = 0;

my @str_col = (); #Use basic escape.
my @str_col2 = (); #Use second escape (currently for scripts).

my $nb_columns;
my $nb_columbs_info;
my $create_csv_info;
my $create_csv_drop;
my @defaults = ();

Main();

sub GetArgs {
	GetOptions(
	'i=s' => \$sFilein, #Input file name.
	'o=s' => \$sFileoutinfo, #Mob Info Output file name.
	'o2:s' => \$sFileoutdrop, #Mob Drop Output file name.
	'help!' => \$sHelp,
	) or $sHelp=1; #Display help if invalid options are supplied.

	if( $sHelp ) {
		print "Incorrect option specified. Available options:\n"
			."\t --o=filename => Mob Info Output file name. \n"
			."\t --o2=filename => Mob Drop Output file name. \n"
			."\t --i=filename => Input file name. \n";
		exit;
	}
	unless($sFilein or $sFileoutinfo or $sFileoutdrop){
		print "ERROR: Filename_in, Filename_out, Filename_out2 are required to continue.\n";
		exit;
	}
}

sub Main {
	GetArgs();
	my($filename, $dir, $suffix) = fileparse($0);
	chdir $dir; #put ourself like was called in tool folder
	BuildData();
	ConvertFile($sFilein,$sFileoutinfo,$sFileoutdrop);
	print "Conversion ended.\n";
}

sub ConvertFile { my($sFilein,$sFileoutinfo,$sFileoutdrop)=@_;
	my $sFHoutinfo;
	my $sFHoutdrop;
	print "Starting ConvertFile with: \n\t filein=$sFilein \n\t fileoutinfo=$sFileoutinfo \n\t fileoutdrop=$sFileoutdrop \n";
	open FHIN,"$sFilein" or die "ERROR: Can't read or locate $sFilein.\n";
	open $sFHoutinfo,">$sFileoutinfo" or die "ERROR: Can't write $sFileoutinfo.\n";
	open $sFHoutdrop,">$sFileoutdrop" or die "ERROR: Can't write $sFileoutdrop.\n";
	
	#printf $sFHoutinfo ("%s\n",$create_csv_info);
	printf $sFHoutdrop ("%s\n",$create_csv_drop);
	while(my $ligne=<FHIN>) {
		my $sWasCom = 0;
		if ($ligne =~ /^\s*$/ ) { #if white space line, print a newline, continue
				print $sFHoutinfo "\n";
				print $sFHoutdrop "\n";
				next;
		}
		if ($ligne =~ /[^\r\n]+/) { #if the line has a lineending, continue
			$ligne = $&;
			if ($ligne =~ /^\/\//) { #check for comment
				printf $sFHoutinfo ("//");
				$ligne = substr($ligne, 2);
				$sWasCom = 1;
			}
			my @champ = ();
			@champ = split(",",$ligne); #split the string to an array
			
			if ($#champ != $nb_columns - 1) { #Can't parse, it's a real comment.
				printf $sFHoutinfo ("%s\n", $ligne);
				#print "Real String!\n" . $ligne . "\n";
			} else {
				my $string;
				for (my $i=0; $i<$nb_columbs_info; $i++) {
					$string .= $champ[$i] . ",";
				}
				$string = substr($string, 0, -1);
				printf $sFHoutinfo ("%s\n",$string);

			}
		}
	}
	print $sFHoutinfo "\n";
}

sub BuildData{ 
	print "Starting BuildData\n";
	$nb_columns = 57;
	$nb_columbs_info = 31;
	$create_csv_info =
"// Monster Info Database
//
// Structure of Database :
// ID,Sprite_Name,kROName,iROName,LV,HP,SP,EXP,JEXP,Range1,ATK1,ATK2,DEF,MDEF,STR,AGI,VIT,INT,DEX,LUK,Range2,Range3,Scale,Race,Element,Mode,Speed,aDelay,aMotion,dMotion,MEXP
// Note: Keep the Sprite_Name field as it is in the game client.
";

	$create_csv_drop =
"// Monster Drop Database
//
// Structure of Database :
// MonsterId,DropIndex,isMvpDrop,ItemId,ItemPercentage,isStealProtected,isCard
// Note: DropIndex is used to overwrite item drops
";
}

