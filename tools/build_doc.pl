#!/usr/bin/perl
# building documentation using doxygen and updation versionning number
use strict;
use File::Basename;

my $sDocFile = "doxyconf";
my $outputdir = "doc/doxygen";
my @line = ();
my $repoversion;
my $gitversion;
my $doxyversion;
my $chked;

my($filename, $dir, $suffix) = fileparse($0);
chdir "../$dir"; #put ourself like was called in tools

#checking for doxygen
open PIPE,"doxygen --version |" or die $!;
@line = grep { /\d.\d.\d/ } <PIPE>;
$doxyversion = $line[0];
print "doxyversion = [ $doxyversion ]\n";
if($doxyversion eq ""){
	die "Please install doxygen to proceed";
}
close PIPE;

#cheking for git cli
open PIPE,"git --version |" or die $!;
@line = grep { /\d.\d.\d.\d/ } <PIPE>;
$gitversion = $line[0];
$gitversion =~ s/[^\d.\d.\d.\d]//g;
print "doxyversion = [ $gitversion ]\n";
if($gitversion eq ""){
	die "Please install git to proceed";
}
close PIPE;

open PIPE,"git rev-parse --short HEAD |" or die $!;
@line = grep { /\w/ } <PIPE>;
$repoversion = $line[0];
print "Git hash is : $repoversion";
close PIPE;

unless(-r "$outputdir"){
	mkdir "$outputdir" or die "Can't create output directory for documentation (outdir=$outputdir)\n";
}

die "$sDocFile doesn't seem to exist or coudldn't be read" unless(-r "$sDocFile");
print "Updating doxygen file version (doxyconf=$sDocFile)\n";
open FHIN,"$sDocFile" || die "couldn't openfile/create $sDocFile \n";
open FHOUT,">doxyconf.tmp" || die "couldn't openfile/create doxyconf.tmp \n";
while(<FHIN>){ 
	if(($chked&1)==0 && $_ =~ /^PROJECT_NUMBER/) {
		@line = split(" ",$_);
		print FHOUT "PROJECT_NUMBER  = $repoversion";
		print "Updated project number\n";
		$chked &=1;
	}
	elsif(($chked&2)==0 && $_ =~ /^OUTPUT_DIRECTORY/){
		print FHOUT "OUTPUT_DIRECTORY  = $outputdir";
		print "Updated output dir\n";
		$chked &=2;
	}
	else { print FHOUT $_; }
}
close FHIN;
close FHOUT;
unlink $sDocFile;
rename "doxyconf.tmp", $sDocFile;

print "Building doc\n";
system("doxygen doxyconf");
