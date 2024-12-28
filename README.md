# Async Boost TCP Server Implementation

Within this implementation, I needed to write the server side of a network protocol that's a centralized map with the following requirements:

1. Client sends commands in a form of "key value\n", i.e. string key, single space, string value and a NL character.
2. Server maintains a map of (key, heavyHash(value)) pairs, each client request sets or updates a pair in this map.
3. heavyHash(value) is assumed to be CPU intensive task, there's an implementation called calcHeavyHash in vmsserver/Utils.cpp, you can use it
   for testing or use your own.
4. Server supports multiple clients.
5. Whenever a key is added or changed in the map on server this change is propagated to all clients (including the one who made it). The change is sent as
   "key heavyHash(value)\n", i.e. similar to how client sends its commands. It's not necessary to send all of the updates to client if that client has,
   for example, low connection bandwidth, but he must receive fresh server state as soon as possible.
6. Whenever a client connects to server with a non-empty map he must receive all entries in that map, one by one, as defined in the protocol.

This project builds 2 binaries: vmsclient and vmsserver.

To start the services, go to the `scripts` directory:
```shell
cd .\scripts\
```

Then to start the server, run the following script:
```shell
.\run_server.bat 
```

And to start the client, proceed to run the script below:
```shell
.\run_client.bat
```

Or just run the script below to start both services:
```shell
.\run_services.bat
```

***P.S.: Do not forget to build the binaries before trying to run them***

Once the client establishes connection to the server, you can start typing commands in its CMD like this:
```
key1 value1
key2 value2
...
```
or type
```
exit
```
to disconnect. 