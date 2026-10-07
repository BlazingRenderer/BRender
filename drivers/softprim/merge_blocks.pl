#!/usr/bin/env perl
#
# Merge the per-table softprim block lists into the two consumers' tables.
#
# infogen.pl emits one SOFTPRIM_BLOCK line per .ifg block, carrying the axis
# tuple (which names the kernel) and the metadata pentprim's matcher tests, and
# one SOFTPRIM_REFUSED line per block outside the implemented axis spec.
# The tuple is the kernel's identity across tables - the general RGB tables and
# the MMX tables both describe "INDEX_8 texture into an RGB output" shapes - so
# the same kernel name appears once per contributing block.
#
# Two outputs, because they need different things:
#
#   KERNELS  - one line per distinct kernel, for raster.cpp to define. A kernel
#              is the tuple, so duplicates collapse; contributing blocks must
#              agree on the tuple and on colour7bit (part of the kernel).
#              A refused block has no kernel and is not here at all.
#
#   MATCHERS - every block, in table order, for match.c, refused blocks
#              included. This is the ordered block list the matcher walks: a
#              block whose flag or buffer-type requirements the bound state
#              fails falls through to the next, which is pentprim's
#              match_block() behaviour. It must not be deduplicated, because
#              two blocks can share a kernel and disagree on their requirements
#              (the MMX and general tables do exactly that), and because
#              pentprim's table order is what decides which block wins. A
#              refused block has to be in this list or the variant it spells
#              would fall through to its twin - see spFindMatch.
#
# The inputs are passed in the order the matcher must try them: for each output
# format, the MMX table (which pentprim's use_mmx selects first) before the
# general one. See drivers/softprim/CMakeLists.txt.
#
# Written by the build, not part of the source tree.
#
use strict;
use warnings;

my $out_kernels  = shift @ARGV or die "usage: merge_blocks.pl KERNELS MATCHERS IN...\n";
my $out_matchers = shift @ARGV or die "usage: merge_blocks.pl KERNELS MATCHERS IN...\n";
my @in           = @ARGV or die "usage: merge_blocks.pl KERNELS MATCHERS IN...\n";

my (@order, %kernels);

# The fixed fields ahead of the tuple in each generated line.
my $NFIELDS = 16;

open(my $om, '>', $out_matchers) or die "$out_matchers: $!";

for my $f (@in) {
    open(my $fh, '<', $f) or die "$f: $!";
    while (my $l = <$fh>) {
        chomp $l;
        next unless $l =~ /^(SOFTPRIM_BLOCK|SOFTPRIM_REFUSED)\((.*)\)$/;
        my ($macro, $args) = ($1, $2);
        my @p = split(/,\s*/, $args, $NFIELDS + 1);
        die "$f: short $macro line: $l\n" if @p < $NFIELDS + 1;
        my ($name, $fmask, $fcmp, $sd, $us, $oy, $c7, $cs0, $cs1, $cs2, $ms, $dt, $tt, $st, $bt, $ft, $tuple) = @p;

        # A kernel's argument list - and therefore the wrapper both consumers
        # define for it - depends on the topology: softrend calls a triangle
        # with three vertices, a line with two and a point with one. The
        # topology is in the tuple, so the macro name is spelled from it:
        # SOFTPRIM_BLOCK_TRI/_LINE/_POINT. A refused block keeps the single
        # SOFTPRIM_REFUSED name - it has no kernel and no arity.
        my $out_macro = $macro;
        if ($macro eq 'SOFTPRIM_BLOCK') {
            if    ($tuple =~ /\bSP_TOP_TRI\b/)   { $out_macro = 'SOFTPRIM_BLOCK_TRI'; }
            elsif ($tuple =~ /\bSP_TOP_LINE\b/)  { $out_macro = 'SOFTPRIM_BLOCK_LINE'; }
            elsif ($tuple =~ /\bSP_TOP_POINT\b/) { $out_macro = 'SOFTPRIM_BLOCK_POINT'; }
            else { die "softprim: block $name has no topology in its tuple\n"; }
        }

        print $om "$out_macro($args)\n";

        next if $macro eq 'SOFTPRIM_REFUSED';

        if (!exists $kernels{$name}) {
            push @order, $name;
            $kernels{$name} = [$c7, $tuple, $out_macro];
        } else {
            die "softprim: kernel $name has two different tuples\n" if $kernels{$name}->[1] ne $tuple;
            die "softprim: kernel $name disagrees on colour7bit\n" if $kernels{$name}->[0] != $c7;
            die "softprim: kernel $name disagrees on topology\n" if $kernels{$name}->[2] ne $out_macro;
        }
    }
    close $fh;
}

close $om;

open(my $ok, '>', $out_kernels) or die "$out_kernels: $!";
for my $n (@order) {
    printf $ok "%s(%s, %d, %s)\n", $kernels{$n}->[2], $n, $kernels{$n}->[0], $kernels{$n}->[1];
}
close $ok;
