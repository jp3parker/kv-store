
#include "wal.h"

WAL::WAL(const std::string& filename) : filename_(filename) {
    std::filesystem::create_directories(std::filesystem::path(filename).parent_path());
    temp_name_ = std::filesystem::path(filename).parent_path() / "temp-wal.log";
    stream_.open(filename_, std::ios::binary | std::ios::app);
}

void WAL::append(WALRecord record) { //might have to change to pass by value
    record.lsn = next_lsn_++;
    WALSerializer::write_record(stream_, record);
}

RecoveryResult WAL::recover(const std::function<void(const WALRecord&)>& callback) {
    std::ifstream in(filename_, std::ios::binary);
    WALRecord record;
    while (true) {
        RecoveryResult result = WALParser::read_record(in, record);
        
        if (result == RecoveryResult::Success and in.eof()) {
            return RecoveryResult::Success;
        }
        if (result != RecoveryResult::Success) {
            return result;
        }
        
        callback(record); // let caller decide what they want to do with recovered record
        next_lsn_ = record.lsn + 1;
    }
}

void WAL::rewrite(const std::function<void(std::ostream&)>& writer) {
    std::ofstream temp(temp_name_, std::ios::binary);
    writer(temp);
    temp.close();
    stream_.close();
    std::remove(filename_.c_str());
    std::rename(temp_name_.c_str(), filename_.c_str());
    stream_.open(filename_, std::ios::binary | std::ios::app);
}

void WAL::flush() {
    stream_.flush();
}

WAL::~WAL() {
    if (stream_.is_open()) {
        stream_.close();
    }
}
