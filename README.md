# CN_CHomeworks2
In this code, we want to simulate a wireless computer network. 

There are three main parts to our nodes. One is the mapper, the second is the client, the third is the master. We use UDP for communications between master and client and between client and mapper, and TCP for communications between master and mapper. Each class has a handle read which is used when we sent a packet to the node of that class. The handler for our client recieves the packet and prints it. The handler for master takes the packet, removes the header, repackages it into another packet and sends it to all mappers. The handler for maps takes the header, matches it, repackages it into another packet and sends it to the client.

