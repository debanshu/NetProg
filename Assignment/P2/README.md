#####Question:

Write a simple proxy filter with the following specifications:

- Client is a web browser. Set the browser's HTTP proxy to point to your server.
- When the server starts, it reads from a file 'blocklist.txt' having a list of domain names which are to be forbidden to access. one domain per line.
- When a HTTP request comes to your server, it checks if the domain name exists in the list. If it is, the server sends back HTTP error 403 Forbidden to the client. If not it sends the request to the actual server. When it gets the reply, it sends the reply to the client. 
- Your server takes a port number on the command line.  it listens at this port.  It should be a concurrent server with process-per-client model.

Implement proxy.c.


####Build Instructions:

- Compile file using the following commands:

gcc Proxy.c -o Proxy

- Make list of blocked addresses in a blocklist.txt file in same directory as Proxy executable


####Running:

- Start Proxy with a command-line argument for port (3456 for example).

./Proxy 3456

- Set browser to point to localhost & port 3456 (or any port given). Then use browser normally. 
- Trying to access blocked sites will give error.

