# WAL Format

This document describes the current on-disk Write-Ahead Log format used by `KVStore`.
It is meant to be the stable reference for serialization, parsing, recovery, and future
format evolution.

## Goals

- Append records sequentially.
- Detect corruption early with a checksum.
- Support binary keys and values.
- Keep recovery simple and deterministic.
- Leave room for future versioning and snapshots.

## Record Framing

Each WAL record is stored as:

```text
[checksum:u32][payload bytes...]
```

The checksum covers the payload only.
The checksum field itself is not included in the CRC calculation.

## Payload Layout

The payload is:

```text
[lsn:u64][op:u8][op-specific fields...]
```

Where:

- `lsn` is a monotonically increasing log sequence number.
- `op` identifies the logical operation.
- The remaining fields depend on the operation type.

## Operation Codes

| Opcode | Meaning |
| --- | --- |
| `1` | Put |
| `2` | Delete |

## Put Record

A `Put` record stores a key/value pair:

```text
[checksum:u32]
[lsn:u64]
[op:u8 = 1]
[key_size:u32]
[value_size:u32]
[key bytes]
[value bytes]
```

Both key and value may be empty.
Both key and value may contain arbitrary binary data, including `NUL` bytes.

## Delete Record

A `Delete` record stores only the key being removed:

```text
[checksum:u32]
[lsn:u64]
[op:u8 = 2]
[key_size:u32]
[key bytes]
```

Deletes are logically idempotent.
Deleting a key that does not exist is still represented as a normal WAL record.

## Integer Encoding

The current implementation writes multi-byte integers by copying their in-memory
representation directly into the log.

That means the current format is tied to the machine endianness used when the log is
written. On the current development target, that is little-endian. If you later want the
log to be portable across architectures, the next format revision should make the byte
order explicit.

## Parser Limits

The parser currently enforces these limits during recovery:

- Maximum key size: `1024` bytes
- Maximum value size: `16 MiB`

These are safety limits, not part of the logical key/value API.
They exist to keep corrupted records from allocating unreasonable amounts of memory.

## Recovery Rules

Recovery reads records in order and applies them to the in-memory map.

The parser reports:

- `Success` when a clean end-of-file is reached after the last full record
- `Truncated` when a record ends partway through the log
- `ChecksumMismatch` when the payload bytes do not match the stored CRC32
- `InvalidOperation` when the opcode is not recognized
- `ReadError` when a field violates parser limits or cannot be read safely

Recovery should stop at the first non-success result.

## Compaction Behavior

Compaction rewrites the current live key/value set into a new WAL file.
Only the latest visible value for each key is written back.

This removes historical overwrite and delete records while preserving the current logical
database state.

## Compatibility Notes

If this format changes in a breaking way, the log should gain an explicit version marker
or header so future recovery code can detect and migrate old files safely.

For now, this document defines the contract that the current code is expected to follow.
