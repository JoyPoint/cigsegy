pybind11_add_module(cigsegy MODULE
  PySegy.cpp 
)

target_include_directories(cigsegy PRIVATE ${PYBIND11_INCLUDE_DIR})
target_link_libraries(cigsegy PRIVATE segy)

install(TARGETS cigsegy 
  LIBRARY DESTINATION python/cigsegy
)

set(COPYRIGHT 
  ${CMAKE_SOURCE_DIR}/LICENSE
  ${CMAKE_SOURCE_DIR}/Attributions.md
  ${CMAKE_SOURCE_DIR}/README.md
  ${CMAKE_SOURCE_DIR}/setup.py)
set(PYFILE
  ${CMAKE_SOURCE_DIR}/python/cigsegy.pyi
  ${CMAKE_SOURCE_DIR}/python/__init__.py)
install(FILES ${COPYRIGHT} DESTINATION python)
install(FILES ${PYFILE} DESTINATION python/cigsegy)