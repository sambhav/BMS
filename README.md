![Build Status](https://api.travis-ci.org/samj1912/BMS.svg?branch=master)

# BMS
A simple Banking Management System using unix sockets

## How to Run
* Compile the server and client
```
gcc myserver.c -o myserver

gcc myclient.c -o myclient
```
* Run the server in one terminal instance with the port as the first argument
```
./myserver $PORT
```
* Run the client in a different terminal tab with the IP and port of the server instance as its arguments.
```
./myclient $IP $PORT
```
For local debug environment IP = 127.0.0.1
* Create a login credentials file named ***login_file***. The format of an example *login_file* is given below
```
Customer1 Pass1 C
Customer2 Pass2 C
Admin Admin A
Police Police P
```
The format is 
```
$USERNAME $PASSWORD $AUTH_TYPE
```
where AUTH_TYPE can be **A** for admin, **P** for police and **C** for customers.

