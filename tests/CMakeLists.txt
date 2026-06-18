# SYSMON Test Suite
# Tests will be added in Phase 8

# For now, just a placeholder
add_executable(test_placeholder test_placeholder.cpp)
target_link_libraries(test_placeholder PRIVATE sysmon_core)
add_test(NAME placeholder COMMAND test_placeholder)
