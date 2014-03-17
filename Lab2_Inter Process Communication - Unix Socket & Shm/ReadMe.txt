Inter Process Communication:  Unix Socket and Shared Memory to enable processes to send/receive messages.

Driver behaves like client process.
Application behaves like server process. 
run Driver after application.


Correctness
In your program, the driver process generates random integers and keeps track of the sum of the
integers sent to the application process. On the receiving side, the application process also counts the
sum of the received integers. After the driver is done, it will send a “special” message to the application
and then exit. Upon the exit point, both processes should print the sum of the integers (either sent or
received). If the two sums are equal, your program is “all right”.

Performance
Moreover, you need to measure the throughput of your communication engine. That is, how many
messages can be sent per second? For example, you can measure the time elapsed for a number of M
million messages.


