# win-route

A Windows command-line tool for managing IP routes with support for batch operations.

## Features

- Add routes using the system's default gateway
- Delete specific routes
- Reset routing table (preserving default routes)
- Batch operations support for better performance
- CIDR notation support for route definitions

## Usage

```bash
win-route add <file1.txt> [file2.txt ...] default   # Add routes from files using default gateway
win-route delete <file1.txt> [file2.txt ...]        # Delete routes from files
win-route reset                                     # Reset routing table
```

## Route File Format

```
1.0.1.0/24
1.0.2.0/23
1.0.8.0/21
...
```
