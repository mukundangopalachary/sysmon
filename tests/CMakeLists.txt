# SYSMON Test Suite

add_executable(test_proc_reader test_proc_reader.cpp)
target_link_libraries(test_proc_reader PRIVATE sysmon_core)
add_test(NAME proc_reader COMMAND test_proc_reader)

add_executable(test_snapshot_manager test_snapshot_manager.cpp)
target_link_libraries(test_snapshot_manager PRIVATE sysmon_core)
add_test(NAME snapshot_manager COMMAND test_snapshot_manager)
