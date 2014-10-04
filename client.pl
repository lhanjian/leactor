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
my $wait = <>;
print "wait end\n";
    

my $rcv = threads->create(sub {
        threads->detach();

        while (1) {
            print "recv S\n";
            my $msg;
            defined $sock->recv($msg, 32) or die "recvF:error<$!>\n";
            print "recv O\n";
            print ">".$msg."\0";
            print "recv P\n";
        }
});

if (1) {
    my $snd = 
    threads->create(sub {
            threads->detach();

            while (1) {
                print "send S\n";
                my $data = "dATA\n";
                defined $sock->send($data) or die "sendF: $!";
                print "send O\n";
                sleep 5;
            }

        });
}

sleep 15000;
print "sleep over\n";

$sock->close();
