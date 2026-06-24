
#include "test_kv_store.h"

TEST_F(KVStoreTest, RecoverDetectsCorruptedChecksum)
{
    {
        KVStore store(wal_file);
        store.put("a", "1");
    }

    std::fstream f(wal_file,
        std::ios::binary | std::ios::in | std::ios::out);

    char byte;
    f.read(&byte,1);      // first checksum byte
    byte ^= 1;
    f.seekp(0);
    f.write(&byte,1);

    f.close();

    KVStore recovered(wal_file);

    EXPECT_EQ(recovered.recover(),
              RecoveryResult::ChecksumMismatch);
}
TEST_F(KVStoreTest, RecoverDetectsCorruptedPutRecord)
{
    {
        KVStore store(wal_file);
        store.put("a", "1");
    }

    std::fstream f(wal_file, std::ios::binary | std::ios::in | std::ios::out);

    f.seekg(13);          // somewhere inside the payload
    char byte;
    f.read(&byte, 1);
    byte ^= 1;
    f.seekp(13);
    f.write(&byte, 1);
    f.close();

    KVStore recovered(wal_file);

    EXPECT_EQ(recovered.recover(),
              RecoveryResult::ChecksumMismatch);
}
TEST_F(KVStoreTest, RecoverDetectsCorruptedDelRecord)
{
    {
        KVStore store(wal_file);
        store.del("a");
    }

    std::fstream f(wal_file, std::ios::binary | std::ios::in | std::ios::out);

    f.seekg(9);          // somewhere inside the payload
    char byte;
    f.read(&byte, 1);
    byte ^= 1;
    f.seekp(9);
    f.write(&byte, 1);
    f.close();

    KVStore recovered(wal_file);

    EXPECT_EQ(recovered.recover(),
              RecoveryResult::ChecksumMismatch);
}
TEST(WALTest, TruncatedChecksumReturnsTruncated)
{
    std::stringstream ss(std::ios::binary | std::ios::in | std::ios::out);

    WALSerializer::write_record(ss, WALRecord::Put("name", "alice"));

    std::string bytes = ss.str();
    bytes.resize(2);

    std::stringstream truncated(bytes,
        std::ios::binary | std::ios::in | std::ios::out);

    WALRecord record;

    EXPECT_EQ(
        WALParser::read_record(truncated, record),
        RecoveryResult::Truncated);
}
TEST(WALTest, SerializePutRoundTrips)
{
    std::stringstream ss(std::ios::binary | std::ios::in | std::ios::out);

    WALRecord written = WALRecord::Put("name", "alice");

    WALSerializer::write_record(ss, written);

    ss.seekg(0);

    WALRecord read;

    EXPECT_EQ(
        WALParser::read_record(ss, read),
        RecoveryResult::Success);

    EXPECT_EQ(read.op, WALRecord::Operation::Put);
    EXPECT_EQ(read.key, "name");
    EXPECT_EQ(read.value, "alice");
}
TEST(WALTest, SerializeDeleteRoundTrips)
{
    std::stringstream ss(std::ios::binary | std::ios::in | std::ios::out);

    WALRecord written = WALRecord::Delete("name");

    WALSerializer::write_record(ss, written);

    ss.seekg(0);

    WALRecord read;

    ASSERT_EQ(
        WALParser::read_record(ss, read),
        RecoveryResult::Success);

    EXPECT_EQ(read.op, WALRecord::Operation::Delete);
    EXPECT_EQ(read.key, "name");
    EXPECT_TRUE(read.value.empty());
}
TEST(WALTest, MultipleRecordsRoundTrip)
{
    std::stringstream ss(std::ios::binary | std::ios::in | std::ios::out);

    WALSerializer::write_record(ss,
        WALRecord::Put("a", "1"));

    WALSerializer::write_record(ss,
        WALRecord::Put("b", "2"));

    WALSerializer::write_record(ss,
        WALRecord::Delete("a"));

    ss.seekg(0);

    WALRecord record;

    ASSERT_EQ(
        WALParser::read_record(ss, record),
        RecoveryResult::Success);

    EXPECT_EQ(record.key, "a");
    EXPECT_EQ(record.value, "1");

    ASSERT_EQ(
        WALParser::read_record(ss, record),
        RecoveryResult::Success);

    EXPECT_EQ(record.key, "b");
    EXPECT_EQ(record.value, "2");

    ASSERT_EQ(
        WALParser::read_record(ss, record),
        RecoveryResult::Success);

    EXPECT_EQ(record.op,
              WALRecord::Operation::Delete);

    EXPECT_EQ(record.key, "a");
}
TEST(WALTest, EmptyKeyRoundTrips)
{
    std::stringstream ss(std::ios::in |
                     std::ios::out |
                     std::ios::binary);

    WALSerializer::write_record(
        ss,
        WALRecord::Put("", "value"));

    ss.seekg(0);

    WALRecord record;

    ASSERT_EQ(
        WALParser::read_record(ss, record),
        RecoveryResult::Success);

    EXPECT_EQ(record.key, "");
    EXPECT_EQ(record.value, "value");
}
TEST(WALTest, EmptyValueRoundTrips)
{
    std::stringstream ss(std::ios::in |
                     std::ios::out |
                     std::ios::binary);

    WALSerializer::write_record(
        ss,
        WALRecord::Put("key", ""));

    ss.seekg(0);

    WALRecord record;

    ASSERT_EQ(
        WALParser::read_record(ss, record),
        RecoveryResult::Success);

    EXPECT_EQ(record.key, "key");
    EXPECT_EQ(record.value, "");
}
TEST(WALTest, BinaryValueRoundTrips)
{
    std::string value;

    value.push_back('\0');
    value.push_back('A');
    value.push_back('\0');
    value.push_back('B');

    std::stringstream ss(std::ios::in |
                     std::ios::out |
                     std::ios::binary);

    WALSerializer::write_record(
        ss,
        WALRecord::Put("blob", value));

    ss.seekg(0);

    WALRecord record;

    ASSERT_EQ(
        WALParser::read_record(ss, record),
        RecoveryResult::Success);

    EXPECT_EQ(record.value, value);
}
TEST(WALTest, TruncatedRecordReturnsTruncated)
{
    std::stringstream ss(std::ios::in |
                     std::ios::out |
                     std::ios::binary);
    

    WALSerializer::write_record(
        ss,
        WALRecord::Put("abcdef", "123456"));

    std::string bytes = ss.str();

    bytes.pop_back();      // simulate crash during write

    std::stringstream truncated(bytes,
        std::ios::in |
        std::ios::out |
        std::ios::binary);

    WALRecord record;

    EXPECT_EQ(
        WALParser::read_record(truncated, record),
        RecoveryResult::Truncated);
}
TEST(WALTest, InvalidOpcodeReturnsInvalidOperation)
{
    std::stringstream ss(std::ios::in |
                     std::ios::out |
                     std::ios::binary);

    uint32_t checksum = 0;
    uint64_t lsn = 99;
    uint8_t opcode = 99;

    ss.write(reinterpret_cast<char*>(&checksum),
             sizeof(checksum));

    ss.write(reinterpret_cast<char*>(&lsn),
             sizeof(lsn));

    ss.write(reinterpret_cast<char*>(&opcode),
             sizeof(opcode));

    ss.seekg(0);

    WALRecord record;

    EXPECT_EQ(
        WALParser::read_record(ss, record),
        RecoveryResult::InvalidOperation);
}
