
// wal_parser.cpp

#include "wal_parser.h"

template <typename T>
void append_bytes(std::vector<char>& buffer, const T& value)
{
    const char* p = reinterpret_cast<const char*>(&value);
    buffer.insert(buffer.end(), p, p + sizeof(T));
}


RecoveryResult WALParser::read_record(std::istream& in, WALRecord& record)
{

    uint32_t stored_checksum;
    if (!in.read(reinterpret_cast<char*>(&stored_checksum), sizeof(stored_checksum))) {
        if (in.gcount() == 0) {
            return RecoveryResult::Success;
        }
        return RecoveryResult::Truncated;
    }
    
    uint64_t lsn;
    if (!in.read(reinterpret_cast<char*>(&lsn), sizeof(lsn))) {
        return RecoveryResult::Truncated;
    }

    uint8_t op;
    if (!in.read(reinterpret_cast<char*>(&op), sizeof(op))) {
        return RecoveryResult::Truncated;
    }
    
    std::vector<char> payload;

    switch (static_cast<WALRecord::Operation>(op)) {

        case WALRecord::Operation::Put: {
    
          uint32_t ksz;
          uint32_t vsz;
        
          in.read(reinterpret_cast<char*>(&ksz), sizeof(ksz));
          in.read(reinterpret_cast<char*>(&vsz), sizeof(vsz));

          if (!in) {
              return RecoveryResult::ReadError;
          }

          if (ksz > DEFAULT_MAX_KEY_SIZE ||
              vsz > DEFAULT_MAX_VALUE_SIZE) {
              return RecoveryResult::ReadError;
          }
          
          record.lsn = lsn;
          record.op = WALRecord::Operation::Put;
          record.key.resize(ksz);
          record.value.resize(vsz);
          
          in.read(record.key.data(), ksz);
          in.read(record.value.data(), vsz);
          
          if (!in) {
              return RecoveryResult::Truncated;
          }
        
          append_bytes(payload, lsn);
          append_bytes(payload, op);
          append_bytes(payload, ksz);
          append_bytes(payload, vsz);
        
          payload.insert(payload.end(),
                record.key.begin(),
                record.key.end());
               
          payload.insert(payload.end(),
                record.value.begin(),
                record.value.end());
                
          break;

    }

    case WALRecord::Operation::Delete:
    {
        uint32_t ksz;

        in.read(reinterpret_cast<char*>(&ksz), sizeof(ksz));

        if (!in) {
            return RecoveryResult::ReadError;
        }

        if (ksz > DEFAULT_MAX_KEY_SIZE) {
            return RecoveryResult::ReadError;
        }

        record.lsn = lsn;
        record.op = WALRecord::Operation::Delete;
        record.key.resize(ksz);
        record.value.clear();

        in.read(record.key.data(), ksz);

        if (!in) {
            return RecoveryResult::Truncated;
        }
        
        append_bytes(payload, lsn);
        append_bytes(payload, op);
        append_bytes(payload, ksz);
        
        payload.insert(payload.end(),
              record.key.begin(),
              record.key.end());

        break;
    }

    default:
        return RecoveryResult::InvalidOperation;
    
    }
    
    uint32_t computed = crc32(payload.data(), payload.size());
    if (computed != stored_checksum) {
        return RecoveryResult::ChecksumMismatch;
    }

    return RecoveryResult::Success;
    
}
