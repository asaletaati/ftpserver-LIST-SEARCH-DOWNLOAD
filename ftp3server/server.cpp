#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>
#include <dirent.h>

const int PORT = 2121;
const int BUFFER_SIZE = 1024;

void handleClient(int clientSocket) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) break;

        std::string command(buffer);
        
        if (command.substr(0, 4) == "LIST") {
            // Send list of files
            DIR *dir;
            struct dirent *ent;
            std::string fileList;
            if ((dir = opendir(".")) != NULL) {
                while ((ent = readdir(dir)) != NULL) {
                    fileList += ent->d_name;
                    fileList += "\n";
                }
                closedir(dir);
                send(clientSocket, fileList.c_str(), fileList.size(), 0);
            } else {
                const char* error = "Could not open directory\n";
                send(clientSocket, error, strlen(error), 0);
            }
        } else if (command.substr(0, 6) == "SEARCH") {
            // Search for a file
            std::string filename = command.substr(7);
            std::ifstream file(filename);
            if (file.good()) {
                send(clientSocket, "File found\n", 11, 0);
            } else {
                send(clientSocket, "File not found\n", 15, 0);
            }
        } else if (command.substr(0, 8) == "DOWNLOAD") {
            // Download a file
            std::string filename = command.substr(9);
            std::ifstream file(filename, std::ios::binary);
            if (file) {
                // Send file in chunks
                char fileBuffer[BUFFER_SIZE];
                while (file.read(fileBuffer, sizeof(fileBuffer))) {
                    send(clientSocket, fileBuffer, file.gcount(), 0);
                }
                // Send any remaining bytes
                if (file.gcount() > 0) {
                    send(clientSocket, fileBuffer, file.gcount(), 0);
                }
                file.close();
            } else {
                const char* error = "File not found\n";
                send(clientSocket, error, strlen(error), 0);
            }
        }
        // Additional commands can be handled here
    }
    close(clientSocket);
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return -1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        close(serverSocket);
        return -1;
    }

    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Listen failed" << std::endl;
        close(serverSocket);
        return -1;
    }

    std::cout << "FTP server started on port " << PORT << std::endl;

    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue; // Try to accept the next connection
        }
        std::cout << "Client connected" << std::endl;
        handleClient(clientSocket);
    }

    close(serverSocket);
    return 0;
}
