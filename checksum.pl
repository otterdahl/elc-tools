#!/usr/bin/perl -w

use strict;

while(<>) {
	chomp;
	s/\r//;
	die("Invalid hex format") unless s/^://;
	die("Incorrect record size") unless length($_) % 2 == 0;

	# Calculate checksum
	@_ = split(/(.{2})/);
	my $saved_hex = hex pop @_; # don't add the saved crc
	my $sum;
	foreach my $v (@_) {
		$sum += hex($v);
	}

	# Reduce sum according to 2-complement
	$sum = ~$sum + 1;
	$sum %= 0x100;

	unless($saved_hex == $sum) {
		print STDERR "CRC NOT OK\n";
		printf(STDERR "CRC in file:%x  our CRC:%x\n", $saved_hex, $sum);
		die();
	}
}

__END__

=head1 NAME

checksum - verify the checksum of Intel 8-bit hex files

=head1 SYNPOSIS

B<checksum> B<[hex-file]>

=head1 DESCRIPTION

Verifies the checksum of Intel 8-bit hex files. Exit code 0 if successful.

=head1 SEE ALSO

Intel 8-bit hex specification taken from:
http://www.scienceprog.com/shelling-the-intel-8-bit-hex-file-format/

=head1 HISTORY

First version appeared in February 2008

=head1 AUTHOR

David Otterdahl <david.otterdahl@gmail.com>

=cut
