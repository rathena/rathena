#!/bin/sh
#simple script to setupperl environnement with all module dependancy needed
cpan install File::Basename Getopt::Long DBI DBD::mysql YAML YAML:XS Cwd Net::Ping Scalar::Util Git::Repository;
