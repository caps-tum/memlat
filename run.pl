#!/usr/bin/perl
#

($prefix,$from,$to,$step) = @ARGV;

if ($prefix eq "") {
  print "Usage: $ARGV[0] <prefix> [<from> [<to> [<stepfactor]]]\n\n";
  print "Values <from> and <to> are in units of 1000.\n";
  print "Default: <from> = 10000, <to> = 100000, <stepfactor> = 2\n\n";
  exit;
}
if ($from eq "") { $from = 10000; }
if ($to eq "") { $to = 100000; }
if ($step eq "") { $step = 2; }

if ($from * $step > $to) {
  print "Only 1 step. Please specify a greater range/smaller stepfactor !\n";
  exit;
}

$cur = $from;
@results = ();
while($cur <= $to) {
  print "Running './bench $cur > $prefix.$cur' with size $cur...\n"; 
  system "./bench $cur > $prefix.$cur";
  push @results, "$prefix.$cur";
  $cur = $cur * $step;
}

print "Producing Gnuplot Script '$prefix.gp'...\n";
open OUT, ">$prefix.gp";
print OUT "set term postscript\n";
print OUT "plot ";
foreach $f (@results) {
  if ($fcount>0) { print OUT ","; }
  print OUT "'$f' w l";
  $fcount++;
}
print OUT "\n";
close OUT;

$cmd = "gnuplot $prefix.gp > $prefix.ps";
print "Running Gnuplot ('$cmd')...\n";
system $cmd;



