#####Question:

Consider the need for group communication among the users on the LAN. There are a number of groups constituted by the administrator of this network. These groups are added/deleted/updated by the administrator in a dynamic way. That means nodes can’t remember the addresses of groups. These groups are maintained by the LiPro server in a data structure. Assume that the following is the format:

groupname  multicast ip  port
Networks     239.0.0.1  2300
Databases    239.1.0.2  2301

The nodes every time when they start-up, send a broadcast request to well-known port number. If the LiPro  server is up, it will reply with the list of groups, ips and port numbers. Nodes upon receiving the list, display it to the user. User selects the groups, he wants to join, iteratively. Nodes join the selected groups. Nodes also provide interfaces for sending messages to any of the selected groups. LiPro server takes well-known port number, group1-name, group1-ip, group1-port, group2-name, …. on the command-line. It displays the broadcast requests it receives from nodes with their addresses. The node takes  the well-known port-number on command-line. It 
displays the messages received from groups.

Implement LiPro.c and Node.c for the above specifications.


####Build Instructions:

Compile both using the following commands:

gcc LiPro.c -o LiPro
gcc Node.c -o Node

####Running:

- For LiPro, see the usage example (specifies 4 multicast groups):

./LiPro 2330 Networks 239.1.0.2 2300 Databases 239.1.0.3 2301 Algorithms 239.1.0.4 2302 Architecture 239.1.0.5 2303

- For Node, start with (port number is 2330 according to above LiPro execution):

./Node [LiPro_port_number]

- Select groups to join by replying with lowercase y or n. (single characters only)
- Choose one of the selected groups to send message to by entering Group Name (case-sensitive).

####NOTES:

1. If running multiple instances of Node on same system; you cannot join the same group on two instances. This limitation is due to specificity of port binding (exclusive) for multicast groups. (Can be solved by using proper SOCKET OPTION)

2. Every group receives its own messages as well.
