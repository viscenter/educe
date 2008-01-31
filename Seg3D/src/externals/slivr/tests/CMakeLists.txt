
ADD_EXECUTABLE(test_kd-tree test_kd-tree.cc)
TARGET_LINK_LIBRARIES(test_kd-tree slivrG)
INSTALL(TARGETS test_kd-tree RUNTIME DESTINATION bin/tests)
