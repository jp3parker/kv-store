#include "kv_store.h"

KVStore::KVStore(const std::string& wal_file) : wal_file_(wal_file) {
    wal_.open(wal_file_, std::ios::binary | std::ios::app);
}

//void KVStore::put(const std::string& key, const std::string& value) {
//    wal_ << "PUT "
//         << key.size() << " "
//         << value.size() << " "
//         << key
//         << value
//         << "\n";
//         
//    wal_.flush();
//    map_[key] = value;
//}

void KVStore::put(const std::string& key, const std::string& value) {

    uint8_t op = OP_PUT;

    uint32_t key_size = static_cast<uint32_t>(key.size());
    uint32_t value_size = static_cast<uint32_t>(value.size());

    wal_.write(reinterpret_cast<char*>(&op), sizeof(op));
    wal_.write(reinterpret_cast<char*>(&key_size), sizeof(key_size));
    wal_.write(reinterpret_cast<char*>(&value_size), sizeof(value_size));

    wal_.write(key.data(), key.size());
    wal_.write(value.data(), value.size());
    
    wal_.flush();

    map_[key] = value;
}



std::string KVStore::get(const std::string& key) {
    if (map_.find(key) == map_.end()) {
        return "";
    }
    return map_[key];
}


//void KVStore::del(const std::string& key) {
//    wal_ << "DEL " << key.size() << " " << key << "\n";
//    wal_.flush();
//
//    map_.erase(key);
//}

void dump_wal_bytes(const std::string& filename)
{
    std::ifstream f(filename, std::ios::binary);

    unsigned char byte;

    while (f.read(reinterpret_cast<char*>(&byte), 1))
    {
        printf("%02X ", byte);
    }

    printf("\n");
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


//bool KVStore::recover() {
//
//    map_.clear();
//
//    std::ifstream wal_stream(wal_file_);
//    
//    std::string op;
//    bool recovery_ok = true;
//
//    while (recovery_ok && (wal_stream >> op)) {
//        recovery_ok = process_record(wal_stream, op);
//    }
//    
//    return recovery_ok;
//    
//}

bool KVStore::recover() {

    map_.clear();

    std::ifstream wal_stream(wal_file_, std::ios::binary);

    uint8_t op;
    bool recovery_ok = true;

    while (recovery_ok and wal_stream.read(reinterpret_cast<char*>(&op), sizeof(op))) {
        recovery_ok = process_record(wal_stream, op);
    }

    return recovery_ok;
}

//bool KVStore::process_record(std::ifstream& wal_stream, const std::string& op) {
//
//    bool valid_record = true;
//
//    if (op == "PUT") {
//        size_t ksz, vsz = 0;
//        wal_stream >> ksz >> vsz;
//  
//        if (ksz > DEFAULT_MAX_KEY_SIZE or vsz > DEFAULT_MAX_VALUE_SIZE) {
//            valid_record = false;
//            std::cout << "ERROR READING FROM WAL FILE - KEY SIZE OR VALUE SIZE TOO LARGE" << std::endl;
//        }
//        else {
//            std::string key(ksz, '\0');
//            std::string value(vsz, '\0');
//    
//            wal_stream.ignore(1);  // skip one character (space)
//            wal_stream.read(&key[0], ksz);
//            wal_stream.read(&value[0], vsz);
//    
//            if (wal_stream) {
//                map_[key] = value;
//            }
//            else {
//                valid_record = false;
//                std::cout << "ERROR READING FROM WAL FILE - CORRUPTION DETECTED" << std::endl;
//            }
//        }
//    
//    }
//    else if (op == "DEL") {
//        size_t ksz = 0;
//        wal_stream >> ksz;
//    
//        if (ksz > DEFAULT_MAX_KEY_SIZE) {
//            valid_record = false;
//            std::cout << "ERROR READING FROM WAL FILE - KEY SIZE TOO LARGE" << std::endl;
//        }
//        else {
//            std::string key(ksz, '\0');
//    
//            wal_stream.ignore(1);  // skip one character (space)
//            wal_stream.read(&key[0], ksz);
//    
//            if (wal_stream) {
//                map_.erase(key);
//            }
//            else {
//                valid_record = false;
//                std::cout << "ERROR READING FROM WAL FILE - CORRUPTION DETECTED" << std::endl;
//            }
//        }
//        
//    }
//    else {
//        valid_record = false;
//        std::cout << "ERROR READING FROM WAL FILE - UNRECOGNIZED OPERATION '" << op << "'" << std::endl;
//    }
//    
//    return valid_record;
//}

bool KVStore::process_record(std::ifstream& wal_stream, uint8_t op) {
    bool valid_record = true;
    
    if (op == OP_PUT) {
        uint32_t ksz;
        uint32_t vsz;

        wal_stream.read(reinterpret_cast<char*>(&ksz), sizeof(ksz));
        wal_stream.read(reinterpret_cast<char*>(&vsz), sizeof(vsz));

        if (!wal_stream or ksz > DEFAULT_MAX_KEY_SIZE or vsz > DEFAULT_MAX_VALUE_SIZE) {
            valid_record = false;
        }
        else {
            std::string key(ksz, '\0');
            std::string value(vsz, '\0');

            wal_stream.read(&key[0], ksz);
            wal_stream.read(&value[0], vsz);
            
            if (!wal_stream) {
                valid_record = false;
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
            valid_record = false;
        }
        else {
            std::string key(ksz, '\0');

            wal_stream.read(&key[0], ksz);

            if (!wal_stream) {
                valid_record = false;
            }
            else {
                map_.erase(key);
            }
        }
    }
    else { // UNRECOGNIZED OP
        valid_record = false;
    }
    
    return valid_record;
}

