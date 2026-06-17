#include "kv_store.h"

KVStore::KVStore(const std::string& wal_file) : wal_file_(wal_file) {
    wal_.open(wal_file_, std::ios::app);
}

void KVStore::put(const std::string& key, const std::string& value) {
    wal_ << "PUT "
         << key.size() << " "
         << value.size() << " "
         << key
         << value
         << "\n";
         
    wal_.flush();
    map_[key] = value;
}

std::string KVStore::get(const std::string& key) {
    if (map_.find(key) == map_.end()) {
        return "";
    }
    return map_[key];
}


void KVStore::del(const std::string& key) {
    wal_ << "DEL " << key.size() << " " << key << "\n";
    wal_.flush();

    map_.erase(key);
}


bool KVStore::recover() {

    map_.clear();

    std::ifstream wal_stream(wal_file_);
    
    std::string op;
    bool recovery_ok = true;

    while (recovery_ok && (wal_stream >> op)) {
        recovery_ok = process_record(wal_stream, op);
    }
    
    return recovery_ok;
    
}


bool KVStore::process_record(std::ifstream& wal_stream, const std::string& op) {

    bool valid_record = true;

    if (op == "PUT") {
        size_t ksz, vsz = 0;
        wal_stream >> ksz >> vsz;
    
        std::string key(ksz, '\0');
        std::string value(vsz, '\0');
    
        wal_stream.ignore(1);  // skip one character (space)
        wal_stream.read(&key[0], ksz);
        wal_stream.read(&value[0], vsz);
    
        if (wal_stream) {
            map_[key] = value;
        }
        else {
            valid_record = false;
            std::cout << "ERROR READING FROM WAL FILE - CORRUPTION DETECTED" << std::endl;
        }
    }
    else if (op == "DEL") {
        size_t ksz = 0;
        wal_stream >> ksz;
    
        std::string key(ksz, '\0');
    
        wal_stream.ignore(1);  // skip one character (space)
        wal_stream.read(&key[0], ksz);
    
        if (wal_stream) {
            map_.erase(key);
        }
        else {
            valid_record = false;
            std::cout << "ERROR READING FROM WAL FILE - CORRUPTION DETECTED" << std::endl;
        }
    }
    else {
        valid_record = false;
        std::cout << "ERROR READING FROM WAL FILE - UNRECOGNIZED OPERATION '" << op << "'" << std::endl;
    }
    
    return valid_record;
}

