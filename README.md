
# webserver_42

A HTTP server implemented in C++ 98.
## Usage
```Bash
./webserv [configuration file]
```

## Features
- Takes a configuration file as argument, or use a default path.
- Non-blocking and uses  `select()` for all the I/O operations between the client and the server (listening included).
- `select()` checks read and write at the same time.
- No read or write operation without going through `select()`.
- Compatible with web browsers.
- Accurate HTTP response status codes.
- Default error pages if none provided.
- Serve a fully static website.
- Clients can upload files.
- Supports GET, POST, and DELETE methods.
- Stress-tested and stays available at all costs.
- Able to listen to multiple ports (configured in the configuration file).

## Configuration file
- Choose the port and host of each server.
- Setup server names or not.
- First server for a host:port is the default for that host:port.
- Setup default error pages.
- Limit client body size.
- Setup routes with rules/configuration:
  - List of accepted HTTP methods for the route.
  - HTTP redirection.
  - Turn directory listing on or off.
  - Default file to answer if request is a directory.
  - Execute CGI based on file extension (for example `.php`).
  - Route able to accept uploaded files and configure where to save them.
 ## Config file example
```
server {
    listen 4040;
    server_name localhost;
    root var/localhost/files;
    limit_except GET;

    Location /uploads/ {
        client_max_body_size 100b;
        root var/localhost;
        autoindex on;
    }

    Location /cgi-bin/ {
        limit_except GET;
        error_pages 404 error_pages/4040.html
        autoindex on;
    }

    Location /developer_files/ {
        return 403 error_pages/403.html;
    }

    Location / {
        limit_except GET;
        root var/localhost/files;
        index lol.html;
    }

    Location /error_pages/ {
        limit_except GET;
        autoindex on;
    }
}

## CGI
- The server calls the CGI with the requested file as the first argument.
- The CGI is run in the correct directory for relative path file access.

## Extra Features
- Handle multiple CGI.

