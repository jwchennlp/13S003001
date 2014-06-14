UDP-Filesend
============

A reliable UDP-based file transfer application developed using C sockets. It was written to gain experience in lower-level networking and both the protocol and payload sizes have not yet been optimized.  That being said, it is fully functional and can be used to transfer arbitrary files quite effectively on supported platforms (Linux/Unix).

<h3>Compilation:</h3>
client: make sender <br>
daemon: make receiver

<h3>Usage:</h3>
receiver &<br>
sender host filename
