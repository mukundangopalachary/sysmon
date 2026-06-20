#ifndef SYSMON_TEST_UTILS_H
#define SYSMON_TEST_UTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST(name) void name()

#define EXPECT_EQ(expected, actual) \
    if ((expected) != (actual)) { \
        fprintf(stderr, "%s:%d: Failure\n  Expected: %lld\n  Actual: %lld\n", __FILE__, __LINE__, (long long)(expected), (long long)(actual)); \
        exit(1); \
    }

#define EXPECT_STREQ(expected, actual) \
    if (strcmp((expected), (actual)) != 0) { \
        fprintf(stderr, "%s:%d: Failure\n  Expected: %s\n  Actual: %s\n", __FILE__, __LINE__, (expected), (actual)); \
        exit(1); \
    }

#define EXPECT_TRUE(condition) \
    if (!(condition)) { \
        fprintf(stderr, "%s:%d: Failure\n  Expected true\n", __FILE__, __LINE__); \
        exit(1); \
    }

#define EXPECT_FALSE(condition) \
    if (condition) { \
        fprintf(stderr, "%s:%d: Failure\n  Expected false\n", __FILE__, __LINE__); \
        exit(1); \
    }

#define RUN_TEST(test) \
    printf("Running %s...\n", #test); \
    test(); \
    printf("  %s passed.\n", #test);

#endif
