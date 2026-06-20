
// wal_serializer.cpp

#include "wal_serializer.h"

#include <cstdint>

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
    uint8_t op = static_cast<uint8_t>(record.op);

    uint32_t key_size = static_cast<uint32_t>(record.key.size());
    uint32_t value_size = static_cast<uint32_t>(record.value.size());

    out.write(reinterpret_cast<char*>(&op), sizeof(op));
    out.write(reinterpret_cast<char*>(&key_size), sizeof(key_size));
    out.write(reinterpret_cast<char*>(&value_size), sizeof(value_size));

    out.write(record.key.data(), record.key.size());
    out.write(record.value.data(), record.value.size());
}

void WALSerializer::write_delete(std::ostream& out, const WALRecord& record)
{
    uint8_t op = static_cast<uint8_t>(record.op);

    uint32_t key_size = static_cast<uint32_t>(record.key.size());

    out.write(reinterpret_cast<char*>(&op), sizeof(op));
    out.write(reinterpret_cast<char*>(&key_size), sizeof(key_size));

    out.write(record.key.data(), record.key.size());
}
