#include "kv_store.h"

KVStore::KVStore(const std::string& wal_file) : wal_file_(wal_file) {
    std::filesystem::create_directories(std::filesystem::path(wal_file).parent_path());
    tmp_name = std::filesystem::path(wal_file_).parent_path() / "temp-wal.log";
    wal_.open(wal_file_, std::ios::binary | std::ios::app);
}

void KVStore::put(const std::string& key, const std::string& value) {

    write_put_record(wal_, key, value);
    wal_.flush();

    map_[key] = value;
}

void KVStore::write_put_record(std::ostream& out, const std::string& key, const std::string& value) {
    uint8_t op = OP_PUT;

    uint32_t key_size = static_cast<uint32_t>(key.size());
    uint32_t value_size = static_cast<uint32_t>(value.size());

    out.write(reinterpret_cast<char*>(&op), sizeof(op));
    out.write(reinterpret_cast<char*>(&key_size), sizeof(key_size));
    out.write(reinterpret_cast<char*>(&value_size), sizeof(value_size));

    out.write(key.data(), key.size());
    out.write(value.data(), value.size());

}

std::string KVStore::get(const std::string& key) {
    if (map_.find(key) == map_.end()) {
        return "";
    }
    return map_[key];
}

void KVStore::del(const std::string& key) {

    uint8_t op = OP_DEL;
    
    uint32_t key_size = static_cast<uint32_t>(key.size());
    
    wal_.write(reinterpret_cast<char*>(&op), sizeof(op));
    wal_.write(reinterpret_cast<char*>(&key_size), sizeof(key_size));
    
    wal_.write(key.data(), key.size());
    wal_.flush();
    
    map_.erase(key);
}

RecoveryResult KVStore::recover() {

    map_.clear();

    std::ifstream wal_stream(wal_file_, std::ios::binary);

    uint8_t op;
    RecoveryResult recovery_ok = RecoveryResult::Success;

    while (wal_stream.read(reinterpret_cast<char*>(&op), sizeof(op)) and
           recovery_ok == RecoveryResult::Success) {
        recovery_ok = process_record(wal_stream, op);
    }

    return recovery_ok;
}

RecoveryResult KVStore::process_record(std::ifstream& wal_stream, uint8_t op) {
    RecoveryResult valid_record = RecoveryResult::Success;
    
    if (op == OP_PUT) {
        uint32_t ksz;
        uint32_t vsz;

        wal_stream.read(reinterpret_cast<char*>(&ksz), sizeof(ksz));
        wal_stream.read(reinterpret_cast<char*>(&vsz), sizeof(vsz));

        if (!wal_stream or ksz > DEFAULT_MAX_KEY_SIZE or vsz > DEFAULT_MAX_VALUE_SIZE) {
            valid_record = RecoveryResult::ReadError;
        }
        else {
            std::string key(ksz, '\0');
            std::string value(vsz, '\0');

            wal_stream.read(&key[0], ksz);
            wal_stream.read(&value[0], vsz);
            
            if (!wal_stream) {
                valid_record = RecoveryResult::Truncated;
            }
            else {
                map_[key] = value;
            }
        }
    }
    else if (op == OP_DEL) {
        uint32_t ksz;

        wal_stream.read(reinterpret_cast<char*>(&ksz), sizeof(ksz));

        if (!wal_stream or ksz > DEFAULT_MAX_KEY_SIZE) {
            valid_record = RecoveryResult::ReadError;
        }
        else {
            std::string key(ksz, '\0');
            wal_stream.read(&key[0], ksz);
            
            if (!wal_stream) {
                valid_record = RecoveryResult::Truncated;
            }
            else {
                map_.erase(key);
            }
        }
    }
    else { // UNRECOGNIZED OP
        valid_record = RecoveryResult::InvalidOperation;
    }
    return valid_record;
}

void KVStore::compact() {
    std::ofstream temp(tmp_name, std::ios::binary);
    for (auto& [k,v] : map_) {
        write_put_record(temp, k, v);
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
