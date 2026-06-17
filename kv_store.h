#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>

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
    
    bool process_record(std::ifstream& wal_stream, const std::string& op);
};
