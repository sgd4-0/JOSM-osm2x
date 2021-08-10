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

    // prompt, json id, standard value
    std::string prompt[4][3] = {
        "Enter username             \t\"string\": ", "username", "\"default\"",
        "Enter start position       \t[lat, lon]: ", "start", "",
        "Enter destination position \t[lat, lon]: ", "dest", "[0.000,0.000]",
        "Enter address              \t\"string\": ", "address", "\"\""
    };

    std::string hindernis[2][3] = {
        "Enter username\t\"string\" : ", "username", "\"default\"",
        "Enter position\t[lat, lon]: ", "position", ""
    };

    while (true)
    {
        std::string nav;
        std::cout << "Type 0 for navigation or 1 for obstacle: ";
        std::getline(std::cin, nav);

        if (nav.length() < 1)
        {
            std::cerr << "Input is not valid! Try again.\n";
            continue;
        }

        std::string msg = "{\n\t\"mode\": " + nav;
        if (std::stoi(nav) > 0)
        {
            // obstacle
            for (uint8_t i = 0; i < 1; i++)
            {
                std::string in;
                std::cout << hindernis[i][0];
                std::getline(std::cin, in);

                if (in.size() < 1 && hindernis[i][2].length() < 1)
                {
                    std::cerr << "Mandatory field can not be empty!\n";
                    i -= 1;
                    continue;
                }
                else if (in.size() < 1)
                {
                    in = hindernis[i][2];
                }
                if (msg.size() > 3) msg.append(",\n");
                msg.append("\t\"" + hindernis[i][1] + "\": " + in);
            }
        }
        else
        {
            // navigation
            for (uint8_t i = 0; i < 4; i++)
            {
                std::string in;
                std::cout << prompt[i][0];
                std::getline(std::cin, in);


                if (in.size() < 1 && prompt[i][2].length() < 1)
                {
                    std::cerr << "Mandatory field can not be empty!\n";
                    i -= 1;
                    continue;
                }
                else if (in.size() < 1)
                {
                    in = prompt[i][2];
                }

                if (msg.size() > 3) msg.append(",\n");
                msg.append("\t\"" + prompt[i][1] + "\": " + in);
            }
        }
        
        msg.append("\n}");

        std::cout << "Send message :\n" << msg << "\n";
        std::cout << "\n-------------------------------------------------\n\n";

        send(sock , msg.c_str() , msg.length() , 0 );
        msg.clear();
    }
    
    return 0;
}