#!/usr/bin/perl
# Documentation Builder
# Builds documentation using doxygen and updates version number.
use strict;
use File::Basename;
use Getopt::Long;

#prgm option
my $sHelp	= 0;
my $sDocFile = "doxyconf";
my $outputdir = "doc/doxygen";
my $sForceBuild = 0;
my $sIgnChk = "None";
my $sValidTarget = "None|All|Doxygen|Git";
my $sNoUpd = 0;

GetArgs();
Main();

sub GetArgs {
    GetOptions(
    'doxyconf=s' => \$sDocFile, #specify the doxygen configuration file
    'outdir=s' => \$outputdir, #specify in which folder to build the documentation
    'ignorechk=s'	=> \$sIgnChk,	 #Target (which setup to run)
    'forcebuild=i' => \$sForceBuild, #should we chk if all doc are linked to a src ?
    'noupd=i' => \$sNoUpd, #prevent altering doxygen conf
    'help!' => \$sHelp,
    ) or $sHelp=1; #display help if invalid option	
	
    if( $sHelp ) {
	print "Incorect option specified, available option are:\n"
	    ."\t --doxyconf filename => specify which doxygen configuration to use\n"
	    ."\t --outdir path => specify in which path to build doxygen documentation\n"
	    ."\t --forcebuild=0|1 => should we force building of documentation even if same git detected ?\n"
	    ."\t --noupd=0|1 => should we skip producing a new doxyconf for version ?\n"
	    ."\t --ignorechk => target (specify which check to ignore [$sValidTarget])\n";
	exit;
    }
    if($sIgnChk && !($sIgnChk =~ /$sValidTarget/i)){
    	print "Incorect ignorechk target specified. Available targets:\n"
	    ."\t --ignorechk => target (specify which check to ignore [(default)$sValidTarget])\n";
		exit;
    }
}

sub Main {
	my $repoversion;
	my $sSkipBuild=0;
	
	my($filename, $dir, $suffix) = fileparse($0);
	chdir $dir; #put ourself like was called in main
	chdir "..";

	DoxygenChk() unless($sIgnChk =~ /Doxygen|All/i);
	GitChk() unless($sIgnChk =~ /Git|All/i);
	$repoversion = GetRepoVersion();
	
	unless(-r "$outputdir"){
		mkdir "$outputdir" or die "Can't create output directory for documentation (outdir=$outputdir)\n";
	}
	
	$sSkipBuild = UpdDoxyConf($repoversion);
	
	if($sForceBuild || $sSkipBuild==0){
		print "Building doc\n";
		system("doxygen doxyconf");
	}
}

sub DoxygenChk {
	my $doxyversion;
	my @line = ();
	#checking for doxygen
	open PIPE,"doxygen --version |" or die $!;
	@line = grep { /\d.\d.\d/ } <PIPE>;
	$doxyversion = $line[0];
	chomp($doxyversion); #remove newline
	print "doxyversion = [ $doxyversion ]\n";
	if($doxyversion eq ""){
		die "Please install doxygen to proceed.";
	}
	close PIPE;
	return $doxyversion;
}

sub GitChk {
	my $gitversion;
	my @line = ();
	
	#cheking for git cli
	open PIPE,"git --version |" or die $!;
	@line = grep { /\d.\d.\d/ } <PIPE>;
	$gitversion = $line[0];
	$gitversion =~ s/[^\d.\d.\d]//g;
	chomp($gitversion);
	print "git = [ $gitversion ]\n";
	if($gitversion eq ""){
		die "Please install git to proceed.";
	}
	close PIPE;
	return $gitversion;
}

sub GetRepoVersion {
	my $repoversion;
	my @line = ();
	
	open PIPE,"git rev-parse --short HEAD |" or die $!;
	@line = grep { /\w/ } <PIPE>;
	$repoversion = $line[0];
	chomp($repoversion);
	print "Git hash is : $repoversion\n";
	close PIPE;
	return $repoversion;
}

sub UpdDoxyConf { my ($repoversion) = @_;
	my $sSkipBuild=0;
	my @line = ();
	my $chked=0;
	
	die "$sDocFile doesn't seem to exist or coudldn't be read" unless(-r "$sDocFile");
	
	open FHIN,"$sDocFile" || die "couldn't openfile/create $sDocFile \n";
	open FHOUT,">doxyconf.tmp" || die "couldn't openfile/create doxyconf.tmp \n";
	while(<FHIN>){ 
		if(($chked&1)==0 && $_ =~ /^PROJECT_NUMBER/) {
			chomp($_);
			@line = split('=',$_);
			@line = split('/',$line[1]);
			my $old_version = $line[scalar(@line)-1];
			print "old_version found=$old_version, current version=$repoversion \n";
			if($old_version == $repoversion) { $sSkipBuild=1; }
			elsif($sNoUpd==0) { print "Updated project number\n"; }
			print FHOUT "PROJECT_NUMBER  = http://github.com/rathena/rathena/commit/$repoversion\n";
			$chked &=1;
		}
		elsif(($chked&2)==0 && $_ =~ /^OUTPUT_DIRECTORY/){
			print FHOUT "OUTPUT_DIRECTORY  = $outputdir\n";
			print "Updated output dir\n" if($sNoUpd==0);
			$chked &=2;
		}
		else { print FHOUT $_; }
	}
	close FHIN;
	close FHOUT;
	
	if($sNoUpd==0){
		unlink $sDocFile;
		rename "doxyconf.tmp", $sDocFile;
		print "Updating doxygen file version (doxyconf=$sDocFile)\n";
	}
	else { unlink "doxyconf.tmp"; }
	
	return $sSkipBuild;
}


