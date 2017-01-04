#!/usr/bin/perl
# Documentation Checker
# by trojal (modified by lighta)

use strict;
use File::Basename;
use Getopt::Long;

my $sHelp	= 0;
my $sAtcf = "../doc/atcommands.txt";
my $sSctf = "../doc/script_commands.txt";
my $sLeftOverChk      = 0;
my $sCmd = "chk";
my $sValidCmd = "ls|chk";
my $sTarget	= "All";
my $sValidTarget = "All|Script|Atc";
my $sInc_atcf = "../doc/atcommands2.txt";
my $sInc_scrtf = "../doc/script_commands2.txt";

my($filename, $dir, $suffix) = fileparse($0);
chdir $dir; #put ourself like was called in tool folder
GetArgs();
Main($sCmd,$sTarget);

sub GetArgs {
    GetOptions(
    'cmd=s' => \$sCmd,	 # which command to run
    'atcf=s' => \$sAtcf, #atcommand doc file
    'scriptf=s' => \$sSctf, #script doc file
    'inc_atcf=s' => \$sInc_atcf, #include script doc file (for customs doc)
    'inc_scrtf=s' => \$sInc_scrtf, #include script doc file (for customs doc)
    'target=s'	=> \$sTarget,	 #Target (wich files to run-cmd into)
    'leftover=i' => \$sLeftOverChk, #should we chk if all doc are linked to a src ?
    'help!' => \$sHelp,
    ) or $sHelp=1; #display help if invalid option	
	
    if( $sHelp ) {
	print "Incorrect option specified, available options are:\n"
	    ."\t --atcf filename => file (specify atcommand doc to use)\n"
	    ."\t --inc_atcf filename => include file (specify atcommand doc to use)\n"
	    ."\t --scriptf filename => file (specify script doc to use)\n"
	    ."\t --inc_scrtf filename => include file (specify script doc to use)\n"
	    ."\t --leftover=0|1 => should we run reverse chk for leftover in documentation ?\n"
	    ."\t --target => target (specify wich check to run [$sValidTarget])\n"
	    ."\t --cmd => cmd (specify wich command to run [(default)$sValidCmd])\n";
	exit;
    }
    unless($sTarget =~ /$sValidTarget/i){
    	print "Incorrect target specified. Available targets:\n"
	    ."\t --target => target (specify wich check to run [(default)$sValidTarget])\n";
		exit;
    }
    unless($sCmd =~ /$sValidCmd/i){
    	print "Incorrect command specified. Available commands:\n"
	    ."\t --cmd => cmd (specify wich command to run [(default)$sValidCmd])\n";
		exit;
    }
}


sub Main { my ($sCmd,$sTarget) = @_;
	if($sTarget=~/both|all/i){ #both is keep as backard compatibility here cf check-doc.sh
		$sTarget = "script|atc";
	}
	if($sTarget=~/script/i){ #find which script commands are missing from doc/script_commands.txt
		my $raSct_cmd = Script_GetCmd();
		if($sCmd =~ /ls/i) {
			print "The list of script-commands found are = \n[ @$raSct_cmd ] \n\n";
		}
		if($sCmd =~ /chk/i) { Script_Chk($raSct_cmd); }
	}
	if($sTarget=~/atc/i){ #find which atcommands are missing from doc/atcommands.txt
		my $raAct_cmd = Atc_GetCmd();
		if($sCmd =~ /ls/i) {
			print "The list of atcommands found are = \n[ @$raAct_cmd ] \n\n";
		}
		if($sCmd =~ /chk/i) { Atc_Chk($raAct_cmd); }
	}
}

sub Chk { my($raA,$raB) = @_;
	my @aMissing = ();
	foreach my $sA (@$raA){
		my $sFound = 0;
		foreach my $sB (@$raB){
			$sFound=1 if($sA eq $sB);
		}
		unless($sFound){
			push(@aMissing,$sA);
		}
	}
	return \@aMissing;
}

sub Script_GetCmd {
	my @aSct_src = ("../src/map/script.c","../src/custom/script_def.inc");
	my @aDef_sct = ();
	foreach my $sSct_srcf (@aSct_src){
		unless(open FILE_SRC, "<$sSct_srcf") { 
			print "Couldn't open file '$sSct_srcf'.\n";
			next;
		}
		while(<FILE_SRC>){
			next if($_ =~ /^#/); #ignoe include, define or macro
			if($_ =~ /BUILDIN_DEF|BUILDIN_DEF2/){
				$_ =~ s/\s+$//; #Remove trailing spaces.
				$_ =~ s/^\s+//; #Remove leading spaces.
				if($_ =~ /^BUILDIN_DEF2/){
					my @line = split('"',$_);
					push(@aDef_sct,$line[1]);
				}
				elsif($_ =~ /^BUILDIN_DEF/){
					my @line = split(',',$_);
					@line = split('\(',$line[0]);
					push(@aDef_sct,$line[1]);
				}
			}
		}
		close FILE_SRC;
	}
	return \@aDef_sct;
}

sub Script_Chk { my ($raDef_sct) = @_;
	my @aSct_docf = ($sSctf,$sInc_scrtf);
	my @aDoc_sct = ();
	my $raMiss_sct;

	foreach my $sSct_docf (@aSct_docf){
		unless(open FILE_DOC, "$sSct_docf"){
			print "Couldn't open file '$sSct_docf'.\n";
			next;
		}
		while(<FILE_DOC>){
			next if($_ =~ /^\*\*|^\*\s|^\s+/); #discard **, * foo, foo
			next if(/^\s+/);
			if($_ =~ /^\*/){
				my @line = split(' ',$_);
				@line = split('\(',$line[0]);
				@line = split('\<',$line[0]);
				$line[0] =~ s/\(|\{|\*|\r|\s|\;|\)|\"|\,//g; #todo please harmonize command definition for easier parse
				
				next if($line[0] eq "Name" || $line[0] eq "" || $line[0] eq "function" 
				|| $line[0] eq "if" || $line[0] eq "while" || $line[0] eq "do"  || $line[0] eq "for" ); #exception list
				
				push(@aDoc_sct,$line[0]);
			}
		}
		close FILE_DOC;
	}
	
	$raMiss_sct = Chk($raDef_sct,\@aDoc_sct); #check missing documentation
	if(scalar(@$raMiss_sct)){
		print "Missing script documentation for function :{\n";
		foreach(@$raMiss_sct){
			print "\t$_ \n";
		}
		print "}\n\n";
	}
	else { print "All script commands in src are documented, no issues found.\n"; }
	
	if($sLeftOverChk){
		my $raLeftover_sct = Chk(\@aDoc_sct,$raDef_sct); #we just inverse the chk for leftover
		if(scalar(@$raLeftover_sct)){
			print "These script commands were found in doc with no associated source:{\n";
			foreach(@$raLeftover_sct){
				print "\t$_ \n";
			}
			print "}\n\n";
		}
		else { print "All script commands in documentation match a source BUILDIN, no leftovers found.\n"; }
	}
}

sub Atc_GetCmd {
	my @aAct_src = ("../src/map/atcommand.c","../src/custom/atcommand_def.inc");
	my @aDef_act = ();
	foreach my $sAct_srcf (@aAct_src){
		unless(open FILE_SRC, "<$sAct_srcf"){
			print "Couldn't open file '$sAct_srcf'.\n";
			next;
		}
		while(<FILE_SRC>){
			next if($_ =~ /^#/); #ignore include, define or macro
			if($_ =~ /ACMD_DEF|ACMD_DEF2|ACMD_DEFR|ACMD_DEF2R/){
				$_ =~ s/\s+$//; #Remove trailing spaces.
				$_ =~ s/^\s+//; #Remove leading spaces.
				
				if($_ =~ /^ACMD_DEF2|^ACMD_DEF2R/){
					my @line = split('"',$_);
					push(@aDef_act,$line[1]);
				}
				elsif($_ =~ /^ACMD_DEF|^ACMD_DEFR/){
					my @line = split(',',$_);
					@line = split('\(',$line[0]);
					$line[1] =~ s/\)//; #Remove closing brace
					push(@aDef_act,$line[1]);
				}
			}
		}
		close FILE_SRC;
	}
	return \@aDef_act;
}

sub Atc_Chk {  my ($raDef_act) = @_;
	my @aAct_docf = ($sAtcf,$sInc_atcf);
	my @aDoc_act = ();
	my $raMiss_act;
	
	foreach my $sAct_docf (@aAct_docf){
		unless(open FILE_DOC, "$sAct_docf"){
			print "Couldn't open file '$sAct_docf'.\n";
			next;
		}
		while(<FILE_DOC>){
			next if($_ =~ /^\*\*|^\*\s|^\s+/); #discard **, * foo, foo
			next if(/^\s+/);
			if($_ =~ /^\@/){
				my @line = split(' ',$_);
				@line = split('\(',$line[0]);
				@line = split('\<',$line[0]);
				$line[0] =~ s/\(|\{|\@|\r|\s|\;|\)|\"|\,//g; #todo please harmonize command definition for easier parse
				push(@aDoc_act,$line[0]);
			}
		}
		close FILE_DOC;
	}
	
	$raMiss_act = Chk($raDef_act,\@aDoc_act); #check missing documentation
	if(scalar(@$raMiss_act)){
		print "Missing atcommand documentation for function :{\n";
		foreach(@$raMiss_act){
			print "\t$_ \n";
		}
		print "}\n\n";
	}
	else { print "All atcommands in src are documented, no issues found.\n"; }
	
	if($sLeftOverChk){
		my $raLeftover_sct = Chk(\@aDoc_act,$raDef_act); #we just inverse the chk for leftover
		if(scalar(@$raLeftover_sct)){
			print "These atcommands were found in doc with no associated source: {\n";
			foreach(@$raLeftover_sct){
				print "\t$_ \n";
			}
			print "}\n\n";
		}
		else { print "All atcommands in documentation match a source ATCMD, no leftovers found.\n"; }
	}
}
