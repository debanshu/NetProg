#####Question:

Write a download accelerator using the following specifications:
- The client has to modeled as select with nonblocking I/O. It should use non-blocking connect, non-blocking read and write operations.
- When a user gives URL for downloading a file,  client  makes  N connections. N is the command-line argument.  
- Client should check whether all the connections were successful. 
- Client sends requests over all these connections, each one requesting size/N bytes. 
- All I/O has to be handled with select() call.
- When all requests issued are complete, the tool combines the fragments downloaded into one single file


####ISSUES:
- Server closes the connection before sending the entre requested byte range.
