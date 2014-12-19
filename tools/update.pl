#!/usr/bin/perl
# upgrading rA emulator, (src,npc,db..).
# perform sql db update
# perform binary recompilation

use strict;
use Getopt::Long;
use Cwd;
use Git::Repository;
use File::Copy;
use DBI;
use DBD::mysql;
use YAML::XS;
use rA_Common;

#prgm option
my $sHelp	= 0;
my $srABaseGitHttp	= 'https://github.com/rathena/rathena.git';
my $srABaseGitSSH	= 'git@github.com:rathena/rathena.git';
use constant {
	STATE_FILE => "SQL_Status.yml",
	ST_OLD => "old",
	ST_SK => "skipped",
	ST_DONE => "done",
	SQL_HOST => "SQL_host",
	SQL_PORT => "SQL_port",
	SQL_UID => "SQL_userid",
	SQL_PW => "SQL_userpass",
	SQL_MAIN_DB => "SQL_maindb",
	SQL_LOG_DB => ,"SQL_logdb"
};
my %hFileState = ();
my $sValidTarget = "All|DB|Compile|Restart|Upd|MapDB";

#those following could be edited by user
my $sAutoDB    = 0;	#by default we ask for db setting
my $sTarget = "Upd|DB|Compile|MapDB"; #default target doesn't restart
my %hDefConf = (
	SQL_HOST => "localhost",
	SQL_PORT => "3306",
	SQL_UID => "ragnarok",
	SQL_PW => "ragnarok",
	SQL_MAIN_DB => "ragnarok",
	SQL_LOG_DB => ,"ragnarok",
);

GetArgs();
Main();

sub GetArgs {
	GetOptions(
	'target=s'	=> \$sTarget,	 #Target (wich setup to run)=
	'help!' => \$sHelp,
	) or $sHelp=1; #display help if invalid option	
	
	if( $sHelp ) {
	print "Incorect option specified, available option are:\n"
	    ."\t --target => target (specify wich check to ignore [$sValidTarget])\n";
	exit;
	}
	if(!$sTarget || !($sTarget =~ /$sValidTarget/i)){
		print "Incorect target specified, available target are:\n"
			."\t --target => target (specify wich check to ignore [(default)$sValidTarget])\n 
			NB restart is compiling dependant\n";
		exit;
    }
}

sub Main {
	my $sCurdir = getcwd;
	chdir "..";
	
	UpdateSQL($sCurdir,1,\%hFileState);
	if($sTarget =~ "All|Upd") { GitUpdate($sCurdir); }
	if($sTarget =~ "All|DB") { UpdateSQL($sCurdir,0,\%hFileState); }
	if($sTarget =~ "All|Compile") { RunCompilation($sCurdir,$sTarget); }
}

sub GetSqlFileInDir { my($sDir) = @_;
	opendir (DIR, $sDir) or die $!;
	my @aFiles 
		= grep { 
		/^(?!\.)/      # not begins with a period
		&& /\.sql$/      # finish by .sql
		&& -f "$sDir/$_"   # and is a file
	} readdir(DIR);
	closedir(DIR);
	return \@aFiles;
}

sub UpdateSQL  { my($sBaseDir,$sInit,$rhFileState) = @_;
	my @aMapDBFiles = ();
	my @aCharDBFiles = (); #for now we assum they all in same DB
	my @aLoginDBFiles = ();
	my @aLogDBFiles = ();

	print "Preparing SQL folder\n" if($sInit==1);
	if(-e -r "sql-files/".STATE_FILE) {
		print "Reading file status \n";
		$rhFileState = YAML::XS::LoadFile("sql-files/".STATE_FILE);
	}
	
	if($sTarget =~ "All|MapDB") {
			chdir "sql-files";
			print "Getting Map SQL Db file \n";
			my $raFilesMap = GetSqlFileInDir("./");
			foreach my $sFile (@$raFilesMap){
				if($sInit==1){
					if(exists $$rhFileState{$sFile} && $$rhFileState{$sFile}{"status"} == ST_DONE ){
						next;
					}
					$$rhFileState{$sFile}{"status"} = ST_OLD;
					$$rhFileState{$sFile}{"lastmod"} = (stat ($sFile))[9];
				}
				elsif($$rhFileState{$sFile}{"lastmod"} != (stat ($sFile))[9] )  {
					print "The file $sFile was updated\n {\t ".$$rhFileState{$sFile}->{"lastmod"}." , ".(stat ($sFile))[9]."} \n";
					push(@aMapDBFiles,$sFile);
					$$rhFileState{$sFile}{"status"} = ST_DONE;
				}
			}
			chdir "..";
	}
	
	chdir "sql-files/upgrades";
	my $raFiles = GetSqlFileInDir("./");
	
	foreach my $sFile (@$raFiles){
		#print "Cur file = $sFile \n";
		if($sInit==1){
			if(exists $$rhFileState{$sFile} && $$rhFileState{$sFile}{"status"} == ST_DONE ){
				next;
			}
			
			if( $sFile =~ /_opt_/){
				$$rhFileState{$sFile}{"status"} = ST_SK;
			} else {
				$$rhFileState{$sFile}{"status"} = ST_OLD;
			}
			$$rhFileState{$sFile}{"lastmod"} = (stat ($sFile))[9];
		}
		else {
			if(exists $$rhFileState{$sFile}){
				my $sT = $$rhFileState{$sFile}{"status"};
				my $sLastMode = $$rhFileState{$sFile}{"lastmod"};
		#		#if it's done or skipped don't do it, if it's old but updated do it
				next if ( $sT eq ST_OLD or $sT eq ST_DONE or $sLastMode == (stat ($sFile))[9] );
			}
			
			if( $sFile =~ /_log.sql$/) { 
				print "Found log file = $sFile \n";
				push(@aLogDBFiles,$sFile);
			}
			else { 
				print "Found char file = $sFile \n";\
				push(@aCharDBFiles,$sFile);
			}
			$$rhFileState{$sFile}{"status"} = "done"; #  the query will be applied so mark it so
			
		# This part is for distributed DB, not supported yet
		# proposed nomenclature [lighta] : update_date_{opt_}(map|chr|acc|log).sql
		# (e.g : update_20141218_opt_map.sql or update_20141218_acc.sql
		
		#	if( $sFile =~ /_map.sql$/) { 
		#		print "Found log file = $sFile \n";
		#		push(@aMapDBFiles,$sFile);
		#		
		#	}
		#	elsif( $sFile =~ /_acc.sql$/) { 
		#		print "Found log file = $sFile \n";
		#		push(@aLoginDBFiles,$sFile);
		#		
		#	}
		#	elsif( $sFile =~ /_chr.sql$/) { 
		#		print "Found log file = $sFile \n";
		#		push(@aCharDBFiles,$sFile);
		#		
		#	}
		}
	}
	if($sInit==0){ #apply update
		return;
		if( scalar(@aCharDBFiles)==0 and  scalar(@aLogDBFiles)==0 
			and  scalar(@aMapDBFiles)==0 and  scalar(@aLoginDBFiles)==0
		){
			print "No SQL Update to perform\n";
		}
		else {
			print "Updating DB \n";
			my $rhUserConf;
			if($sAutoDB==0){
				$rhUserConf=GetValidateConf(\%hDefConf);
				print "To make this step auto you can edit hDefConf and set sAutoDB to 1.
					both parameter at the begining of file for the moment\n";
			}
			else {
				$rhUserConf=\%hDefConf; #we assum it's set correctly
			}
			
			CheckAndLoadSQL(\@aMapDBFiles,$rhUserConf,$$rhUserConf{SQL_MAP_DB});
			CheckAndLoadSQL(\@aCharDBFiles,$rhUserConf,$$rhUserConf{SQL_MAIN_DB});
			#CheckAndLoadSQL(\@aLoginDBFiles,$rhUserConf,$$rhUserConf{SQL_ACC_DB});
			CheckAndLoadSQL(\@aLogDBFiles,$rhUserConf,$$rhUserConf{SQL_LOG_DB});
		}
		chdir "../..";
	} else {
		chdir "../..";
		print "Saving stateFile \n";
		YAML::XS::DumpFile("sql-files/".STATE_FILE,$rhFileState);
	}

}

sub RunCompilation { my($sBaseDir,$sTarget) = @_;
	if($^O =~ "linux"){
		print "Recompiling \n";
		system('./configure && make clean server');
		if($sTarget =~ "All|Restart") {
			print "Restarting \n";
			system('./athena-start restart');
		}
	}
	else {
		print "AutoCompilation ain't supported for this OS yet (OS detected=$^O \n";
	}
}

sub GitUpdate { my($sBaseDir) = @_;
	my $sGit = Git::Repository->new(
		work_tree => "$sBaseDir/..",
	);
	
	my $sIsOrigin = CheckRemote($sGit);
	if($sIsOrigin==0){
		print "Saving current work\n";
		$sGit->run( "stash" );
		print "Fetching new content and merging\n";
		$sGit->run( "pull" );
		print "Attempt applying save work\n";
		$sGit->run( "stash" => "pop" );
	}
	else { #it's a fork
		print "Fetching upstream\n";
		$sGit->run( "fetch" => "upstream" );
		print "Switching to master branch\n";
		$sGit->run( "checkout" => "master" );
		print "Merging upstream with master\n";
		$sGit->run( "merge" => "upstream/master" );
	}
}

#Checking rA as a remote or origin
sub CheckRemote { my($sGit) = @_;
	my $sRaOrigin=0;
	my $sRaUpstream=0;
	
	print "Checking remotes\n";
	my @aRemotes = $sGit->run("remote" => "-v");	
	
	#print "My Remotes are\n";
	foreach my $sCurRem (@aRemotes){
		my @aCol = split(' ',$sCurRem);
		#print "$sCurRem\n";
		if( $aCol[1] =~ "$srABaseGitHttp" or $aCol[1] =~ "$srABaseGitSSH"){
			#print "Has rA as origin\n";
			if($aCol[0] =~ "origin") { $sRaOrigin = 1; }
			if($aCol[0] =~ "upstream") { $sRaUpstream = 1; }
		}
	}
	if($sRaOrigin==0 and $sRaUpstream==0){
		print "Adding rA as a upstream\n";
		$sGit->run("remote" => "add" => "upstream" => "$srABaseGitHttp");
	}
	return ($sRaOrigin==0);
}
