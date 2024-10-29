#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>

const char* SERVER_IP = "127.0.0.1"; // Change to your server's IP
const int PORT = 2121;

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        return -1;
    }

    while (true) {
        std::string command;
        std::cout << "Enter command (LIST, SEARCH <filename>, DOWNLOAD <filename>): ";
        std::getline(std::cin, command);
        
        // Send command to server
        send(sock, command.c_str(), command.size(), 0);

        char buffer[1024] = {0};
        int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
        std::string response(buffer, bytesReceived);

        // Handle responses based on command
        if (command.substr(0, 4) == "LIST") {
            std::cout << "Server response: " << response << std::endl;
        } else if (command.substr(0, 6) == "SEARCH") {
            std::cout << "Server response: " << response << std::endl;
        } else if (command.substr(0, 8) == "DOWNLOAD") {
            std::string filename = command.substr(9);
            std::ofstream outFile(filename, std::ios::binary);
            if (outFile.is_open()) {
                // Continue receiving file data until complete
                while ((bytesReceived = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
                    outFile.write(buffer, bytesReceived);
                }
                outFile.close();
                std::cout << "File downloaded successfully: " << filename << std::endl;
            } else {
                std::cerr << "Could not open file for writing: " << filename << std::endl;
            }
        } else {
            std::cerr << "Unknown command." << std::endl;
        }
    }

    close(sock);
    return 0;
}
