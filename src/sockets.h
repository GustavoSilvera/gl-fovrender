#ifndef SOCKET_H
#define SOCKET_H

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <vector>

// TODO: read these from the params.ini
#define MAX_ID_LEN 100
#define RECV_ID "/tmp/send"

using namespace std;

// Global socket information to use throughout the program
/* socket address of the receiver-socket, to listen  */
static struct sockaddr_un receiver_addr = {MAX_ID_LEN, AF_UNIX, RECV_ID};
/* file descriptor of the receiver-socket, to listen  */
static int receiver_fd = 0;
#define VERBOSE true

/**
 * @brief receives all the data from a client descriptor into a buffer at a certain rate
 * @param[in] conn_fd The file descriptor of the connection to receive from
 * @param[out] buffer The resulting buffer to write the data into
 * @param[in] buffer_amnt The maximum amount to recv() at a time
 * @returns response_len The number of bytes received from the socket connection
 */
inline int conn_recv(const int conn_fd, vector<char> &data, const int buf_amnt = 128)
{
    data.clear();
    int response_len = 0;
    char buffer[buf_amnt];
    while (true)
    {
        int chunk_amnt = recv(conn_fd, buffer, sizeof(buffer), 0);
        if (chunk_amnt <= 0)
            break;
        response_len += chunk_amnt;
        // append newly received chunk to overall data
        for (size_t i = 0; i < chunk_amnt; i++)
            data.push_back(buffer[i]);
    }
    return response_len;
}

/**
 * @brief Closes the input sockets manually
 * @param[in] recv_fd The file descriptor of the "receiving" socket
 */
inline void close_sockets(const int &recv_fd)
{
    close(recv_fd);
}

/**
 * @brief waits for a response from the server and receives it all
 * @param[in] data The buffer to write the data into
 * @returns 0 if successful, -1 otherwise
 */
inline int listen_once(vector<char> &data)
{
    int client_fd;
    int addr_len = sizeof(receiver_addr);
    if ((client_fd = accept(receiver_fd, (struct sockaddr *)&receiver_addr, (socklen_t *)&addr_len)) < 0)
    {
        cout << "\033[31m"
             << "Unable to accept connection\n"
             << "\033[00m" << endl;
        return -1;
    }
    int response_len = conn_recv(client_fd, data);
    close(client_fd);
    if (VERBOSE)
        cout << "\033[36m"
             << "Received " << response_len << " bytes from server"
             << "\033[00m" << endl;
    return 0;
}

/**
 * @brief initializes the 'receiver' (server) connection to the simulator
 * @param[in] addr The address of the robot-receiver socket
 * @param[in] receiver_fd The file descriptor of the robot-receiver socket
 * @returns client_fd if successful (nonnegative), -1 otherwise
 */
inline int init_recv_conn(struct sockaddr_un &addr, int &receiver_fd)
{
    int client;
    int opt = 1;
    if ((receiver_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        perror("socket() error");
        exit(EXIT_FAILURE);
    }
    setsockopt(receiver_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    // {
    //     perror("setsockopt() error");
    //     exit(EXIT_FAILURE);
    // }
    unlink(RECV_ID); // delete the UNIX socket file if still in use
    bind(receiver_fd, (struct sockaddr *)&addr, sizeof(addr));
    // {
    //     perror("bind() error");
    //     exit(EXIT_FAILURE);
    // }
    if (listen(receiver_fd, 1) < 0)
    {
        perror("listen() error");
        exit(EXIT_FAILURE);
    }
    int addr_len = sizeof(receiver_fd);
    if ((client = accept(receiver_fd, (struct sockaddr *)&addr, (socklen_t *)&addr_len)) < 0)
    {
        perror("accept() error");
        exit(EXIT_FAILURE);
    }
    // success!
    if (VERBOSE) // at least print the first time
        cout << "\033[32m"
             << "Receiver connection established"
             << "\033[00m" << endl;
    // client should always be nonnegative integer
    return client;
}

#endif