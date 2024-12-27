# Test Assigment

You need to implement server side of a network protocol that's a centralized map with the following requirements:

1. Client sends commands in a form of "key value\n", i.e. string key, single space, string value and a NL character.
2. Server maintains a map of (key, heavyHash(value)) pairs, each client request sets or updates a pair in this map.
3. heavyHash(value) is assumed to be CPU intensive task, there's an implementation called calcHeavyHash in vmsserver/Utils.cpp, you can use it
   for testing or use your own.
4. Server supports multiple clients.
5. Whenever a key is added or changed in the map on server this change is propagated to all clients (including the one who made it). The change is sent as
   "key heavyHash(value)\n", i.e. similar to how client sends its commands. It's not necessary to send all of the updates to client if that client has,
   for example, low connection bandwidth, but he must receive fresh server state as soon as possible.
6. Whenever a client connects to server with a non-empty map he must receive all entries in that map, one by one, as defined in the protocol.

This project builds 2 binaries: vmsclient and vmsserver. vmsclient is already implemented and can be used by simply running it from console
like this: ./vmsclient  
Once it establishes connection to server you can start typing commands like this:
```
key1 value1
key2 value2
...
```
or type
```
exit
```
to disconnect. Alternatively you can cat in a file with commands: `cat in.txt | ./vmsclient` - this will send all commands inside in.txt
to server and then just listen for changes forever.

vmsserver is just a stub that you need to implement, it's implemented up to the point of receiving client connections:

```c++
ec = acceptor->listen(10, [](boost::asio::ip::tcp::socket s) {
    auto conn = std::make_shared<Connection>(std::move(s));
    conn->start();
});
```

Some implementation considerations:

1. The code should be asynchronous, i.e. it should handle many client connections simultaneously.
2. Taking advantage of multiple CPU cores is a big plus.
3. Code should handle misbehaving clients, spurious client disconnects, etc.
