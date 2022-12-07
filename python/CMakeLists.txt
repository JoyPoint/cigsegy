pybind11_add_module(cigsegy MODULE
  PySegy.cpp 
)

target_include_directories(cigsegy PRIVATE ${PYBIND11_INCLUDE_DIR})
target_link_libraries(cigsegy PRIVATE segy)

install(TARGETS cigsegy 
  LIBRARY DESTINATION python
)