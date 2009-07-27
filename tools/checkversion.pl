#!/usr/bin/perl -w

##########################################################################
# INFORMATION TOOL ABOUT THE SERVERS VERSION OF ATHENA
#
# By connection on a server, this software display the version of the
# designed server.
#-------------------------------------------------------------------------
# Usages:
#   ./checkversion.pl IP:port
#   ./checkversion.pl IP port
#   perl checkversion.pl IP:port
#   perl checkversion.pl IP port
#
# note: default port: 6900
#
# When successfull, the software return the value 0.
#
##########################################################################

#------------------------- start of configuration ------------------------

$connecttimeout = 10;   # Connection Timeout (in seconds)

#-------------------------- End of configuration -------------------------

use IO::Socket;

unless($ARGV[0]) {
	print "USAGE: $0 server_ip:port\n";
	exit(1);
}

$server = $ARGV[0];
$port = $ARGV[1];
$port = $1 if ($server =~ s/:(\d+)//);
$port ||= 6900;

# Connection to the server
my($so,$er) = ();
eval{
	$so = IO::Socket::INET->new(
		PeerAddr=> $server,
		PeerPort=> $port,
		Proto   => "tcp",
		Timeout => $connecttimeout) or $er = 1;
};

if($er || $@) {
	print "Can't not connect to server [$server:$port] !\n";
	exit(2);
}

# Request for the server version
print $so pack("v",30000); # 0x7530
$so->flush();

# Receiving of the answer of the server
if (read($so,$buf,10) < 10) {
	print "Invalid answer. It isn't an athena server or it is a too old version.\n";
	exit(5);
}

# Sending end of connection to the server
print $so pack("v",30002); # 0x7532
$so->flush();

# Analyse of the answer
my($ret,$maver,$miver,$rev,$dev,$mod,$type,$mdver) = unpack("v c6 v",$buf);

if ($ret != 30001) { # 0x7531
	print "Invalid answer. It isn't an athena server or it is a too old version.\n";
	exit(6);
}

my(@stype) = ();
foreach $i(0..3) {
	push @stype,(("login","char","inter","map")[$i]) if( $type & (1<<$i) );
}
print "  ".join("/",@stype)." server [$server:$port].\n";
printf "  Athena version %s-%d.%d", ("stable","dev")[$dev], $maver,$miver;
printf " revision %d",$rev if $rev;
printf "%s%d\n",("","-mod")[$mod],$mdver;

exit(0);
