
#include "wal_record.h"

WALRecord WALRecord::Put(std::string key, std::string value) {
    WALRecord record{
        WALRecord::Operation::Put,
        key,
        value
    };
    return record;
}


WALRecord WALRecord::Delete(std::string key) {
    WALRecord record{
        WALRecord::Operation::Delete,
        key
    };
    return record;
}


