find_program(clangUmlBin clang-uml REQUIRED)
find_program(plantUmlBin plantuml REQUIRED)

add_custom_target(umlDoc
	COMMAND ${clangUmlBin} -c ${CMAKE_SOURCE_DIR}/.clang-uml
	COMMAND ${plantUmlBin} -Tsvg ${CMAKE_BINARY_DIR}/diagrams/argParser.puml
	COMMAND ${plantUmlBin} -Tsvg ${CMAKE_BINARY_DIR}/diagrams/renderEngine.puml
)
