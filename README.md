# Janus is a portable, unified and lightweight interface for mitm applications.

It acts like a deamon and offers two simple stream sockets, one for input and one for the output traffic manipulations.
Over this sockets, before a packet, it's always appended it's size (16bit), and Janus expects to receive data back with this precise format.
The code is a portable and optimized rewrite of a first idea implemented in SniffJoke software written by Claudio Agosti.
Janus overrides the actual routing table, creating a fake gateway with the aim to block packets after the kernel (on outgoing traffic) and before the kernel (on incoming traffic).

# Requirements

    found your operating system between those implemented in os-confs/
    starting janus, with the correct symlink /etc/janus/current-os poiting to your selected os-confs file, contains the appropriate checks.

    If you're using a package, could have provided itself with the appropriate dependencies.

# Mac OSX Requirements

    Because of Darwin TUN/TAP absence by default, you need install this external support:
    http://tuntaposx.sourceforge.net/download.xhtml
    Anyway, at the moment the MacOSX supports don't work, for both an ipfw and /dev/tun issues.

# Below is an example of Janus usage starting from this initial routing table:

    root@linux# route -n
    Kernel IP routing table
    Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
    0.0.0.0         10.196.136.1    0.0.0.0         UG    0      0        0 eth0
    10.196.135.0    0.0.0.0         255.255.255.0   U     0      0        0 eth0
    10.196.136.0    0.0.0.0         255.255.255.0   U     0      0        0 eth1
    94.23.192.28    10.196.136.1    255.255.255.255 UGH   0      0        0 eth0
    94.228.214.57   10.196.135.1    255.255.255.255 UGH   0      0        0 eth1

#Example of Janus usage:
    root@linux# janus
    Janus is now going to background, use SIGTERM to stop it.

    root@linux# route -n
    Kernel IP routing table
    Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
    0.0.0.0         212.77.1.1      0.0.0.0         UG    0      0        0 tun0
    10.196.135.0    0.0.0.0         255.255.255.0   U     0      0        0 eth0
    10.196.136.0    0.0.0.0         255.255.255.0   U     0      0        0 eth1
    94.23.192.28    10.196.135.1    255.255.255.255 UGH   0      0        0 eth0
    94.228.214.57   10.196.136.1    255.255.255.255 UGH   0      0        0 eth1
    212.77.1.1      0.0.0.0         255.255.255.255 UH    0      0        0 tun0

## Client POC

    % This is a proof of concept written in Erlang that implements a simple Janus Client.
    % It shows how simple is the Janus interface executing a simple packet-echo.
    %
    % Usage: erl -compile janus_client_poc && erl --noshell -s janus_client_poc start
    %

    -module(janus_client_poc).
    -export([start/0]).

    start() ->
        JanusHost = "127.0.0.1",
        JanusPort1 = 10203,
        JanusPort2 = 30201,
        spawn(fun() -> connect(JanusHost, JanusPort1) end),
        spawn(fun() -> connect(JanusHost, JanusPort2) end).

    connect(Host, Port) ->
        {ok, Socket} = gen_tcp:connect(Host, Port, [{active,false}, {packet,0}]),
        echo_loop(Socket).

    echo_loop(Socket) ->
        case gen_tcp:recv(Socket, 0) of
            {ok, Data} ->
                % potential packet mangling activity
                io:format("Hello, World!~n"),
                gen_tcp:send(Socket, Data),
                echo_loop(Socket);
            {error, _} ->
                ok
        end.

## Installed files (paths may vary on your system)

Janus binary /usr/local/sbin/janus

Janus man page /usr/local/man/man1/janus.1

Official Janus page:
    https://github.com/evilaliv3/janus

# GPG public keys

    $ gpg --keyserver pgp.mit.edu --recv-key D9A950DE
    $ gpg --fingerprint --list-keys D9A950DE
    pub   1024D/D9A950DE 2009-05-10 [expires: 2014-05-09]
          Key fingerprint = C1ED 5C8F DB6A 1C74 A807  5695 91EC 9BB8 D9A9 50DE
    uid                  Giovanni Pellerano <giovanni.pellerano@evilaliv3.org>
    sub   4096g/50A7F150 2009-05-10 [expires: 2014-05-09]

    $ gpg --keyserver pgp.mit.edu --recv-key C6765430
    $ gpg --fingerprint --list-keys C6765430
    pub   1024D/C6765430 2009-08-25 [expires: 2011-08-25]
          Key fingerprint = 341F 1A8C E2B4 F4F4 174D  7C21 B842 093D C676 5430
    uid                  vecna <vecna@s0ftpj.org>
    uid                  vecna <vecna@delirandom.net>
    sub   3072g/E8157737 2009-08-25 [expires: 2011-08-25]
