
# KV Store Design

> **Note:** This document describes the architecture and design philosophy of the project rather than every implementation detail. As the codebase evolves, implementation details may change while the architectural goals and system invariants described here should remain largely consistent.

---

# Project Overview

This project is an educational implementation of a durable key-value store written in modern C++. The primary objective is to explore storage engine design through incremental development of realistic database components.

The project intentionally favors correctness, modularity, and readability over maximum performance. Features are added progressively so that each subsystem can be understood independently before introducing additional complexity.

Current areas of focus include:

* Durable writes
* Write-ahead logging (WAL)
* Crash recovery
* Data integrity
* Log compaction
* Modular serialization
* Comprehensive testing

---

# Design Philosophy

Several principles guide the implementation.

## Correctness before optimization

Features are first implemented in the simplest correct manner. Performance optimizations are introduced only after correctness has been established and verified through testing.

## Small, focused components

Responsibilities are separated into independent modules whenever practical.

For example:

* WAL management
* Record serialization
* Record parsing
* Recovery
* KVStore interface

This reduces coupling and allows each component to evolve independently.

## Explicit failure handling

Failure cases are represented explicitly through return values rather than hidden behavior whenever possible.

Examples include:

* Truncated WAL records
* Invalid operations
* Checksum failures
* Read errors

---

# High-Level Architecture

The storage engine currently consists of two major layers.

```
Application
      │
      ▼
KVStore
      │
      ▼
Write-Ahead Log (WAL)
      │
      ├── Serializer
      ├── Parser
      └── File Storage
```

The in-memory map represents the current state of the database.

The WAL represents the durable history required to reconstruct that state after a crash.

---

# Write Path

A write operation follows this general flow:

```
Client

    │

PUT / DELETE

    │

Append WAL record

    │

Flush WAL

    │

Update in-memory state

    │

Return
```

This ordering ensures that committed operations are durable before becoming visible in memory.

---

# Recovery

Recovery reconstructs the in-memory state by replaying WAL records in the order they were originally written.

During recovery:

* Records are read sequentially.
* Checksums are validated.
* Successfully parsed records are applied to the in-memory state.
* Recovery stops if corruption or truncation is encountered.

Future versions may introduce snapshotting or checkpointing to reduce recovery time.

---

# Write-Ahead Log

The WAL is append-only during normal operation.

Each record contains sufficient information to reproduce a single logical operation.

Current record metadata includes:

* Log Sequence Number (LSN)
* Operation type
* Key
* Value (for PUT operations)
* CRC32 checksum

The WAL is considered the authoritative durable representation of recent changes.

---

# Log Sequence Numbers (LSNs)

Each WAL record receives a monotonically increasing Log Sequence Number.

LSNs uniquely identify the order in which operations were written to the log.

Current uses include:

* Maintaining write ordering
* Recovery bookkeeping
* Providing a foundation for future checkpointing and snapshot support

Additional uses may be introduced as the storage engine evolves.

---

# Checksums

Every WAL record includes a CRC32 checksum.

Checksums are computed over the serialized payload and verified during recovery.

Their purpose is to detect:

* Partial writes
* Corrupted records
* Unexpected modification of WAL contents

The current implementation treats checksum failures as recovery errors.

---

# Compaction

As updates accumulate, the WAL contains obsolete historical operations.

Compaction rewrites the log so that it represents only the current logical database state.

The primary goals are:

* Reduce recovery time
* Reduce log size
* Remove obsolete history

Compaction is considered a maintenance operation and does not change the logical contents of the database.

---

# Current System Policies

The following behaviors are intentional design decisions.

## Durability

* Successful writes are appended to the WAL before being applied to the in-memory state.
* WAL writes are flushed before operations return successfully.

## Recovery

* Recovery processes records in WAL order.
* Recovery never applies partially read records.
* Recovery stops when corruption or truncation is detected.

## Consistency

* The most recent operation for a key determines its final value.
* Delete operations are idempotent.
* The WAL is append-only except during compaction.

## Integrity

* Every record is protected by a checksum.
* Invalid record formats are treated as recovery errors.

---

# Testing Philosophy

The project emphasizes automated testing of both normal operation and failure scenarios.

Tests should verify:

* Correct database behavior
* WAL serialization
* WAL parsing
* Recovery correctness
* Corruption detection
* Edge cases
* Regression prevention

As the project grows, tests should remain organized by subsystem rather than accumulating into a single large file.

---

# Non-Goals

This project intentionally does not currently implement:

* Concurrent writers
* Transactions
* MVCC
* Replication
* Distributed consensus
* Compression
* Encryption
* Background compaction
* Page cache
* B-Tree or LSM-based storage

These features are intentionally deferred so that the durability and recovery infrastructure can mature first.

---

# Future Directions

Potential future work includes:

* Snapshot/checkpoint files
* Faster recovery
* Configurable durability policies
* Storage statistics
* Versioned WAL formats
* Atomic compaction
* Multiple column families
* Background maintenance
* Transaction support
* Alternative storage engines

The project is intended to evolve incrementally, with each feature building on previously established infrastructure.
