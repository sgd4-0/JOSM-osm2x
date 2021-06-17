// Client side C/C++ program to demonstrate Socket programming
// Code from https://www.geeksforgeeks.org/socket-programming-cc/
#include <stdio.h>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <vector>

#define PORT 8080
   
int main(int argc, char const *argv[])
{
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
   
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
       
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    // ROS liefert aktuelle Position und Zielposition
    // weitere Daten: username, 

    std::vector<std::string> prompt = {
        "Enter username: ",
        "Enter start position (lat)      : ",
        "Enter start position (lon)      : ",
        "Enter destination position (lat): ",
        "Enter destination position (lon): ",
        "Enter address: "
    };

    std::vector<std::string> hindernis = {
        "Enter node lat: ",
        "Enter node lon: "
    };
    
    while (true)
    {
        std::string nav;
        std::cout << "Type 0 for navigation or 1 for obstacle: ";
        std::getline(std::cin, nav);

        std::string msg = "{0:" + nav;
        if (std::stoi(nav) > 0)
        {
            // obstacle
            for (uint8_t i = 0; i < hindernis.size(); i++)
            {
                std::string in;
                std::cout << hindernis[i];
                std::getline(std::cin, in);

                if (in.size() < 1)  continue;

                if (msg.size() > 3) msg.append(",");
                msg.append(std::to_string(i+2) + ":" + in);
            }
        }
        else
        {
            for (uint8_t i = 0; i < prompt.size(); i++)
            {
                std::string in;
                std::cout << prompt[i];
                std::getline(std::cin, in);

                if (in.size() < 1)  continue;

                if (msg.size() > 3) msg.append(",");
                msg.append(std::to_string(i+1) + ":" + in);
            }
        }
        
        msg.append("}");

        std::cout << "Send message " << msg << "\n";
        std::cout << "\n-------------------------------------------------\n\n";

        send(sock , msg.c_str() , msg.length() , 0 );
        msg.clear();
    }
    
    return 0;
}