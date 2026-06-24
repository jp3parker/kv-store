
#pragma once

#include <fstream>
#include <filesystem>
#include <functional>

#include "wal_record.h"
#include "wal_parser.h"
#include "wal_serializer.h"

class WAL
{
public:
    explicit WAL(const std::string& filename);
    ~WAL();

    void append(WALRecord record);
    void write_record(std::ostream& out, WALRecord record);

    RecoveryResult recover(const std::function<void(const WALRecord&)>& callback);
    
    void rewrite(const std::function<void(std::ostream&)>& writer);

    void flush();

private:
    std::string filename_;
    std::filesystem::path temp_name_;

    std::fstream stream_;
    uint64_t next_lsn_ = 1;
};
