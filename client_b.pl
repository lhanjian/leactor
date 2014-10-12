#!/usr/bin/perl -w
use IO::Socket::INET;
use strict;
use threads;

my $sock = IO::Socket::INET->new(PeerAddr=>'localhost',
                              PeerPort=>'12344',
                              Proto   =>'tcp',) or die("Failed !");
#$sock->write("test", 10, 0):
$sock->autoflush(1);
print "connected to the server\n";
<>;
print "wait end\n";
<>;
use vars;
my $number = 0;
open(my $file, '>', 'report.txt') or die;
my $num = \$number;

my $recvvv = threads->create(
    sub {
        local $SIG{INT} = sub {
            print $file "$$num\n";
            return $$num;
        };
        while (1) {
            my $msg;
            (defined $sock->recv($msg, 32)) or die "\ndie:recvF:error<$!>\nnum:$$num\n";
            print ">".$msg."\0";
            $$num++;
        } 
    }
);

threads->create(
    sub {
        threads->detach();
        my $counter = 1;
        while ($counter > 0) {
            my $data = "send CT:$counter to SERVER:BBTB";
            $counter++;
            $sock->send($data) or die "sendF: $!";
        }

        print "wait\n";
        <>;
        print "waitEND\n";
        my $data = "sendSECOND CT:$counter to SERVER:BBTB";
        $sock->send($data) or die "sendF: $!";
    }
);
$SIG{INT} = sub {
    $recvvv->kill('INT')->join();
};
sleep 15000;

$sock->close();
