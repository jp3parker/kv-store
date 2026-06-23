
#include <iostream>
#include <sstream>
#include "kv_store.h"

int main() {

    std::string wal_file = "data/wal.log";
    
        KVStore store(wal_file);
        store.put("a", "1");
        store.del("a");
    
    
    KVStore recovered(wal_file);
    recovered.recover();
    
    std::cout << "recovered.get(\"a\") = " << recovered.get("a") << std::endl;
    
    
//    std::cout << "recovered.recover() = " << static_cast<int>(recovered.recover()) << std::endl;
//    std::cout << "RecoveryResult::Success = " << static_cast<int>(RecoveryResult::Success) << std::endl;
//    EXPECT_EQ(recovered.recover(), RecoveryResult::Success);
//    EXPECT_EQ(recovered.get("blob"), value);

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
