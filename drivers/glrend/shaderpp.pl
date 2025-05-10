use strict;
use warnings FATAL => 'all';

open(OUTPUT, '>', $ARGV[1]);


sub write_hex {
    print OUTPUT $_[0];
}

sub read_file {
    my $fh;
    open($fh, '<', $_[0]) || die "Can't open file '$_[0]'";
    while (my $line = <$fh>) {
        my $ifile;
        if($line =~ /^\s*#include\s*"(.+)(?{ $ifile = $^N })"\s*$/) {
            write_hex("// BEGIN INCLUDE $ifile\n");
            read_file($ifile);
            write_hex("// END INCLUDE $ifile\n");
        } else {
            write_hex($line);
        }
    }
    close($fh);
}

read_file($ARGV[0]);
