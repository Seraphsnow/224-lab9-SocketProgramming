#include <bits/stdc++.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;

#define BACKLOG 10

#define MAXDATASIZE 1024
void send_file(FILE *fp, int sockfd)
{
    int n;
    char data[MAXDATASIZE] = {0};

    while (fgets(data, MAXDATASIZE, fp) != NULL)
    {

        if (send(sockfd, data, sizeof(data), 0) == -1)
        {
            perror("[-]Error in sending file.");
            exit(1);
        }
        bzero(data, MAXDATASIZE);
    }
}

int main(int argc, char *argv[])
{
    int portnum = stoi(argv[1]);
    if (argc != 2)
    {
        cerr << "Run the code with only 1 command line arguments, the port number.\n";
        exit(1);
    }

    int sockfd, new_fd;
    int yes = 1;
    struct sockaddr_in my_addr;
    struct sockaddr_in their_addr;
    socklen_t sin_size = sizeof(struct sockaddr);
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        cerr << "Socket Error\n";
        exit(0);
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        cerr << "Port already in use\n";
        exit(0);
    }
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(portnum);
    my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    memset(&(my_addr.sin_zero), '\0', 8);
    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
    {
        cerr << "Issues in binding, try using a different port number\n";
        exit(2);
    }
    else
    {
        cout << "BindDone: " << portnum << endl;
    }
    if (listen(sockfd, BACKLOG) != -1)
    {
        cout << "ListenDone: " << portnum << endl;
    }
    else
    {
        cerr << "Issues is listening\n";
        exit(0);
    }
    while (1)
    {

        sin_size = sizeof(struct sockaddr_in);
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd != -1)
        {
            char str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &their_addr.sin_addr, str, INET_ADDRSTRLEN);
            cout << "Client: " << str << ":" << ntohs(their_addr.sin_port) << endl;
        }
        else
        {
            cerr << "Issues in accepting\n";
            exit(0);
        }
        char filenamebuf[MAXDATASIZE];
        memset(filenamebuf, '\0', MAXDATASIZE);

        int rec;
        rec = recv(new_fd, filenamebuf, MAXDATASIZE, 0);
        string filename = filenamebuf;
        if (filename[0] != 'g' || filename[1] != 'e' || filename[2] != 't' || filename[3] != ' ')
        {
            cout << "UnknownCmd\n";
            cerr << "Command format given via client is not known\n";
            close(new_fd);
            continue;
            // 1c chalra tha part 2 ka
        }
        else
        {
            filename = filename.substr(4, filename.size() - 4);
            cout << "FileRequested: " << filename << "\n";
        }

        ifstream file;
        file.open(filename, ios::binary);
        if (file.fail())
        {
            cout << "FileTransferFail\n";
            cerr << "Given file not present or is unreadable\n";
            close(new_fd);
            continue;
        }

        file.seekg(0, ios::end);
        int size = file.tellg();
        file.seekg(0, ios::beg);
        char *buffer = new char[size];
        file.read(buffer, size);
        file.close();
        int *fsize = &size;

        int total = 0, bytes_sent, left = size;
        while (1)
        {
            bytes_sent = send(new_fd, buffer + total, min(left, MAXDATASIZE), 0);
            // cout << bytes_sent << " " << left << endl;
            if (bytes_sent == -1)
                continue;
            if (total == size && bytes_sent >= 0)
            {
                break;
            }
            else
            {
                total += bytes_sent;
                left -= bytes_sent;
            }

            // cout << size << endl;
        }
        cout << "TransferDone: " << size << " bytes\n";

        // Transfer end
        close(new_fd);
    }
    close(sockfd);
}