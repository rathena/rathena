#!/usr/bin/perl
# rAthena Monster Mode Converter
# Upgrades monster's mode to the new structure.
#

use strict;
use warnings;
use Getopt::Long;
use File::Basename;
use Scalar::Util qw(looks_like_number);

my $sFileins;
my @aFilein = ("../db/re/mob_db.txt","../db/pre-re/mob_db.txt","../db/mob_db2.txt");
my $sFileouts;
my @aFileout = ("../db/re/mob_db.txt","../db/pre-re/mob_db.txt","../db/mob_db2.txt");
my $sHelp = 0;

Main();

sub convertmode {
	my $bits = hex(shift);
	my $mexp = shift;

	if ($bits&32) { # MD_BOSS
		$bits -= 32;
		$bits |= 69206016; # Doesn't include MD_DETECTOR
	}

	if ($bits&64) { # MD_PLANT
		$bits -= 64;
		$bits |= 1507328;
	}

	if ($bits&256) { # MD_DETECTOR
		$bits -= 256;
		$bits |= 33554432;
	}

	if ($bits&4194304) { # MD_NORANDOM_WALK
		$bits -= 4194304;
		$bits |= 32;
	}

	if ($bits&8388608) { # MD_NOCAST_SKILL
		$bits -= 8388608;
		$bits |= 64;
	}

	if ($mexp > 0) { # MD_MVP
		$bits |= 524288;
	}

	return $bits;
}

sub GetArgs {
	GetOptions(
	'i=s' => \$sFileins, #Output file name.
	'o=s' => \$sFileouts, #Input file name.
	'help!' => \$sHelp,
	) or $sHelp=1; #Display help if invalid options are supplied.

	if( $sHelp ) {
		print "Incorrect option specified. Available options:\n"
			."\t --o=filename => Output file name. (file must be separate by coma if multiple) \n"
			."\t --i=filenames => Input files name. (file must be separate by coma if multiple) \n";
		exit;
	}
	if($sFileins){
		chomp($sFileins);
		@aFilein = split(",",$sFileins);
	}
	if($sFileouts){
		chomp($sFileouts);
		@aFileout = split(",",$sFileouts);
	}
	unless(scalar(@aFileout)==scalar(@aFilein)){
		print "ERROR: Number of input files doesn't match number of output files. You must specify an output for each input:\n"
			."afilein = [ @aFilein ] \n"
			."afileout = [ @aFileout ] \n";
		exit;
	}
}

sub Main {
	my $sI=0;
	my($filename, $dir, $suffix) = fileparse($0);
	chdir $dir; #put ourself like was called in tool folder
	GetArgs();
	print "Running rAthena's Monster Mode Converter...\n";
	print "Files to be converted: '@aFilein' into '@aFileout'.\n";
	foreach my $sFile (@aFilein){
		my $sReplace=0; #should we replace file when finished
		my $sFileouttmp=$aFileout[$sI];
		if($sFile eq $sFileouttmp){
			$sReplace=1;
			$sFileouttmp = $sFileouttmp.".out";
			print "Asking to replace file tmp fileout= $sFileouttmp \n";
		}
		unless(open FHIN,"$sFile"){
			print "ERROR: Can't read or locate $sFile.\n";
			next;
		}
		unless(open FHOUT,">$sFileouttmp"){
			print "ERROR: Can't write or locate $aFileout[$sI].\n";
			next;
		}
		$sI++;
		while (<FHIN>){
			if( $_ =~ /^\s*$/) {  #ignore empty line
				print FHOUT $_;
				next;
			}
			my @champ = split(",",$_);
			my $sDoconvertion=0; #should this comment be converted
			if( $_ =~ /^\/\// ) { # // line
				if(scalar(@champ)>0){
					$champ[0] =~ s!\/\/!!g;
					$sDoconvertion=looks_like_number($champ[0]);
					$champ[0] = "//".$champ[0]; #recomment it
				}
				if($sDoconvertion==0) {
					print FHOUT $_;
					next;
				}
			}
			if(scalar(@champ>0)){
				if($champ[25] =~ /^0[xX]/) {
					$champ[25] = sprintf("0x%X", convertmode($champ[25], $champ[30]));
				} else {
					$champ[25] = sprintf("0x%X", convertmode(sprintf("0x%X", $champ[25]), $champ[30]));
				}
				my $newline = join(",",@champ);
				print FHOUT $newline;
			}
			else { print FHOUT $_; }
		}
		close FHOUT;
		close FHIN;
		if($sReplace){
			unlink $sFile;
			rename $sFileouttmp, $sFile;
		}
	}
}
