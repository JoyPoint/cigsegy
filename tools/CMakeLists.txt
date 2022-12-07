add_executable(SEGYRead SEGYRead.cpp cxxopts.hpp)
target_link_libraries(SEGYRead PUBLIC segy fmt::fmt)

target_compile_options(SEGYRead PRIVATE -Werror=return-type)
set_target_properties(SEGYRead PROPERTIES FOLDER tools)
target_compile_options(SEGYRead PRIVATE -Wall -Wextra -pedantic -Werror -Wno-unused-parameter -fms-extensions)

install(TARGETS SEGYRead 
  RUNTIME DESTINATION bin
)
if(APPLE)
  set_target_properties(SEGYRead PROPERTIES INSTALL_RPATH "@executable_path/../lib${LIBSUFFIX}")
else()
  set_target_properties(SEGYRead PROPERTIES INSTALL_RPATH "$ORIGIN/../lib${LIBSUFFIX}")
endif()



add_executable(SEGYCreate SEGYCreate.cpp cxxopts.hpp)
target_link_libraries(SEGYCreate PUBLIC segy fmt::fmt)

target_compile_options(SEGYCreate PRIVATE -Werror=return-type)
set_target_properties(SEGYCreate PROPERTIES FOLDER tools)
target_compile_options(SEGYCreate PRIVATE -Wall -Wextra -pedantic -Werror -Wno-unused-parameter -fms-extensions)

install(TARGETS SEGYCreate
  RUNTIME DESTINATION bin
)
if(APPLE)
  set_target_properties(SEGYCreate PROPERTIES INSTALL_RPATH "@executable_path/../lib")
else()
  set_target_properties(SEGYCreate PROPERTIES INSTALL_RPATH "$ORIGIN/../lib")
endif()
