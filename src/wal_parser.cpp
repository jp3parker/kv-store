
// wal_parser.cpp

#include "wal_parser.h"

RecoveryResult WALParser::read_record(std::istream& in, WALRecord& record)
{
    uint8_t op;

    if (!in.read(reinterpret_cast<char*>(&op), sizeof(op))) {
        return RecoveryResult::Success;
    }

    switch (static_cast<WALRecord::Operation>(op)) {

    case WALRecord::Operation::Put:
    {
        uint32_t ksz;
        uint32_t vsz;

        in.read(reinterpret_cast<char*>(&ksz), sizeof(ksz));
        in.read(reinterpret_cast<char*>(&vsz), sizeof(vsz));

        if (!in)
            return RecoveryResult::ReadError;

        if (ksz > DEFAULT_MAX_KEY_SIZE ||
            vsz > DEFAULT_MAX_VALUE_SIZE)
            return RecoveryResult::ReadError;

        record.op = WALRecord::Operation::Put;
        record.key.resize(ksz);
        record.value.resize(vsz);

        in.read(record.key.data(), ksz);
        in.read(record.value.data(), vsz);

        if (!in)
            return RecoveryResult::Truncated;

        return RecoveryResult::Success;
    }

    case WALRecord::Operation::Delete:
    {
        uint32_t ksz;

        in.read(reinterpret_cast<char*>(&ksz), sizeof(ksz));

        if (!in)
            return RecoveryResult::ReadError;

        if (ksz > DEFAULT_MAX_KEY_SIZE)
            return RecoveryResult::ReadError;

        record.op = WALRecord::Operation::Delete;
        record.key.resize(ksz);
        record.value.clear();

        in.read(record.key.data(), ksz);

        if (!in)
            return RecoveryResult::Truncated;

        return RecoveryResult::Success;
    }

    default:
        return RecoveryResult::InvalidOperation;
    }
}
