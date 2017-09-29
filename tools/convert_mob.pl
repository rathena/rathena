#!/usr/bin/perl

# Mob Database:
#     --iinfo=../db/pre-re/mob_db.txt --idrop=../db/pre-re/mob_drop.txt --oinfo=../db/pre-re/mob_db_new.txt --odrop=../db/pre-re/mob_drop_new.txt
#     --iinfo=../db/re/mob_db.txt --idrop=../db/re/mob_drop.txt --oinfo=../db/re/mob_db_new.txt --odrop=../db/re/mob_drop_new.txt


# List of options:
#   convert_mob.pl --help

use strict;
use warnings;
use Getopt::Long;
use File::Basename;
use File::Copy;

my $iInfoFile = "";
my $iDropFile = "";
my $oInfoFile = "";
my $oDropFile = "";
my $sHelp = 0;

my $nb_columns;
my $nb_columns_info;
my $nb_columns_mvp;
my $nb_columns_item;
my $nb_columns_card;
my $create_csv_info;
my $create_csv_drop;

Main();

sub GetArgs {
	GetOptions(
	'iinfo=s' => \$iInfoFile, #Mob Info Input file name.
	'idrop:s' => \$iDropFile, #Mob Drop Input file name
	'oinfo=s' => \$oInfoFile, #Mob Info Output file name.
	'odrop=s' => \$oDropFile, #Mob Drop Output file name.
	'help!' => \$sHelp,
	) or $sHelp=1; #Display help if invalid options are supplied.

	if( $sHelp ) {
		print "Incorrect option specified. Available options:\n"
			."\t --iinfo=filename => Mob Info Input file name. \n"
			."\t --idrop=filename => Mob Drop Input file name. (Optional) \n"
			."\t --oinfo=filename => Mob Info Output file name. \n"
			."\t --odrop=filename => Mob Drop Output file name. \n";
		exit;
	}
	unless($iInfoFile and $oInfoFile and $oDropFile){
		print "ERROR: iinfo, oinfo, odrop are required to continue.\n";
		exit;
	}
}

sub Main {
	GetArgs();
	my($filename, $dir, $suffix) = fileparse($0);
	chdir $dir; #put ourself like was called in tool folder
	BuildData();
	if (!$iDropFile) {
		print "No Input drop file, ignoring...";
	}
	ConvertFile($iInfoFile,$iDropFile,$oInfoFile,$oDropFile);
	print "Conversion ended.\n";
}

sub ConvertFile { my($iInfoFile,$iDropFile,$oInfoFile,$oDropFile)=@_;
	my $oDrop;
	my $oInfo;
	my @lines;
	my @drops_in;
	my @drops = ();
	print "Starting ConvertFile with: \n"
		."\t fileininfo=$iInfoFile \n"
		."\t fileindrop=$iDropFile \n"
		."\t fileoutinfo=$oInfoFile \n"
		."\t fileoutdrop=$oDropFile \n";
	open FHIN,"$iInfoFile" or die "ERROR: Can't read or locate $iInfoFile.\n";
	@lines = <FHIN>;
	close FHIN or die "Error: closing FHIN $!.\n";
	if ($iDropFile and open FHDROP, "cat $iDropFile | sed '/^\\/\\// d' | sort -t ',' -n -k 1,1 |") {
		@drops_in = <FHDROP>;
		foreach(@drops_in) {
			my $line = $_;
			if ($line =~ /^\s*$/) { #skip whitespace lines
				next;
			}
			if ($line =~ /[^\r\n]+/) { #if line has a line ending
				$line = $&; #strip line ending
				if ($line =~ /^\/\//) { #skip comments
					next;
				}
				my @arr = ();
				@arr = split(",", $line); #split string into an array
				if ($#arr < 2 || $#arr > 4) { #if the length is less than 3 or more than 5
					next;								#not a real line
				}
				unless ($arr[0] =~ /^\d+$/) { #if not a number
					next; 							#not a real line
				}
				push(@drops, join(',', @arr));
			}
		}
	}
	print join("\n", @drops) . "\n";
	return;
	open $oDrop,">$oDropFile" or die "ERROR: Can't write $oDropFile $!.\n";
	open $oInfo,">$oInfoFile" or die "ERROR: Can't write $oInfoFile $!.\n";

	#printf $oInfo ("%s\n",$create_csv_info);
	printf $oDrop ("%s\n",$create_csv_drop);
	foreach(@lines) {
		my $line = $_;
		my $sWasCom = 0;
		if ($line =~ /^\s*$/ ) { #if white space line, print a newline, continue
				print $oInfo "\n";
				print $oDrop "\n";
				next;
		}
		if ($line =~ /[^\r\n]+/) { #if the line has a line ending
			$line = $&; #strip the line ending
			if ($line =~ /^\/\//) { #check for comment
				printf $oInfo ("//");
				$line = substr($line, 2);
				$sWasCom = 1;
			}
			my @champ = ();
			@champ = split(",",$line); #split the string to an array
			
			if ($#champ != $nb_columns - 1) { #Can't parse, it's a real comment.
				printf $oInfo ("%s\n", $line);
				#print "Real String!\n" . $line . "\n";
			} else {
				my $string;
				my $mobid;

				#mobinfo 
				for (my $i=0; $i<$nb_columns_mvp; $i++){
					$string .= $champ[$i] . ",";
				}
				$string = substr($string, 0, -1);
				printf $oInfo ("%s\n",$string);
				printf $oDrop ("\n// %s | %s | %s\n", $champ[0], $champ[2], $champ[3]);
				
				#mob drops
				$string = "";
				$mobid = $champ[0];
				for (my $i = $nb_columns_mvp; $i < $nb_columns; $i += 2) {
					my $idx;
					my $dropType;
					my $dropflag = 0;
					$string = "";
					if ($champ[$i] eq "0") {
						next;
					}
					if ($i < $nb_columns_item) { #mvp item
						$idx = ($i - $nb_columns_mvp) / 2 + 1;
						$dropType = 1;
					} else {
						$idx = ($i - $nb_columns_item) / 2 + 1;
						$dropType = 0;
						if ($idx < 7) {
							$dropflag |= 1; #Is Stealable?
						}
						if ($idx == 10) {
							$dropflag |= 2; #Is Card?
						}
					}
					if ($sWasCom == 1) {
						$string = '//';
					}
					$string .= $mobid; #mobid
					$string .= ',' . $dropType; #DropType
					$string .= ',' . $idx; #DropIndex
					$string .= ',' . $champ[$i]; #ItemId
					$string .= ',' . $champ[$i+1]; #ItemPercentage
					unless ($dropflag == 0) {
						$string .= ',' . $dropflag;
					}
					printf $oDrop ("%s\n", $string);
				}
			}
		}
	}
	print $oInfo "\n";
	print $oDrop "\n";
}

sub BuildData{ 
	print "Starting BuildData\n";
	$nb_columns = 57;
	$nb_columns_info = 31;
	$nb_columns_mvp = 31;
	$nb_columns_item = 37;
	$nb_columns_card = 55;
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
// MonsterId,DropType,DropIndex,ItemId,ItemPercentage{,DropFlag{,randopt_groupid}}
// Note: DropType valid values: 0 - Normal Drop | 1 - MvP Drop
// Note: DropFlags are bitmask:
//     0x0 = None
//     0x1 = Is Stealable
//     0x2 = Is Card
// Note: DropIndex is used to overwrite item drops
";
}

