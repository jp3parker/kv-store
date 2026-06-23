
#include "wal_record.h"

WALRecord WALRecord::Put(std::string key, std::string value) {
    WALRecord record{
        .op = WALRecord::Operation::Put,
        .key = key,
        .value = value
    };
    return record;
}


WALRecord WALRecord::Delete(std::string key) {
    WALRecord record{
        .op = WALRecord::Operation::Delete,
        .key = key
    };
    return record;
}


