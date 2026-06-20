# KV Store

A lightweight key-value store written in modern C++ with an append-only Write-Ahead Log (WAL), crash recovery, log compaction, and comprehensive unit testing.

This project is being built as an educational exploration of storage engine design and the techniques used by databases such as LevelDB, RocksDB, and SQLite.

## Features

* In-memory hash table for fast lookups
* Append-only binary Write-Ahead Log (WAL)
* Crash recovery by replaying the WAL
* Log compaction to eliminate obsolete records
* Binary-safe keys and values
* Validation of malformed WAL records during recovery
* Extensive unit and fuzz testing using GoogleTest

## Project Structure

```
.
├── data/                  # WAL files
├── kv_store.cpp
├── kv_store.h             # Public KVStore interface
├── wal_record.h           # WAL record representation
├── wal_serializer.cpp/.h  # Serialize records to binary format
├── wal_parser.cpp/.h      # Parse binary WAL records
├── status.h               # Recovery status codes
├── test_kv_store.cpp      # Unit and fuzz tests
└── README.md
```

## WAL Format

Each operation is appended to the write-ahead log before the in-memory state is modified.

### Put Record

```
+-----------+------------+--------------+---------+-----------+
| Operation | Key Length | Value Length |   Key   |   Value   |
| 1 byte    | 4 bytes    | 4 bytes      | N bytes | M bytes   |
+-----------+------------+--------------+---------+-----------+
```

### Delete Record

```
+-----------+------------+---------+
| Operation | Key Length |   Key   |
| 1 byte    | 4 bytes    | N bytes |
+-----------+------------+---------+
```

The log is binary encoded, allowing arbitrary byte sequences to be stored as keys and values.

## Recovery

On startup, the WAL is replayed from beginning to end.

Each record is parsed and applied to reconstruct the in-memory hash table.

During recovery the parser detects:

* Invalid operation codes
* Invalid key sizes
* Invalid value sizes
* Truncated records
* Read failures

Recovery reports an appropriate status if corruption is encountered.

## Compaction

Since the WAL is append-only, overwritten and deleted entries accumulate over time.

Calling `compact()` writes the current contents of the hash table into a new WAL and replaces the old log, reducing disk usage and recovery time.

## Testing

The project currently includes tests for:

* Basic put/get/delete operations
* Overwriting existing keys
* Empty keys and values
* Binary data
* Large values
* Recovery after restart
* Recovery after deletes
* Recovery after overwrites
* WAL compaction
* Randomized stress tests
* Fuzz testing with randomly generated binary data

## Current Architecture

```
          +-------------+
          |   KVStore   |
          +-------------+
          | put/get/del |
          | recover()   |
          +------+------+
                 |
        WALRecord objects
                 |
      +----------+----------+
      |                     |
+-------------+     +---------------+
| Serializer  |     |    Parser     |
+-------------+     +---------------+
| record ->   |     | bytes ->      |
| binary      |     | WALRecord     |
+-------------+     +---------------+
```

The storage engine is organized into three primary components:

* **KVStore** manages the in-memory state and public API.
* **WALSerializer** converts records into binary log entries.
* **WALParser** reconstructs records from the binary log.

This separation keeps serialization logic independent from storage logic and makes future format changes easier to implement.

## Future Work

Planned improvements include:

* Record checksums for corruption detection
* Stronger crash consistency guarantees
* Snapshot support
* Configurable flushing and durability policies
* More efficient compaction
* Performance benchmarking
* Concurrent reads and writes

## Building

The project uses CMake and GoogleTest.

```bash
mkdir build
cd build
cmake ..
cmake --build .
ctest
```

## Purpose

This project is intended as a learning exercise in storage engine internals rather than a production-ready database.

The goal is to progressively implement features commonly found in real-world database systems while keeping the codebase small, readable, and well-tested.
