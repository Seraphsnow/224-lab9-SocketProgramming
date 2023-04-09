#include <bits/stdc++.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
using namespace std;

#define BACKLOG 10

#define MAXDATASIZE 1000
#define ll long long int

int main(int argc, char *argv[])
{
    int portnum = stoi(argv[1]);
    ll fdmax = 0;
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
    map<int, string> filenameData;
    map<int, ll> bytesSent;
    map<int, bool> fileopen;
    map<int, ifstream> files;
    map<int, ofstream> outfiles;
    map<int, ll> bytesrec;
    map<int, bool> get;
    struct timeval tv;
    fd_set readset, writeset, totalset;
    tv.tv_sec = 2; // change these for timeout purposes
    tv.tv_usec = 500000;
    FD_ZERO(&readset);
    FD_ZERO(&writeset);
    FD_ZERO(&totalset);
    FD_SET(sockfd, &totalset);
    fdmax = sockfd;
    for (;;)
    {
        readset = totalset;
        writeset = totalset;
        if (select(fdmax + 1, &readset, &writeset, NULL, &tv) == -1)
        {
            cerr << "Issue in select";
            exit(0);
        }
        for (int i = 0; i <= fdmax; i++)
        {
            if (FD_ISSET(i, &readset))
            {
                if (i == sockfd) // new connection
                {

                    int addrlen = sizeof(their_addr);
                    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
                    if (new_fd != -1)
                    {
                        FD_SET(new_fd, &totalset);
                        if (new_fd > fdmax)
                        {
                            fdmax = new_fd;
                        }
                        char str[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &their_addr.sin_addr, str, INET_ADDRSTRLEN);
                        cout << "Client: " << str << ":" << ntohs(their_addr.sin_port) << endl;
                        char filenamebuf[MAXDATASIZE];
                        memset(filenamebuf, '\0', MAXDATASIZE);

                        int rec;
                        rec = recv(new_fd, filenamebuf, MAXDATASIZE, 0);
                        string filename = filenamebuf;
                        if ((filename[0] != 'g' || filename[1] != 'e' || filename[2] != 't' || filename[3] != ' ') &&
                            (filename[0] != 'p' || filename[1] != 'u' || filename[2] != 't' || filename[3] != ' '))
                        {
                            cout << "UnknownCmd\n";
                            cerr << "Command format given via client is not known\n";
                            close(new_fd);
                            FD_CLR(new_fd, &totalset);
                            continue;
                        }
                        else if (strcmp(filename.substr(0, 4).c_str(), "get ") == 0)
                        {

                            filename = filename.substr(4, filename.size() - 4);
                            cout << "FileRequested: " << filename << "\n";
                            filenameData[new_fd] = filename;
                            fileopen[new_fd] = 0;
                            get[new_fd] = 1;
                        }
                        else
                        {
                            filename = filename.substr(4, filename.size() - 4);
                            cout << "FileBeingTransfered: " << filename << "\n";
                            filenameData[new_fd] = filename;
                            fileopen[new_fd] = 0;
                            get[new_fd] = 0;
                        }
                    }
                    else
                    {
                        cerr << "Issues in accepting\n";
                        exit(0);
                    }
                }
                else
                {
                    if (!fileopen[i])
                    {
                        outfiles[i].open(filenameData[i], ios::binary);
                        if (!outfiles[i].is_open())
                        {
                            cerr << "Given file cannot be created or written to for socket " << i << endl;
                            close(i);
                            FD_CLR(i, &totalset);
                            continue;
                        }
                        fileopen[i] = 1;
                        bytesrec[i] = 0;
                    }
                    char buf[MAXDATASIZE];
                    memset(buf, '\0', MAXDATASIZE);
                    int rec = recv(i, buf, MAXDATASIZE, 0);
                    if (rec == -1)
                    {
                        continue;
                    }
                    if (rec == 0)
                    {
                        if (bytesrec[i] == 0)
                        {
                            cerr << "Unable to create/write the given/recieved file\n";
                        }
                        close(i);
                        FD_CLR(i, &totalset);
                        outfiles[i].close();
                        if (bytesrec[i] != 0)
                            cout << "FileWritten: " << bytesrec[i] << " bytes\n";
                    }
                    else
                    {
                        outfiles[i].write(buf, rec);
                        bytesrec[i] += rec;
                    }
                }
            }
            else if (FD_ISSET(i, &writeset) && get[i])
            {
                

                if (!fileopen[i])
                {

                    files[i].open(filenameData[i], ios::binary);
                    if (files[i].fail())
                    {
                        cout << "FileTransferFail\n";
                        cerr << "Given file not present or is unreadable\n";
                        close(i);
                        FD_CLR(i, &totalset);
                        continue;
                    }
                    fileopen[i] = 1;
                    bytesSent[i] = 0;
                }

                files[i].seekg(0, ios::end);
                int sizeleft = files[i].tellg();
                sizeleft -= bytesSent[i];
                files[i].seekg(bytesSent[i], ios::beg);
                char *buffer = new char[min(sizeleft, MAXDATASIZE)];
                files[i].read(buffer, min(sizeleft, MAXDATASIZE));
                int len, bytes_sent;

                bytes_sent = send(i, buffer, min(sizeleft, MAXDATASIZE), 0);

                if (bytes_sent == -1)
                    continue;
                bytesSent[i] += bytes_sent;
                if (bytes_sent == sizeleft)
                {
                    cout << "TransferDone: " << bytesSent[i] << " bytes\n";
                    files[i].close();
                    fileopen[i] = 0;
                    close(i);
                    FD_CLR(i, &totalset);
                }
            }
        }
    }
    close(sockfd);
}