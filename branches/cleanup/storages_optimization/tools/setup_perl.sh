#!/bin/sh
# Perl Setup
# Sets up perl environment with all required module dependencies.
cpan install File::Basename Getopt::Long DBI DBD::mysql YAML YAML:XS Cwd Net::Ping Scalar::Util Git::Repository;
