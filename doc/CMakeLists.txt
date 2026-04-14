if (UNIX)
	find_program(clangUmlBin clang-uml REQUIRED)
	find_program(plantUmlBin plantuml REQUIRED)
elseif(WIN32)
	find_program(clangUmlBin clang-uml PATHS "C:/Program Files/clang-uml/bin" REQUIRED)
	find_program(plantUmlBin plantuml PATHS "C:/Program Files/plant-uml" REQUIRED)
endif()

add_custom_target(umlDoc
	COMMAND ${clangUmlBin} -c ${CMAKE_SOURCE_DIR}/.clang-uml
	COMMAND ${plantUmlBin} -Tsvg ${CMAKE_BINARY_DIR}/diagrams/argParser.puml
	COMMAND ${plantUmlBin} -Tsvg ${CMAKE_BINARY_DIR}/diagrams/renderEngine.puml
)