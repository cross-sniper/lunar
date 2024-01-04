#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <functional>
#include <unordered_map>

using namespace std;

static char *_read_file(const char *f);

unordered_map<string, string> url_to_file_;
unordered_map<string, function<void(const char *, size_t, int)>> url_to_post_handler_;

static int handle_client_request(int client_socket);
static char *_read_file(const char *f) {
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

static int cwrite(luna_State *L) {
    const char *response = lunaL_checkstring(L, 1);
    int client_socket = lunaL_checknumber(L, 2);

    write(client_socket, response, strlen(response));

    return 0;
}

static int handle_post(luna_State *L) {
    const char *url = lunaL_checkstring(L, 1);
    lunaL_checktype(L, 2, LUNA_TFUNCTION);

    // Store the Lua function in a closure
    luna_pushvalue(L, 2);
    int luna_function_ref = lunaL_ref(L, LUNA_REGISTRYINDEX);

    // Create a lambda function that will call the Lua function when handling the request
    auto post_handler = [L, luna_function_ref](const char *data, size_t size, int client_socket) {
        // Retrieve the Lua function from the registry
        luna_rawgeti(L, LUNA_REGISTRYINDEX, luna_function_ref);

        // Push the necessary parameters onto the Lua stack
        luna_pushstring(L, data);
        luna_pushnumber(L, size);
        luna_pushnumber(L, client_socket);

        // Call the Lua function
        if (luna_pcall(L, 3, 0, 0) != 0) {
            // Handle Lua error if needed
            const char *error_message = luna_tostring(L, -1);
            fprintf(stderr, "Lua error: %s\n", error_message);
            luna_pop(L, 1); // Pop the error message from the stack
        }
    };

    // Store the URL and corresponding post handler
    url_to_post_handler_[url] = post_handler;
    return 0;
}

static int bind(luna_State *L) {
    const char *url = lunaL_checkstring(L, 1);
    const char *file_to_show = lunaL_checkstring(L, 2);
    url_to_file_[url] = file_to_show;
    return 0;
}

static int run(luna_State *L) {
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

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
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
    const char *response_template = "HTTP/1.1 200 OK\r\nContent-Length: %zu\r\n\r\n%s";
    const char *not_found_response = "HTTP/1.1 404 Not Found\r\n\r\nFile not found.";

    char buffer[1024];
    ssize_t bytesRead = read(client_socket, buffer, sizeof(buffer) - 1);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';


        const char *url_start = strstr(buffer, "GET ");
        if (url_start) {
            const char *url_end = strchr(url_start + 4, ' ');
            if (url_end) {
                string requested_url(url_start + 4, url_end);
                auto it = url_to_file_.find(requested_url);

                if (it != url_to_file_.end()) {
                    char *file_contents = _read_file(it->second.c_str());
                    if (file_contents) {
                        char response[1024];
                        snprintf(response, sizeof(response), response_template, strlen(file_contents), file_contents);
                        write(client_socket, response, strlen(response));
                        free(file_contents);
                    } else {
                        write(client_socket, not_found_response, strlen(not_found_response));
                    }
                } else {
                    write(client_socket, not_found_response, strlen(not_found_response));
                }
            }
        } else {

            const char *url_start = strstr(buffer, "POST ");
            if (url_start) {
                const char *url_end = strchr(url_start + 5, ' ');
                if (url_end) {
                    string requested_url(url_start + 5, url_end);
                    auto it = url_to_post_handler_.find(requested_url);
                    // Replace the problematic section in handle_client_request function
                    if (it != url_to_post_handler_.end()) {
                        // Extract POST data from the request
                        const char *data_start = strstr(buffer, "\r\n\r\n");
                        if (data_start) {
                            data_start += 4;  // Move past "\r\n\r\n"
                            size_t data_size = bytesRead - (data_start - buffer);

                            // Call the registered POST handler
                            it->second(data_start, data_size, client_socket);
                        }
                    } else {
                        // Handle case where no POST handler is registered for the URL
                        const char *not_found_response = "HTTP/1.1 404 Not Found\r\n\r\nNo POST handler registered.";
                        write(client_socket, not_found_response, strlen(not_found_response));
                    }

                }
            }
        }
        close(client_socket);
    }

    return 0;
}

static int cbind(luna_State *L) {
    if (!luna_istable(L, 1)) {
        lunaL_error(L, "Argument must be a table");
        return 0;
    }

    luna_pushnil(L);

    while (luna_next(L, 1) != 0) {
        const char *url = lunaL_checkstring(L, -2);
        const char *file = lunaL_checkstring(L, -1);

        url_to_file_[url] = file;

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

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
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

        if (handle_client_request(client_socket) != 0) {
            close(server_socket);
            close(client_socket);
            exit(EXIT_FAILURE);
        }

        close(client_socket);
    }

    return 0;
}

static int init_server(luna_State *L) {
    luna_newtable(L);

    luna_pushcfunction(L, bind);
    luna_setfield(L, -2, "bind");

    luna_pushcfunction(L, cbind);
    luna_setfield(L, -2, "cbind");

    luna_pushcfunction(L, run);
    luna_setfield(L, -2, "run");

    luna_pushcfunction(L, crun);
    luna_setfield(L, -2, "crun");

    luna_pushcfunction(L, handle_post);
    luna_setfield(L, -2, "handle_post");

    // Add the cwrite function
    luna_pushcfunction(L, cwrite);
    luna_setfield(L, -2, "cwrite");

    return 1;
}
