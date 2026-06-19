#include <gtest/gtest.h>
#include "kv_store.h"

class KVStoreTest : public ::testing::Test {
protected:
    std::string wal_file = "data/test.wal";

    void SetUp() override {
        std::remove(wal_file.c_str());
    }

    void TearDown() override {
        std::remove(wal_file.c_str());
    }
};

TEST_F(KVStoreTest, PutAndGet) {
    KVStore store(wal_file);
    store.put("name", "alice");
    EXPECT_EQ(store.get("name"), "alice");
}

TEST_F(KVStoreTest, OverwriteKey) {
    KVStore store(wal_file);
    store.put("name", "alice");
    store.put("name", "bob");
    EXPECT_EQ(store.get("name"), "bob");
}

TEST_F(KVStoreTest, LargeValue) {
    KVStore store(wal_file);
    std::string value(100000, 'x');
    store.put("blob", value);
    EXPECT_EQ(store.get("blob"), value);
}

TEST_F(KVStoreTest, RecoverLargeValue) {
    std::string value(100000, 'x');
    {
        KVStore store(wal_file);
        store.put("blob", value);
    }
    KVStore recovered(wal_file);
    recovered.recover();
    EXPECT_EQ(recovered.get("blob"), value);
}

TEST_F(KVStoreTest, EmptyKey) {
    KVStore store(wal_file);
    store.put("", "value");
    EXPECT_EQ(store.get(""), "value");
}

TEST_F(KVStoreTest, KeyContainsSpaces) {
    KVStore store(wal_file);
    store.put("first last", "alice");
    EXPECT_EQ(store.get("first last"), "alice");
}

TEST_F(KVStoreTest, ValueContainsSpaces) {
    KVStore store(wal_file);
    store.put("msg", "hello world");
    EXPECT_EQ(store.get("msg"), "hello world");
}

TEST_F(KVStoreTest, ValueContainsNewlines) {
    KVStore store(wal_file);
    std::string value = "hello\nworld\nfoo";
    store.put("msg", value);
    EXPECT_EQ(store.get("msg"), value);
}

TEST_F(KVStoreTest, BinaryValue) {
    KVStore store(wal_file);
    std::string value;
    value.push_back('\0');
    value.push_back('a');
    value.push_back('\0');
    value.push_back('b');
    store.put("blob", value);
    EXPECT_EQ(store.get("blob"), value);
}

TEST_F(KVStoreTest, RecoverBinaryValue) {
    std::string value;
    value.push_back('\0');
    value.push_back('a');
    value.push_back('\0');
    value.push_back('b');
    {
        KVStore store(wal_file);
        store.put("blob", value);
    }
    KVStore recovered(wal_file);
    EXPECT_EQ(recovered.recover(), RecoveryResult::Success);
    EXPECT_EQ(recovered.get("blob"), value);
}

TEST_F(KVStoreTest, EmptyKeyAndValue) {
    KVStore store(wal_file);
    store.put("", "");
    EXPECT_EQ(store.get(""), "");
}

TEST_F(KVStoreTest, DeleteKey) {
    KVStore store(wal_file);
    store.put("name", "alice");
    store.del("name");
    EXPECT_EQ(store.get("name"), "");
}

TEST_F(KVStoreTest, MultipleDeletes) {
    KVStore store(wal_file);
    store.put("a", "1");
    store.del("a");
    store.del("a");
    EXPECT_EQ(store.get("a"), "");
}

TEST_F(KVStoreTest, DeleteMissingKey) {
    KVStore store(wal_file);
    EXPECT_NO_THROW(store.del("missing"));
}

TEST_F(KVStoreTest, GetMissingKey) {
    KVStore store(wal_file);
    EXPECT_EQ(store.get("missing"), "");
}

TEST_F(KVStoreTest, RecoverEmptyWal) {
    KVStore store(wal_file);
    EXPECT_EQ(store.recover(), RecoveryResult::Success);
}

TEST_F(KVStoreTest, RecoverPut) {
    {
        KVStore store(wal_file);
        store.put("a", "1");
        store.put("b", "2");
    }
    KVStore recovered(wal_file);
    recovered.recover();
    EXPECT_EQ(recovered.get("a"), "1");
    EXPECT_EQ(recovered.get("b"), "2");
}

TEST_F(KVStoreTest, RecoverDelete) {
    {
        KVStore store(wal_file);
        store.put("a", "1");
        store.del("a");
    }
    KVStore recovered(wal_file);
    recovered.recover();
    EXPECT_EQ(recovered.get("a"), "");
}

TEST_F(KVStoreTest, RecoverOverwrite) {
    {
        KVStore store(wal_file);
        store.put("a", "1");
        store.put("a", "2");
        store.put("a", "3");
    }
    KVStore recovered(wal_file);
    recovered.recover();
    EXPECT_EQ(recovered.get("a"), "3");
}

//TEST_F(KVStoreTest, RecoveryStopsOnCorruption) {
//    std::ofstream wal(wal_file);
//    wal << "PUT 1 1 a1\n";
//    wal << "PUT 1 1 b2\n";
//    wal << "BADOP\n";
//    wal << "PUT 1 1 c3\n";
//    wal.close();
//    KVStore store(wal_file);
//    EXPECT_FALSE(store.recover());
//    EXPECT_EQ(store.get("a"), "1");
//    EXPECT_EQ(store.get("b"), "2");
//    EXPECT_EQ(store.get("c"), "");
//}
//
//TEST_F(KVStoreTest, RidiculousLength) {
//    std::ofstream wal(wal_file);
//    wal << "PUT 99999999999999999999 1 a\n";
//    wal.close();
//    KVStore store(wal_file);
//    EXPECT_FALSE(store.recover());
//}
//
//TEST_F(KVStoreTest, InvalidLengthField) {
//    std::ofstream wal(wal_file);
//    wal << "PUT X 5 abcde\n";
//    wal.close();
//    KVStore store(wal_file);
//    EXPECT_FALSE(store.recover());
//}
//
//TEST_F(KVStoreTest, CorruptedWalReturnsFailure) {
//    KVStore store(wal_file);
//    std::ofstream wal(wal_file);
//    wal << "PUT 3 5 abc";
//    wal.close();
//    EXPECT_FALSE(store.recover());
//}

TEST_F(KVStoreTest, EmptyValue) {
    KVStore store(wal_file);
    store.put("key", "");
    EXPECT_EQ(store.get("key"), "");
}

TEST_F(KVStoreTest, RandomOperationsRecoverCorrectly) {
    std::unordered_map<std::string,std::string> expected;

    {
        KVStore store(wal_file);

        for (int i = 0; i < 1000; i++) {
            std::string key = "k" + std::to_string(rand() % 50);

            if (rand() % 2) {
                std::string value = std::to_string(rand());
                store.put(key, value);
                expected[key] = value;
            } else {
                store.del(key);
                expected.erase(key);
            }
        }
    }

    KVStore recovered(wal_file);
    recovered.recover();

    for (auto& [k,v] : expected) {
        EXPECT_EQ(recovered.get(k), v);
    }
}

TEST_F(KVStoreTest, RecoverTwice) {
    {
        KVStore store(wal_file);
        store.put("a", "1");
        store.put("b", "2");
    }
    KVStore store(wal_file);
    EXPECT_EQ(store.recover(), RecoveryResult::Success);
    EXPECT_EQ(store.recover(), RecoveryResult::Success);
    EXPECT_EQ(store.get("a"), "1");
    EXPECT_EQ(store.get("b"), "2");
}

TEST_F(KVStoreTest, PutAfterRecovery) {
    {
        KVStore store(wal_file);
        store.put("a", "1");
    }
    KVStore store(wal_file);
    EXPECT_EQ(store.recover(), RecoveryResult::Success);
    store.put("b", "2");
    EXPECT_EQ(store.get("a"), "1");
    EXPECT_EQ(store.get("b"), "2");
}

TEST_F(KVStoreTest, RecoveryAfterPutAfterRecovery) {
    {
        KVStore store(wal_file);
        store.put("a", "1");
    }
    KVStore store(wal_file);
    EXPECT_EQ(store.recover(), RecoveryResult::Success);
    store.put("b", "2");
    EXPECT_EQ(store.get("a"), "1");
    EXPECT_EQ(store.get("b"), "2");
    KVStore recovered(wal_file);
    recovered.recover();
    EXPECT_EQ(recovered.get("a"), "1");
    EXPECT_EQ(recovered.get("b"), "2");
}

TEST_F(KVStoreTest, DeleteAfterRecovery) {
    {
        KVStore store(wal_file);
        store.put("a", "1");
    }
    KVStore store(wal_file);
    store.recover();
    store.del("a");
    EXPECT_EQ(store.get("a"), "");
}

TEST_F(KVStoreTest, FuzzRecovery) {
    std::unordered_map<std::string,std::string> oracle;
    for (int run = 0; run < 100; run++) {
        std::remove(wal_file.c_str());
        {
            KVStore store(wal_file);
            for (int i = 0; i < 500; i++) {
                std::string key = std::string(1, char(rand() % 128));
                std::string value;
                int len = rand() % 50;
                for (int j = 0; j < len; j++) {
                    value.push_back(char(rand() % 256));
                }
                if (rand() % 2) {
                    store.put(key, value);
                    oracle[key] = value;
                }
                else {
                    store.del(key);
                    oracle.erase(key);
                }
            }
        }
        KVStore recovered(wal_file);
        EXPECT_EQ(recovered.recover(), RecoveryResult::Success);
        for (auto& [k,v] : oracle)
            EXPECT_EQ(recovered.get(k), v);
        oracle.clear();
    }
}

TEST_F(KVStoreTest, CompactRemovesOldVersions)
{
    {
        KVStore store(wal_file);

        store.put("a", "1");
        store.put("a", "2");
        store.put("a", "3");
        store.put("b", "x");

        store.compact();
    }

    KVStore recovered(wal_file);

    ASSERT_EQ(recovered.recover(), RecoveryResult::Success);

    EXPECT_EQ(recovered.get("a"), "3");
    EXPECT_EQ(recovered.get("b"), "x");
}


TEST_F(KVStoreTest, CompactShrinksWal)
{
    size_t before;

    {
        KVStore store(wal_file);

        for (int i = 0; i < 100; i++)
            store.put("counter", std::to_string(i));

        before = std::filesystem::file_size(wal_file);

        store.compact();
    }

    size_t after = std::filesystem::file_size(wal_file);

    EXPECT_LT(after, before);
}
