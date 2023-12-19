#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <ostream>

#include <unordered_map>
char *_read_file(const char *f);
using namespace std;

// Removed the unnecessary cast of malloc
char *_read_file(const char *f) {
    FILE *file = fopen(f, "r");
    if (file) {
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        rewind(file);
        char *contents = (char *)malloc(size + 1);
        if (contents) {
            fread(contents, 1, size, file);
            contents[size] = '\0';
            fclose(file);
            return contents;
        }
        fclose(file);
    }
    return nullptr;  // File not found or error occurred
}

std::unordered_map<std::string, std::string> url_to_file_;
static int bind(luna_State *L);
static int run(luna_State *L);
static int handle_client_request(int client_socket);


static int bind(luna_State *L) {
    const char *url = lunaL_checkstring(L, 1);
    const char *file_to_show = lunaL_checkstring(L, 2);
    // For simplicity, let's assume binding means associating a URL with a file
    url_to_file_[url] = file_to_show;
    return 0;
}

static int run(luna_State *L) {
    int port = lunaL_checknumber(L, 1);
    // For simplicity, let's assume running means starting a basic HTTP server
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set the SO_REUSEADDR option
    int reuse = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
        perror("Error setting socket option");
        exit(EXIT_FAILURE);
    }

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);
    if (::bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }
    if (listen(server_socket, 5) < 0) {
        perror("Error listening on socket");
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d...\n", port);
    while (true) {
        int client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket < 0) {
            perror("Error accepting connection");
            continue;
        }
        handle_client_request(client_socket);
        close(client_socket);
    }
    return 0;
}


static int handle_client_request(int client_socket) {
    // For simplicity, let's assume handling a request means sending the contents of the associated file
    const char *response_template = "HTTP/1.1 200 OK\r\nContent-Length: %zu\r\n\r\n%s";
    const char *not_found_response = "HTTP/1.1 404 Not Found\r\n\r\nFile not found.";
    // Read the request (assuming a simple GET request for demonstration purposes)
    char buffer[1024];
    ssize_t bytesRead = read(client_socket, buffer, sizeof(buffer) - 1);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        printf("Received request:\n%s\n", buffer);
        // Extract the requested URL
        const char *url_start = strstr(buffer, "GET ");
        if (url_start) {
            const char *url_end = strchr(url_start + 4, ' ');
            if (url_end) {
                std::string requested_url(url_start + 4, url_end);
                // Find the associated file
                auto it = url_to_file_.find(requested_url);
                if (it != url_to_file_.end()) {
                    // File found, send its contents
                    char *file_contents = _read_file(it->second.c_str());
                    if (file_contents) {
                        char response[1024];
                        snprintf(response, sizeof(response), response_template, strlen(file_contents), file_contents);
                        write(client_socket, response, strlen(response));
                        free(file_contents);
                    } else {
                        // Error reading file
                        write(client_socket, not_found_response, strlen(not_found_response));
                    }
                } else {
                    // URL not found
                    write(client_socket, not_found_response, strlen(not_found_response));
                }
            }
        }
    }
    return 0;
}

// Assuming luna_State and lunaL_checktype are part of your luna C API bindings.
static int cbind(luna_State *L) {
    // Check if the argument is a table
    if (!luna_istable(L, 1)) {
        lunaL_error(L, "Argument must be a table");
        return 0;
    }

    // Iterate over the table and bind URLs to files
    luna_pushnil(L);  // Start iterating from the beginning
    while (luna_next(L, 1) != 0) {
        // Key is at index -2 and value is at index -1
        const char *url = lunaL_checkstring(L, -2);
        const char *file = lunaL_checkstring(L, -1);

        url_to_file_[url] = file;

        // Pop the value, leaving the key for the next iteration
        luna_pop(L, 1);
    }

    return 0;
}

static int crun(luna_State *L) {
    int port = lunaL_checknumber(L, 1);

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    int reuse = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
        perror("Error setting socket option");
        exit(EXIT_FAILURE);
    }

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if (::bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) < 0) {
        perror("Error listening on socket");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", port);

    while (true) {
        int client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket < 0) {
            perror("Error accepting connection");
            continue;
        }

        // Handle client request
        if (handle_client_request(client_socket) != 0) {
            // Error handling request, exit
            close(server_socket);
            close(client_socket);
            exit(EXIT_FAILURE);
        }

        close(client_socket);
    }

    return 0;
}

static int init_server(luna_State *L) {

    // Create a new table
    luna_newtable(L);

    // Add functions to the table
    luna_pushcfunction(L, bind);
    luna_setfield(L, -2, "bind");

    luna_pushcfunction(L, cbind);
    luna_setfield(L, -2, "cbind");
    // Add functions to the table
    luna_pushcfunction(L, run);
    luna_setfield(L, -2, "run");

    luna_pushcfunction(L, crun);
    luna_setfield(L, -2, "crun");
    // You can add more functions as needed

    return 1;  // Return the table
}
