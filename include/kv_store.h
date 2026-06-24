#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <cstdint>
#include "status.h"
#include "wal_record.h"
#include "wal_serializer.h"
#include "wal_parser.h"
#include "wal.h"

class KVStore {
public:
    KVStore(const std::string& wal_file);

    void put(const std::string& key, const std::string& value);
    void del(const std::string& key);
    std::optional<std::string> get(const std::string&) const;
    
    RecoveryResult recover();
    void compact();
    
private:
    std::unordered_map<std::string, std::string> map_;
    WAL wal_;
    
    void apply_record(const WALRecord& record);
    
};

void dump_wal_bytes(const std::string& filename);
