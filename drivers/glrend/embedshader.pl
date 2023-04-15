use strict;
use warnings FATAL => 'all';

open(OUTPUT, '>', $ARGV[2]);

printf OUTPUT "static const GLchar %s[] = {\n", $ARGV[0];

open(INPUT, '<', $ARGV[1]);
while (<INPUT>) {
    print OUTPUT ("    ", join(", ", map ord, split(//, $_)), ", \n");
}
close(INPUT);

printf OUTPUT "    0\n};\n";
