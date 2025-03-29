/**
 * @File httpserver.c
 *
 * This file contains the main function for the HTTP server.
 *
 * @author Shreya Handa
 */

 #include "asgn2_helper_funcs.h"
 #include "debug.h"
 #include "protocol.h"
 #include <arpa/inet.h>
 #include <err.h>
 #include <errno.h>
 #include <fcntl.h>
 #include <netinet/in.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <signal.h>
 #include <string.h>
 #include <sys/stat.h>
 #include <unistd.h>
 #include <regex.h>
 #include <stdbool.h>
 
 #define BUFFER_SIZE 4096
 /** @brief Handles a connection from a client.
  *
  *  @param connfd The file descriptor for the connection.
  *
  *  @return void
  */
 
 // server writing
 void get_request(int connfd, const char *uri) {
     int fd = open(uri, O_RDONLY);
     if (fd < 0) {
         const char *error_404 = "HTTP/1.1 404 Not Found\r\n"
                                 "Content-Length: 10\r\n"
                                 "\r\n"
                                 "Not Found\n";
         write(connfd, error_404, strlen(error_404));
         //        return;
     }
 
     // finding file size
     struct stat file_stat;
     if (fstat(fd, &file_stat) < 0) {
         close(fd);
         const char *error_500 = "HTTP/1.1 500 Internal Server Error\r\n"
                                 "Content-Length: 22\r\n"
                                 "\r\n"
                                 "Internal Server Error\n";
         write(connfd, error_500, strlen(error_500));
         //        return;
     }
 
     if (stat(uri, &file_stat) == 0 && S_ISDIR(file_stat.st_mode)) {
         const char *error_response = "HTTP/1.1 403 Forbidden\r\n"
                                      "Content-Length: 10\r\n"
                                      "\r\n"
                                      "Forbidden\n";
         write(connfd, error_response, strlen(error_response));
         //   		 return;
     }
 
     // prepares HTTP response headers
     // takes into account large files
     char correct_ok[BUFFER_SIZE];
     snprintf(correct_ok, sizeof(correct_ok),
         "HTTP/1.1 200 OK\r\n"
         "Content-Length: %lld"
         "\r\n\r\n",
         (long long) file_stat.st_size);
 
     // sends response headers
     write(connfd, correct_ok, strlen(correct_ok));
 
     // reads file contents and sends to client
     char buffer[BUFFER_SIZE];
     ssize_t bytes_read;
     while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
         write(connfd, buffer, bytes_read);
     }
 
     close(fd);
 }
 
 void put_request(
     int connfd, const char *uri, int content_length, char *buffer, int bytes, int bytes_read) {
     int file_exists = access(uri, F_OK) == 0;
 
     // opening file and writing to it
     int fd = open(uri, O_WRONLY | O_CREAT | O_TRUNC, 0666);
     if (fd < 0) {
         if (errno == EACCES || errno == EPERM) {
             const char *error_response = "HTTP/1.1 403 Forbidden\r\n"
                                          "Content-Length: 10\r\n"
                                          "\r\n"
                                          "Forbidden\n";
             write(connfd, error_response, strlen(error_response));
             //            close(fd);
             //            return;
         } else {
             const char *error_response = "HTTP/1.1 500 Internal Server Error\r\n"
                                          "Content-Length: 22\r\n"
                                          "\r\n"
                                          "Internal Server Error\n";
             write(connfd, error_response, strlen(error_response));
             //            close(fd);
             //            return;
         }
         close(fd);
         return;
     }
 
     ssize_t total_written = bytes;
     int content_written = 0;
     int length = bytes_read;
     while (total_written < length) {
         ssize_t char_seen = write_n_bytes(fd, buffer + total_written, length - total_written);
         if (char_seen < 0) {
             close(fd);
             const char *error_response = "HTTP/1.1 500 Internal Server Error\r\n"
                                          "Content-Length: 22\r\n"
                                          "\r\n"
                                          "Internal Server Error\n";
             write(connfd, error_response, strlen(error_response));
             //   close(connfd);
             //   return;
             //              regfree(&myregex2);
             //            return;
         }
 
         // Update total bytes written
         total_written += char_seen;
         content_written += char_seen;
     }
 
     int bytes_remain = content_length - content_written;
     if (bytes_remain > 0) {
         pass_n_bytes(connfd, fd, bytes_remain);
     }
 
     if (file_exists) {
         const char *error_200 = "HTTP/1.1 200 OK\r\n"
                                 "Content-Length: 3\r\n"
                                 "\r\n"
                                 "OK\n";
         write(connfd, error_200, strlen(error_200));
         // return;
         //        close(fd);
         //        return;
     } else {
         const char *created_response = "HTTP/1.1 201 Created\r\n"
                                        "Content-Length: 8\r\n"
                                        "\r\n"
                                        "Created\n";
         write(connfd, created_response, strlen(created_response));
         // return;
         //        close(fd);
         //        return;
     }
 
     //regfree(&myregex2);
     close(fd);
     return;
 }
 
 bool validate_clrf(const char *buffer) {
     const char *end = strstr(buffer, "Content-Length:");
     if (end) {
         end = strstr(end, "\r\n"); // finds end of Content-Length line
         if (end) {
             return strncmp(end + 2, "\r\n", 2) == 0; // checks if \r\n\r\n follows
         }
     }
     return false;
 }
 
 // parsing through the printf command
 void handle_connection(int connfd) {
     /* Your code here */
 
     char buffer[BUFFER_SIZE];
     ssize_t bytes_read;
 
     bytes_read = read_until(connfd, buffer, sizeof(buffer), "\r\n\r\n");
 
     if (bytes_read <= 0) {
         const char *error_404 = "HTTP/1.1 400 Bad Request\r\n"
                                 "Content-Length: 12\r\n"
                                 "\r\n"
                                 "Bad Request\n";
         write(connfd, error_404, strlen(error_404));
         return;
     }
 
     // null terminate
     buffer[bytes_read] = '\0';
 
     regex_t myregex;
     regmatch_t mymatches[4];
 
     if (regcomp(&myregex, REQUEST_LINE_REGEX, REG_EXTENDED) != 0) {
         fprintf(stderr, "500 Internal Server Error\n");
         return;
     }
 
     // currently a pointer
     // also checks if everything matches with regexec
     if (regexec(&myregex, buffer, 4, mymatches, 0) == 0) {
         char method[65];
         char uri[65];
         char version[65];
 
         strncpy(method, buffer + mymatches[1].rm_so,
             mymatches[1].rm_eo
                 - mymatches[1].rm_so); // changing method from pointer to string, GET/PUT
         // null terminate, method[len] = '\0'
         method[mymatches[1].rm_eo - mymatches[1].rm_so] = '\0';
 
         strncpy(uri, buffer + mymatches[2].rm_so,
             mymatches[2].rm_eo
                 - mymatches[2].rm_so); // changing uri from pointer to string, foo.txt
         // null terminate
         uri[mymatches[2].rm_eo - mymatches[2].rm_so] = '\0';
 
         strncpy(version, buffer + mymatches[3].rm_so,
             mymatches[3].rm_eo
                 - mymatches[3].rm_so); // changing version from pointer to string, HTTP/1.1
         // null terminate
         version[mymatches[3].rm_eo - mymatches[3].rm_so] = '\0';
 
         printf("Method: %s\n", method);
         printf("URI: %s\n", uri);
         printf("Version: %s\n", version);
 
         if (strcmp(version, "HTTP/1.1") != 0) {
             const char *error_response = "HTTP/1.1 505 Version Not Supported\r\n"
                                          "Content-Length: 22\r\n"
                                          "\r\n"
                                          "Version Not Supported\n";
             write(connfd, error_response, strlen(error_response));
             regfree(&myregex);
             close(connfd);
             return;
         }
 
         if (strcmp(method, "GET") != 0 && strcmp(method, "PUT") != 0) {
             const char *error_response = "HTTP/1.1 501 Not Implemented\r\n"
                                          "Content-Length: 16\r\n"
                                          "\r\n"
                                          "Not Implemented\n";
             write(connfd, error_response, strlen(error_response));
             regfree(&myregex);
             close(connfd);
             return;
         }
 
         if (strcmp(method, "GET") == 0) {
             if (strncmp(buffer + bytes_read - 4, "\r\n\r\n", 4) != 0) {
                 const char *error_400 = "HTTP/1.1 400 Bad Request\r\n"
                                         "Content-Length: 12\r\n"
                                         "\r\n"
                                         "Bad Request\n";
                 write(connfd, error_400, strlen(error_400));
                 close(connfd);
                 return;
             } else {
                 get_request(connfd, uri);
             }
         } else if (strcmp(method, "PUT") == 0) {
             if (!validate_clrf(buffer)) {
                 const char *error_400 = "HTTP/1.1 400 Bad Request\r\n"
                                         "Content-Length: 12\r\n"
                                         "\r\n"
                                         "Bad Request\n";
                 write(connfd, error_400, strlen(error_400));
                 close(connfd);
             } else {
                 regex_t myregex2;
                 regmatch_t mymatches2[3];
 
                 if (regcomp(&myregex2, CONTENT_LENGTH_REGEX, REG_EXTENDED) != 0) {
                     const char *error_404 = "HTTP/1.1 400 Bad Request\r\n"
                                             "Content-Length: 12\r\n"
                                             "\r\n"
                                             "Bad Request\n";
                     write(connfd, error_404, strlen(error_404));
                     //              regfree(&myregex2);
                     //              return;
                 }
 
                 // THIS ONE CHECKS FOR CONTENT LENGTH
                 if (regexec(&myregex2, buffer, 3, mymatches2, 0) != 0) {
                     const char *error_response = "HTTP/1.1 400 Bad Request\n"
                                                  "Content-Length: 12\r\n"
                                                  "\r\n"
                                                  "Bad Request\n";
                     write(connfd, error_response, strlen(error_response));
                     //                    close(connfd);
                     //                    regfree(&myregex2);
                     //                    return;
                 }
 
                 regfree(&myregex2);
 
                 regex_t myregex3;
                 regmatch_t mymatches3[100];
 
                 if (regcomp(&myregex3, HEADER_FIELD_REGEX, REG_EXTENDED) != 0) {
                     const char *error_404 = "HTTP/1.1 400 Bad Request\r\n"
                                             "Content-Length: 12\r\n"
                                             "\r\n"
                                             "Bad Request\n";
                     write(connfd, error_404, strlen(error_404));
                     //                regfree(&myregex3);
                     //                return;
                 }
 
                 // THIS ONE CHECKS FOR NUMBER CONTENT LENGTH?
                 if (regexec(&myregex3, buffer, 100, mymatches3, 0) != 0) {
                     const char *error_response = "HTTP/1.1 400 Bad Request\n"
                                                  "Content-Length: 12\r\n"
                                                  "\r\n"
                                                  "Bad Request\n";
                     write(connfd, error_response, strlen(error_response));
                     //		close(connfd);
                     //		return;
                     //                regfree(&myregex3);
                     //                return;
                 }
 
                 char cont_len[128];
 
                 strncpy(cont_len, buffer + mymatches2[1].rm_so,
                     mymatches2[1].rm_eo - mymatches2[1].rm_so);
                 cont_len[mymatches2[1].rm_eo - mymatches2[1].rm_so] = '\0';
 
                 int content_length = atoi(cont_len);
 
                 if (content_length < 0) {
                     const char *error_response = "HTTP/1.1 400 Bad Request\n"
                                                  "Content-Length: 12\r\n"
                                                  "\r\n"
                                                  "Bad Request\n";
                     write(connfd, error_response, strlen(error_response));
                     //                regfree(&myregex3);
                     //                return;
                 }
 
                 regfree(&myregex3);
 
                 regex_t myregex4;
                 regmatch_t mymatches4[2];
 
                 if (regcomp(&myregex4, "\r\n\r\n", REG_EXTENDED) != 0) {
                     const char *error_404 = "HTTP/1.1 400 Bad Request\r\n"
                                             "Content-Length: 12\r\n"
                                             "\r\n"
                                             "Bad Request\n";
                     write(connfd, error_404, strlen(error_404));
                     //                regfree(&myregex4);
                     //                return;
                 }
 
                 // THIS ONE checks for double \r\n\r\n
                 if (regexec(&myregex4, buffer, 2, mymatches4, 0) != 0) {
                     const char *error_response = "HTTP/1.1 400 Bad Request\n"
                                                  "Content-Length: 12\r\n"
                                                  "\r\n"
                                                  "Bad Request\n";
                     write(connfd, error_response, strlen(error_response));
                     close(connfd);
                     return;
                     //                regfree(&myregex4);
                     //                return;
                 }
 
                 int bytes = mymatches4[0].rm_eo;
                 put_request(connfd, uri, content_length, buffer, bytes, bytes_read);
 
                 regfree(&myregex4);
 
                 //            regfree(&myregex);
                 //            regfree(&myregex2);
                 //            regfree(&myregex3);
                 //            regfree(&myregex4);
             }
         }
     } else {
         const char *error_404 = "HTTP/1.1 400 Bad Request\r\n"
                                 "Content-Length: 12\r\n"
                                 "\r\n"
                                 "Bad Request\n";
         write(connfd, error_404, strlen(error_404));
         //        return;
     }
 
     regfree(&myregex);
     close(connfd);
     return;
 }
 
 /** @brief Main function for the HTTP server.
  *
  *  @param argc The number of arguments.
  *  @param argv The arguments.
  *
  *  @return EXIT_SUCCESS if successful, EXIT_FAILURE otherwise.
  */
 
 int main(int argc, char **argv) {
     if (argc < 2) {
         warnx("wrong arguments: %s port_num", argv[0]);
         fprintf(stderr, "usage: %s <port>\n", argv[0]);
         return EXIT_FAILURE;
     }
 
     char *endptr = NULL;
     size_t port = (size_t) strtoull(argv[1], &endptr, 10);
 
     // Add error checking for the port number
     if (errno == ERANGE || *endptr != '\0' || port < 1 || port > 65535) {
         fprintf(stderr, "Invalid port number.");
         return EXIT_FAILURE;
     }
 
     signal(SIGPIPE, SIG_IGN);
     Listener_Socket sock;
     listener_init(&sock, port);
 
     while (1) {
         int connfd = listener_accept(&sock);
         handle_connection(connfd);
     }
 
     return EXIT_SUCCESS;
 } 