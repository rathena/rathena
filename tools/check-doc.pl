#!/usr/bin/perl
# checking-doc original script by trojal
# modified by lighta

use strict;
use File::Basename;
use Getopt::Long;

my $sHelp	= 0;
my $sCmd = $1;
my $sAtcf = "../doc/atcommands.txt";
my $sSctf = "../doc/script_commands.txt";
my $sLeftOverChk      = 0;
my $sTarget	= "All";
my $sValidTarget = "All|Script|Atc";

my($filename, $dir, $suffix) = fileparse($0);
chdir $dir; #put ourself like was called in tool folder
GetArgs();
Main($sTarget);

sub GetArgs {
    GetOptions(
    'atcf=s' => \$sAtcf, #atc doc file
    'scriptf=s' => \$sSctf, #script doc file
    'target=s'	=> \$sTarget,	 #Target (wich setup to run)
    'leftover=i' => \$sLeftOverChk, #should we chk if all doc are linked to a src ?
    'help!' => \$sHelp,
    ) or $sHelp=1; #display help if invalid option	
	
    if( $sHelp ) {
	print "Incorect option specified, available option are:\n"
	    ."\t --atcf filename => file (specify atcommand doc to use)\n"
	    ."\t --scriptf filename => file (specify script doc to use)\n"
	    ."\t --leftover=0|1 => should we run reverse chk for leftover in documentation ?\n"
	    ."\t --target => target (specify wich check to run [$sValidTarget])\n";
	exit;
    }
    unless($sTarget =~ /$sValidTarget/i){
    	print "Incorect target specified, available target are:\n"
	    ."\t --target => target (specify wich check to run [(default)$sValidTarget])\n";
		exit;
    }
}


sub Main { my ($sCmd) = @_;
	if($sCmd=~/both|all/i){ #both is keep as backard compatibility here cf check-doc.sh
		$sCmd = "script|atc";
	}
	if($sCmd=~/script/i){ #find which script commands are missing from doc/script_commands.txt
		Script_Chk();
	}
	if($sCmd=~/atc/i){ #find which atcommands are missing from doc/atcommands.txt
		Atc_Chk();
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

sub Script_Chk {
	my @aSct_src = ("../src/map/script.c","../src/custom/script_def.inc");
	my @aDef_sct = ();
	my @aDoc_sct = ();
	my $raMiss_sct;
	foreach my $sSct_srcf (@aSct_src){
		open FILE_SRC, "<$sSct_srcf" || die "couldn't open file $sSct_srcf \n";
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
	open FILE_DOC, "$sSctf" || die "couldn't open file $sSctf \n";
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
	$raMiss_sct = Chk(\@aDef_sct,\@aDoc_sct); #check missing documentation
	if(scalar(@$raMiss_sct)){
		print "Missing script documentation for function :{\n";
		foreach(@$raMiss_sct){
			print "\t$_ \n";
		}
		print "}\n\n";
	}
	else { print "All script command in Src are documented, no issues found\n"; }
	
	if($sLeftOverChk){
		my $raLeftover_sct = Chk(\@aDoc_sct,\@aDef_sct); #we just inverse the chk for leftover
		if(scalar(@$raLeftover_sct)){
			print "Those script command was found in doc but no source associated, leftover ? :{\n";
			foreach(@$raLeftover_sct){
				print "\t$_ \n";
			}
			print "}\n\n";
		}
		else { print "All script command in documentation match a source BUILDIN, no leftover found\n"; }
	}
}

sub Atc_Chk {
	my @aAct_src = ("../src/map/atcommand.c","../src/custom/atcommand_def.inc");
	my @aDef_act = ();
	my @aDoc_act = ();
	my $raMiss_act;
	foreach my $sAct_srcf (@aAct_src){
		open FILE_SRC, "<$sAct_srcf" || die "couldn't open file $sAct_srcf \n";
		while(<FILE_SRC>){
			next if($_ =~ /^#/); #ignoe include, define or macro
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
	open FILE_DOC, "$sAtcf" || die "couldn't open file $sAtcf \n";
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
	
	$raMiss_act = Chk(\@aDef_act,\@aDoc_act); #check missing documentation
	if(scalar(@$raMiss_act)){
		print "Missing atcommand documentation for function :{\n";
		foreach(@$raMiss_act){
			print "\t$_ \n";
		}
		print "}\n\n";
	}
	else { print "All atcommand in Src are documented, no issues found\n"; }
	
	if($sLeftOverChk){
		my $raLeftover_sct = Chk(\@aDoc_act,\@aDef_act); #we just inverse the chk for leftover
		if(scalar(@$raLeftover_sct)){
			print "Those atcommand command was found in doc but no source associated, leftover ? : {\n";
			foreach(@$raLeftover_sct){
				print "\t$_ \n";
			}
			print "}\n\n";
		}
		else { print "All atcommand in documentation match a source ATCMD, no leftover found\n"; }
	}
}
