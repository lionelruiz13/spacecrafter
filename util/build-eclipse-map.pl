#!/usr/bin/perl

# Build an eclipse shadow lookup texture
# For accurate moon shadows on planets

# Copyright 2010 Digitalis Education Solutions, Inc.
# Distributed under the same license as Nigthshade (GPLv3+)
# http://Stellarium360software.org

use Math::Trig;

$VSIZE = 1024;
$HSIZE = 512;
$M_PI = 3.14159;
$MAX = 255;

# ppm format
print "P6 $HSIZE $VSIZE $MAX\n"; 

for($y=$VSIZE-1; $y>=0; $y--) {

	$radius = $y/20;

	for($x=0; $x<$HSIZE; $x++) {

		$distance = $x/($HSIZE-1) * ($radius + 1);

		$R = pack("C", int(0.5 + ($MAX-1) * &EclipseFactor(1, $radius, $distance)));
		#print "$R$R$R";
		# for easier debugging, not used otherwise
		$G = pack("C", ($VSIZE-$y)/($VSIZE/$MAX));
		$B = pack("C", ($MAX)/2); 
		print "$R$G$B";


	}
}


# Below algorithm converted from the Glunatic project
# Copyright Johannes Gajdosik, 2009

sub EclipseFactor {
	my($R, $r, $d) = @_;

	my $epsilon = $R*1e-6;
	my $rval;
	my $C1 = $R+$r-$d;

	if ($C1 <= $epsilon) {
		$rval = 1.0; # circles are disjoint
	} else {
		my $C2 = $R-$r+$d;
		if ($C2 <= $epsilon) {
			$rval = 0.0; # circle(r) contains circle(R): total eclipse
		} else {
			my $RR = $R*$R;
			my $C3 = $r+$d-$R;
			if ($C3 <= $epsilon) { # circle(R) contains circle(r)
				$rval = ($R-$r)*($R+$r)/$RR;
			} else {
				my $A4 = 0.5*sqrt(($R+$r+$d)*$C1*$C2*$C3);
				my $h = $A4 / $d;
				my $rr = $r*$r;
				my $dd = $d*$d;
				my($Ar,$AR);
				if ($RR >= $rr+$dd) {
					$Ar = $rr * asin($h/$r);
					$AR = $RR * ($M_PI-asin($h/$R));
				} else {
					$Ar = $rr * ($M_PI-asin($h/$r));
					$AR = $RR * (($rr >= $RR+$dd) ? asin($h/$R) : ($M_PI-asin($h/$R)));
				}
				my $A = $A4 + $Ar + $AR;
				$rval = ($A - $rr*$M_PI) / ($RR*$M_PI);
			}
		}
	}
	return $rval;
}

