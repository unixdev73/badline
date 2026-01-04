add_custom_target(umlDoc
	COMMAND clang-uml -c ${CMAKE_SOURCE_DIR}/.clang-uml
	COMMAND plantuml -Tsvg ${CMAKE_BINARY_DIR}/diagrams/argParser.puml
	COMMAND plantuml -Tsvg ${CMAKE_BINARY_DIR}/diagrams/renderEngine.puml
)
