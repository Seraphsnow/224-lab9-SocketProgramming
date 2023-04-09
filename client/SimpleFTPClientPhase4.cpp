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

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        cerr << "Run the code with 4 command line arguments, first is serverIPAddr:port and second is the operation, get or put, third is the file to send/recieve and fourth is the recieve interval.\n";
        exit(1);
    }

    string ipport = argv[1], filename = argv[3];
    int revInt = stoi(argv[4]);
    int portnum;
    int start = ipport.find(':', 0) + 1;
    portnum = stoi(ipport.substr(start, ipport.find('\0', 0) - start));
    string ip = ipport.substr(0, start - 1);
    if (strcmp(argv[2], "get") == 0)
    {
        //cout << "hi\n";
        char buf[MAXDATASIZE];
        ofstream file(filename, ios::binary);
        if (!file.is_open())
        {
            cerr << "Given file cannot be created or written to\n";
            exit(3);
        }
        int sockfd;
        struct sockaddr_in dest_addr;
        sockfd = socket(PF_INET, SOCK_STREAM, 0);
        if (sockfd == -1)
        {
            cerr << "Socker Error\n";
            exit(0);
        }
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(portnum);
        dest_addr.sin_addr.s_addr = inet_addr(ip.c_str());
        memset(&(dest_addr.sin_zero), '\0', 8);
        if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)) == -1)
        {
            cerr << "Error in forming connection, try again\n";
            exit(2);
        }
        else
        {
            char str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &dest_addr.sin_addr, str, INET_ADDRSTRLEN);
            cout << "ConnectDone: " << str << ":" << ntohs(dest_addr.sin_port) << endl;
        }
        string stringtosend = "get " + filename + "\0";
        cout << send(sockfd, stringtosend.c_str(), stringtosend.size(), 0) << endl;
        int bytes = 0;
        while (1)
        {
            memset(buf, '\0', MAXDATASIZE);
            // file Transfer left
            int rec = recv(sockfd, buf, MAXDATASIZE, 0);
            if (rec == -1)
            {
                continue;
            }
            if (rec == 0)
            {
                if (bytes == 0)
                {
                    cerr << "Unable to create/write the given/recieved file\n";
                    exit(3);
                }
                close(sockfd);
                file.close();
                cout << "FileWritten: " << bytes << " bytes\n";
                return 0;
            }
            else
            {
                file.write(buf, rec);
                bytes += rec;
            }
            usleep(1000 * revInt);
        }
    }
    else if (strcmp(argv[2], "put") == 0)
    {
        ifstream file;
        file.open(filename, ios::binary);
        if (file.fail())
        {
            cerr << "Given file not present or is unreadable\n";
            exit(3);
        }
        int sockfd;
        struct sockaddr_in dest_addr;
        sockfd = socket(PF_INET, SOCK_STREAM, 0);
        if (sockfd == -1)
        {
            cerr << "Socket Error\n";
            exit(0);
        }
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(portnum);
        dest_addr.sin_addr.s_addr = inet_addr(ip.c_str());
        memset(&(dest_addr.sin_zero), '\0', 8);
        if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)) == -1)
        {
            cerr << "Error in forming connection, try again\n";
            exit(2);
        }
        else
        {
            char str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &dest_addr.sin_addr, str, INET_ADDRSTRLEN);
            cout << "ConnectDone: " << str << ":" << ntohs(dest_addr.sin_port) << endl;
        }
        string stringtosend = "put " + filename + "\0";
        send(sockfd, stringtosend.c_str(), stringtosend.size(), 0);
        // usleep(500000);
        //cout << stringtosend << endl;
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
            bytes_sent = send(sockfd, buffer + total, min(left, MAXDATASIZE), 0);
            //cout << bytes_sent << " " << left << endl;
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
            usleep(1000 * revInt);
        }
        close(sockfd);
        file.close();

        cout << "TransferDone: " << size << " bytes\n";
    }
    else
    {
        cerr << "Wrong op code\n";
        exit(0);
    }
}