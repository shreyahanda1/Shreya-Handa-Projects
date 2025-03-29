Goal
Experience building a system that uses client-server/strong modularity. My learning objectives are: (1) practice implementing a client-server system, (2) practice the advantages of powerful abstractions, (3) practice implementing a large system that solves a large problem, (4) review string parsing, and (5) review memory management.

Table 1
200 OK OK\n When a method is successful
201 Created Created\n When a URI’s file is created
400 Bad Request Bad Request\n When a request is ill-formatted
403 Forbidden Forbidden\n When a URI’s file is not accessible
404 Not Found Not Found\n When the URI’s file does not exist
500 Internal Server Error Internal Server Error\n When an unexpected issue prevents processing
501 Not Implemented Not Implemented\n When a request includes an unimplemented method
505 Version Not Supported Version Not Supported\n When a request includes an unsupported version

Methods
I implement two HTTP methods, GET and PUT.
• GET: A GET request indicates that the client would like to receive the content of the file identified by the URI. If a request is valid and specifies a URI that is resident in the directory in which httpserver is executing, then httpserver should produce a response that...
    1. has a status-code of 200
    2. has a message-body that includes the current state of the file pointed to by uri, and
    3. has a Content-Length that indicates the number of bytes in the file.
For all other requests (i include those that are valid but where the uri indicates a non-existent file), my server produces a status-code message-body, and Content-Length based upon Table 1.

• PUT: A PUT request indicates that the client would like to update or replace the content of the file identified by the uri.
If a valid PUT request’s URI points to a file that does not yet exist, httpserver should...
    1. create the file
    2. set the file’s contents equal to the message-body in the request
    3. produce a response with a status-code, message-body, and Content-length for the status-code 201 based upon Table 1.
If a valid PUT request’s URI points to a file that already exists, httpserver should...
    1. replace the file’s contents with the message-body in the request
    2. produce a response with a status-code, message-body, and Content-length for the status-code 200 based upon Table 1.
For all other requests, my server produces a status-code, message-body, and Content-Length based upon Table 1.

Examples
1. Ex. 1 The client makes a GET request by sending this string
GET /foo.txt HTTP/1.1\r\n\r\n
The server does not change any files. It responds to the client by sending the client the following response:
HTTP/1.1 200 OK\r\nContent-Length: 21\r\n\r\nHello World, I am foo
2. Ex. 2 The client makes a PUT by sending:
PUT /foo.txt HTTP/1.1\r\nContent-Length: 21\r\n\r\nHello foo, I am World
The server replaces the contents of foo.txt with “Hello foo, I am World” and responds by sending the client the following content
HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nOK\n
3. Ex. 3 The client makes a PUT request by sending:
PUT /new.txt HTTP/1.1\r\nContent-Length: 14\r\n\r\nHello\nI am new
The server creates a new file in its directory, named new.txt, with the contents “Hello\nI am new”.
It then responds to the client with the following:
HTTP/1.1 201 Created\r\nContent-Length: 8\r\n\r\nCreated\n
4. Ex. 4 The client makes a GET request by sending:
GET /not.txt HTTP/1.1\r\n\r\n
Since not.txt does not exist in the server’s directory, the server responds with the following:
HTTP/1.1 404 Not Found\r\nContent-Length: 10\r\n\r\nNot Found\n
5. Ex. 5 The client makes an invalid request by sending:
GET /foo.txt HTTP/1.10\r\nhello*world: value\r\n\r\n
This request is invalid because (1) HTTP/1.10 is an invalid Version since 10 is not a single digit number; AND (2) The header-field is invalid since the key hello*world contains the * character. So, the server will respond by sending the client the following content:
HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\
