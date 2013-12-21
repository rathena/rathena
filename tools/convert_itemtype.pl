#!/usr/bin/perl
use strict;
use warnings;
use Getopt::Long;
use File::Basename;
use Scalar::Util qw(looks_like_number);

my $sFileins;
my @aFilein = ("../db/re/item_db.txt","../db/pre-re/item_db.txt","../db/item_db2.txt");
my $sFileouts;
my @aFileout = ("../db/re/item_db.txt","../db/pre-re/item_db.txt","../db/item_db2.txt");
my $sHelp = 0;


Main();

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
		print "ERROR: number of filein doesn't match number of fileout, you must speficy each fileout for each filein:\n"
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
	print "Welcome to rA itemtype converter\n";
	print "Were going to convert those files : @aFilein into @aFileout \n";
	foreach my $sFile (@aFilein){
		my $sReplace=0; #should we replace file when finished
		my $sFileouttmp=$aFileout[$sI];
		if($sFile eq $sFileouttmp){
			$sReplace=1;
			$sFileouttmp = $sFileouttmp.".out";
			print "Asking to replace file tmp fileout= $sFileouttmp \n";
		}
		unless(open FHIN,"$sFile"){
			print "Error, can't read or locate $sFile.\n";
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
				if(scalar(@champ)>3){
					$champ[0] =~ s!\/\/!!g;
					$sDoconvertion=looks_like_number($champ[0]);
					$champ[0] = "//".$champ[0]; #recomment it
				}
				if($sDoconvertion==0) {
					print FHOUT $_;
					next; 
				}
			}
			if(scalar(@champ>3)){
				if($champ[3] == 4) { $champ[3]=5; }
				elsif($champ[3] == 5) { $champ[3]=4; }
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
