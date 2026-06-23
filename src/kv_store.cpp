#include "kv_store.h"

KVStore::KVStore(const std::string& wal_file) : wal_file_(wal_file) {
    std::filesystem::create_directories(std::filesystem::path(wal_file).parent_path());
    tmp_name = std::filesystem::path(wal_file_).parent_path() / "temp-wal.log";
    wal_.open(wal_file_, std::ios::binary | std::ios::app);
}

void KVStore::put(const std::string& key, const std::string& value) {
    WALSerializer::write_record(wal_, WALRecord::Put(key, value));
    wal_.flush();
    map_[key] = value;
}

void KVStore::del(const std::string& key) {
    WALSerializer::write_record(wal_, WALRecord::Delete(key));
    wal_.flush();
    map_.erase(key);

}

std::string KVStore::get(const std::string& key) {
    auto it = map_.find(key);
    if (it == map_.end()) {
        return "";
    }
    return it->second;
}

RecoveryResult KVStore::recover() {
    map_.clear();
    std::ifstream wal_stream(wal_file_, std::ios::binary);
    WALRecord record;
    while (true) {
        RecoveryResult result = WALParser::read_record(wal_stream, record);

        // EOF
        if (result == RecoveryResult::Success &&
            wal_stream.eof())
            return RecoveryResult::Success;

        if (result != RecoveryResult::Success)
            return result;

        apply_record(record);
    }
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
    std::ofstream temp(tmp_name, std::ios::binary);
    for (const auto& [k,v] : map_) {
        WALSerializer::write_record(temp, WALRecord::Put(k, v));
    }
    temp.close();
    wal_.close();
    std::remove(wal_file_.c_str());
    std::rename(tmp_name.c_str(), wal_file_.c_str());
    wal_.open(wal_file_, std::ios::binary | std::ios::app);
}

void dump_wal_bytes(const std::string& filename) {
    std::ifstream f(filename, std::ios::binary);
    unsigned char byte;
    while (f.read(reinterpret_cast<char*>(&byte), 1)) {
        printf("%02X ", byte);
    }
    printf("\n");
}
