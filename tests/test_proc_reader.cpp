#include "test_utils.h"

extern "C" {
#include "proc_reader.h"
}

TEST(test_parse_uint64) {
    EXPECT_EQ(12345, parse_uint64("12345"));
    EXPECT_EQ(0, parse_uint64("0"));
    EXPECT_EQ(18446744073709551615ULL, parse_uint64("18446744073709551615"));
    EXPECT_EQ(42, parse_uint64("42abc")); // Stops at first non-digit
}

TEST(test_read_file_to_buf) {
    char buf[128];
    // Write a temp file
    FILE* f = fopen("/tmp/sysmon_test.txt", "w");
    fputs("Hello World\n", f);
    fclose(f);

    int bytes = read_file_to_buf("/tmp/sysmon_test.txt", buf, sizeof(buf));
    EXPECT_EQ(12, bytes);
    EXPECT_STREQ("Hello World\n", buf);

    // Clean up
    remove("/tmp/sysmon_test.txt");
}

int main() {
    RUN_TEST(test_parse_uint64);
    RUN_TEST(test_read_file_to_buf);
    return 0;
}
