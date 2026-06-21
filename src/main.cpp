
#include <iostream>
#include <sstream>
#include "kv_store.h"

int main() {

    std::string wal_file = "data/wal.log";
        
    std::stringstream ss(std::ios::in |
                     std::ios::out |
                     std::ios::binary);

    WALSerializer::write_record(
        ss,
        WALRecord::Put("abcdef", "123456"));

    std::string bytes = ss.str();

    bytes.pop_back();      // simulate crash during write

    std::stringstream truncated(bytes,
        std::ios::binary);

    WALRecord record;
    
    std::cout << "WALParser::read_record(truncated, record) = " << static_cast<int>(WALParser::read_record(truncated, record)) << std::endl;
    
    std::cout << "RecoveryResult::Truncated = " << static_cast<int>(RecoveryResult::Truncated) << std::endl;
    

//    EXPECT_EQ(
//        WALParser::read_record(truncated, record),
//        RecoveryResult::Truncated);
    


    return 0;

}


//TEST_F(KVStoreTest, CompactRemovesOldVersions)
//{
//    {
//        KVStore store(wal_file);
//
//        store.put("a", "1");
//        store.put("a", "2");
//        store.put("a", "3");
//        store.put("b", "x");
//
//        store.compact();
//    }
//
//    KVStore recovered(wal_file);
//
//    ASSERT_EQ(recovered.recover(), RecoveryResult::Success);
//
//    EXPECT_EQ(recovered.get("a"), "3");
//    EXPECT_EQ(recovered.get("b"), "x");
//}
