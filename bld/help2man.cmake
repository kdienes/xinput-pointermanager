find_file(HELP2MAN_EXECUTABLE help2man HINTS /bin /usr/bin /usr/local/bin)
if(NOT HELP2MAN_EXECUTABLE)
  message(FATAL_ERROR "unable to find help2man")
endif(NOT HELP2MAN_EXECUTABLE)

find_file(SED_EXECUTABLE sed HINTS /bin /usr/bin /usr/local/bin)
if(NOT SED_EXECUTABLE)
  message(FATAL_ERROR "unable to find sed")
endif(NOT SED_EXECUTABLE)

macro(manpage program description)
ADD_CUSTOM_COMMAND (
  OUTPUT ${program}.1
  DEPENDS ${program}
  COMMAND ${HELP2MAN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/${program}
  -n "${description}"
  --no-info
  --output=${CMAKE_CURRENT_BINARY_DIR}/${program}.1
  COMMAND ${SED_EXECUTABLE} -i "s#${CMAKE_CURRENT_BINARY_DIR}/##g" ${CMAKE_CURRENT_BINARY_DIR}/${program}.1
  )
INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/${program}.1 DESTINATION share/man/man1)
endmacro(manpage)
