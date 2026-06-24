#include "kv_store.h"

KVStore::KVStore(const std::string& wal_file) : wal_(wal_file) { }

void KVStore::put(const std::string& key, const std::string& value) {
    wal_.append(WALRecord::Put(key, value));
    wal_.flush();
    map_[key] = value;
}

void KVStore::del(const std::string& key) {
    wal_.append(WALRecord::Delete(key));
    wal_.flush();
    map_.erase(key);
}

std::optional<std::string> KVStore::get(const std::string& key) const {
    auto it = map_.find(key);
    if (it == map_.end()) {
        return std::nullopt;
    }
    return it->second;
}

RecoveryResult KVStore::recover() {
    map_.clear();
    return wal_.recover(
        [this](const WALRecord& record)
        {
            apply_record(record);
        });
}

void KVStore::apply_record(const WALRecord& record) {
    switch (record.op) {
        case WALRecord::Operation::Put: {
            map_[record.key] = record.value;
            break;
        }
        case WALRecord::Operation::Delete:
            map_.erase(record.key);
            break;
        }
}

void KVStore::compact() {
    wal_.rewrite(
        [this](std::ostream& out)
        {
            for (const auto& [k, v] : map_)
            {
                wal_.write_record(out, WALRecord::Put(k, v));
            }
        });
}

void dump_wal_bytes(const std::string& filename) {
    std::ifstream f(filename, std::ios::binary);
    unsigned char byte;
    while (f.read(reinterpret_cast<char*>(&byte), 1)) {
        printf("%02X ", byte);
    }
    printf("\n");
}
