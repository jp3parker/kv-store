#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>
#include <cstdint>
#include "status.h"
#include "wal_record.h"
#include "wal_serializer.h"
#include "wal_parser.h"

class KVStore {
public:
    KVStore(const std::string& wal_file);

    void put(const std::string& key, const std::string& value);
    void del(const std::string& key);
    std::string get(const std::string& key);
    
    RecoveryResult recover();
    void compact();
    
private:
    std::unordered_map<std::string, std::string> map_;
    std::ofstream wal_;
    std::string wal_file_;
    std::string tmp_name;
    void apply_record(const WALRecord& record);
    
};


void dump_wal_bytes(const std::string& filename);
