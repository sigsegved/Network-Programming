------------------------------------------
USER DOCUMENTATION:
------------------------------------------
The archived file submitted contains the following directories and files.. The files mentioned in bin and obj directory will be seen after make. 

CS533_Assignment1
|
|-- bin
|   |-- client			
|   |-- daytimecli
|   |-- echocli
|   `-- server
|-- inc
|   |-- CS533_Client.h
|   |-- cs533.h
|   |-- CS533_Server.h
|   |-- echocli.h
|   |-- echotimecli.h
|   |-- readline.h
|   |-- services.h
|   `-- utils.h
|-- makefile
|-- obj
|   |-- CS533_Client.o
|   |-- CS533_Server.o
|   |-- echocli.o
|   |-- echotimecli.o
|   |-- services.o
|   `-- utils.o
`-- src
    |-- CS533_Client.c
    |-- CS533_Server.c
    |-- echocli.c
    |-- echotimecli.c
    |-- services.c
    `-- utils.c

4 directories, 25 files

---------------------------------------
Directories
----------------------------------------
* bin --> contains the executables after the projec is compile using make. 
* obj --> contains all the object files 
* src --> contains all the source files used in the project
* inc --> contains all the header files used. 

----------------------------------------
FILES
-----------------------------------------
* Executables 
--------------
	** server -->  The main executable file, which creates 2 sockets listening on ports 18181 and 18182 and accepts connection and starts threads for each client accepted. 
	USAGE : ./server 

	** client -->  The main executable file for running the client. It simply takes the ip/hostname as command line arguement and asks the user what kind of client he is wants and starts the requested client in a new xterm window. 
	USAGE : ./client <ip/hostname>

	** echocli --> The echocli lets the user talk to the server. It simply creates a connection with the server and writes a message to server and reads the message written back by server. This executable is solely intended to be used by a process as his child process. But it can also be used as 
	USAGE : ./echocli <ip/hostname> <port> <pipe-fd> 

	** daytimecli --> The echotimecli lets creates a port on which the server writes its current time every 5 seconds. This executable is solely intended to be used by a process as his child process. But it can also be used as 
	USAGE : ./echocli <ip/hostname> <port> <pipe-fd> 

----------------------
HOW TO COMPILE 
----------------------
To compile the source code, go the home directory (CS533_Assignment1) and call make. The makefile is present in ./CS533_Assignment1/makefile. 
The make process will automatically put the objects files to obj directory, executable files to bin directory. 


------------------------------------------
SYSTEM DOCUMENTATION
------------------------------------------

Client Robustness : 
---------------------
Client robustness is handled using various error checking. IO multiplexing technique (select) is used to handle a lot of errors in client side. 
The main client source code files 
	** CS533_Client.c  	--> Reads the user input and starts the requested client. uses the execlp to load the process into xterm by forking a child process. Creats a pipe and reads for the status messages from the child process. When the Pipe is no more readable, parent process assumes that the child is no more. 

	** echocli.c 		--> Contains the code for echo client. Apart from the normal functionality, the client is made robust by using select on both the stdin and the socket descriptor. Which makes the process of identifying the status of the server process simpler. 

	** echotimeclie.c	--> Contains the code for daytime client.  It also uses select to read the data from socket to make it more robust. 

Both the echocli.c and echotimecli.c process are meant to be child process and they keep writing the status on to the pipe. 

Though i tried adding signal handling in the client process using pthread_sigmask function, it created a lot of issues with xterm... So I had to write the client code without signal handlers. But even without the signal handlers, the flow is very robust and handles all error scenarios. 


Server Robustness :
---------------------

Server is made robust by using signal and error handling. since server is daemon process, it can never go down. So enough care has been take care for various signals.

PTHREAD SIGNALLING :
----------------------
The  posix signalling mechanism for threads has been used in this code to handle various signals. I have not handled SIGINT and SIGTERM so that the server can be terminated easily for testing purpose. pthread_sigmask is used to handle threads. And a dedicated thread has been created to handle signals. This signal_handler thread will take care of any signals occuring from any threads.


	** CS533_Server.c 	--> Contains the main server logic... and the signal_handler functions. The code has been highly modularized. startSuperServer is the functions which acts like the inetd super server. The function Accept accepts all the pending connections in a particular socket. 

***NOTE*** NONBLOCK sockets are giving an error message which reads, 'Resourse temporarily not available'. So i have commented the setsockopt and the acceptConnections cannot accept all the incoming connection at the same time ******

	** Services.c 		--> Services.c contains all 2 functions echoService and daytimeService. These functions are thread safe. It uses the thread safe version of the Readline given in unpv13e. 

	** utils.c 		-->  It contains the common utility functions needed by the whole project. Few of the functions have been taken the from unpv13e codes and the rest are implemented as per the project requirements. 


----------------------------------------------------------------
TEST DOCUMENTATION 
----------------------------------------------------------------

Starting the server 
* goto /CS533_Assignment1/bin/
* ./server 

Starting the client
* goto /CS533_Assignment1/bin/
* ./client 127.0.0.1 

Various scenarios tested 
1. Echo client & Server functionality test. 
2. Daytime client & server functionality test. 
3. Abrubt termination of server detected by both the clients. 
4. Hard Termination of clients handled by the server as smooth termination. 
5. Server restart without any bind issues. 
6. Server could handle more than 2 clients of each type at the same time. Maximum tested --> 4 of each clients. 
7. PIPE issues between parent and child process. 
8. Ip/host-name issues as client's cmd line args. 




