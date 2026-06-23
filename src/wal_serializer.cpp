
// wal_serializer.cpp

#include "wal_serializer.h"

template <typename T>
void append_bytes(std::vector<char>& buffer, const T& value)
{
    const char* p = reinterpret_cast<const char*>(&value);
    buffer.insert(buffer.end(), p, p + sizeof(T));
}


void WALSerializer::write_record(std::ostream& out, const WALRecord& record)
{
    switch (record.op) {
    case WALRecord::Operation::Put:
        write_put(out, record);
        break;

    case WALRecord::Operation::Delete:
        write_delete(out, record);
        break;
    }
}


void WALSerializer::write_put(std::ostream& out, const WALRecord& record)
{
    std::vector<char> payload;

    uint8_t op = static_cast<uint8_t>(record.op);
    uint32_t key_size = static_cast<uint32_t>(record.key.size());
    uint32_t value_size = static_cast<uint32_t>(record.value.size());

    // Build the payload.
    append_bytes(payload, op);
    append_bytes(payload, key_size);
    append_bytes(payload, value_size);

    payload.insert(payload.end(),
                   record.key.begin(),
                   record.key.end());

    payload.insert(payload.end(),
                   record.value.begin(),
                   record.value.end());

    // Compute checksum over the payload.
    uint32_t checksum = crc32(payload.data(), payload.size());

    // Write checksum first.
    out.write(reinterpret_cast<char*>(&checksum), sizeof(checksum));

    // Then write the payload.
    out.write(payload.data(), payload.size());
}

void WALSerializer::write_delete(std::ostream& out, const WALRecord& record)
{
    std::vector<char> payload;

    uint8_t op = static_cast<uint8_t>(record.op);
    uint32_t key_size = static_cast<uint32_t>(record.key.size());

    append_bytes(payload, op);
    append_bytes(payload, key_size);

    payload.insert(payload.end(),
                   record.key.begin(),
                   record.key.end());

    uint32_t checksum = crc32(payload.data(), payload.size());

    out.write(reinterpret_cast<char*>(&checksum), sizeof(checksum));
    out.write(payload.data(), payload.size());
}
