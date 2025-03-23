# win-route

A Windows command-line tool for managing IP routes with support for batch operations.

## Features

- Add routes using the system's default gateway
- Delete specific routes
- Reset routing table (preserving default routes)
- Batch operations support for better performance
- CIDR notation support for route definitions

## Usage

```powershell
win-route add <file1.txt> [file2.txt ...] default   # Add routes from files using default gateway
win-route delete <file1.txt> [file2.txt ...]        # Delete routes from files
win-route reset                                     # Reset routing table
```

## Route File Format

```plaintext
1.0.1.0/24
1.0.2.0/23
1.0.8.0/21
...
```

## Example

### Add routing table via default gateway

```powershell
.\win-route.exe add .\custom.txt .\chnroute.txt default
```

### Remove routing table

```powershell
.\win-route.exe delete .\custom.txt .\chnroute.txt
```

### Reset by default

```powershell
.\win-route.exe reset
```

### Compile

```powershell
g++ main.cpp route_operations.cpp network_utils.cpp file_operations.cpp -o win-route.exe -liphlpapi -lws2_32
```
