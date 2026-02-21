#!/usr/bin/env perl
#
# Embed a SPIR-V binary as a C array of 32-bit words, for vkCreateShaderModule.
#
# Usage: embedspv.pl <varname> <input.spv> <output.h>
#
# The array is emitted as `unsigned int` rather than bytes so it is naturally aligned
# and can be handed to VkShaderModuleCreateInfo::pCode without alignment games.
use strict;
use warnings FATAL => 'all';

my ($varname, $input, $output) = @ARGV;
die "usage: embedspv.pl <varname> <input.spv> <output.h>\n" unless defined $output;

open(my $in, '<:raw', $input) || die "Can't open '$input': $!";
my $data = do { local $/; <$in> };
close($in);

die "SPIR-V '$input' is not a multiple of 4 bytes\n" if length($data) % 4;

my @words = unpack('V*', $data);

open(my $out, '>', $output) || die "Can't open '$output': $!";

print $out "static const unsigned int ${varname}[] = {\n";

my $line = '    ';
for my $i (0 .. $#words) {
    my $word = sprintf('0x%08XU, ', $words[$i]);
    if(length($line) + length($word) > 100) {
        print $out $line, "\n";
        $line = '    ';
    }
    $line .= $word;
}
print $out $line, "\n};\n";

close($out);
