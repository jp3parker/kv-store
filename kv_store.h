#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>
#include <cstdint>

class KVStore {
public:
    KVStore(const std::string& wal_file);

    void put(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
    void del(const std::string& key);
    
    bool recover();
private:
    std::unordered_map<std::string, std::string> map_;
    std::ofstream wal_;
    std::string wal_file_;
    
    //bool process_record(std::ifstream& wal_stream, const std::string& op);
    bool process_record(std::ifstream& wal_stream, uint8_t op);
    
    static constexpr size_t DEFAULT_MAX_KEY_SIZE = 1024;          // 1 KB
    static constexpr size_t DEFAULT_MAX_VALUE_SIZE = 16 * 1024 * 1024; // 16 MB
    
    static constexpr uint8_t OP_PUT = 1;
    static constexpr uint8_t OP_DEL = 2;
};


void dump_wal_bytes(const std::string& filename);
