#!/usr/bin/perl -w
use IO::Socket::INET;
use strict;
use threads;

my $sock = IO::Socket::INET->new(PeerAddr=>'localhost',
                              PeerPort=>'12344',
                              Proto   =>'tcp',) or die("Failed !");
#$sock->write("test", 10, 0):
#$sock->autoflush(1);
print "connected to the server\n";
<>;
print "wait end\n";
<>;
my $number = 0;
open(my $file, '>', 'report.txt') or die;

$SIG{INT} = sub {
    die "Caught$\$number\n";
};

threads->create(
    sub {
        threads->detach();
        while (1) {
            my $msg;
            (defined $sock->recv($msg, 32)) or die "recvF:error<$!>\n";
            print ">".$msg."\0";
            $number++;
            print $number;
        } 
    }
);

threads->create(
    sub {
        threads->detach();

        while (1) {
            my $data = "send to SERVER:BBTB";
            $sock->send($data) or die "sendF: $!";
        }
    }
);

sleep 15000;

$sock->close();
