function(addShader targetName pathToSrc outputPath comment)
	get_filename_component(shaderOutDir ${outputPath} DIRECTORY)
	file(MAKE_DIRECTORY ${shaderOutDir})

	add_custom_command(OUTPUT ${outputPath}
		COMMAND glslangValidator -V ${pathToSrc} -o ${outputPath}
		DEPENDS ${pathToSrc}
		COMMENT ${comment}
	)

	add_custom_target(${targetName} ALL DEPENDS ${outputPath})
endfunction()

addShader(vertexShader
	${CMAKE_CURRENT_SOURCE_DIR}/shader.vert
	${CMAKE_CURRENT_BINARY_DIR}/vertex.spv
	"Compiling vertex shader"
)

addShader(fragmentShader
	${CMAKE_CURRENT_SOURCE_DIR}/shader.frag
	${CMAKE_CURRENT_BINARY_DIR}/fragment.spv
	"Compiling fragment shader"
)

addShader(fragNoSamplerShader
	${CMAKE_CURRENT_SOURCE_DIR}/shaderNoSampler.frag
	${CMAKE_CURRENT_BINARY_DIR}/fragNoSampler.spv
	"Compiling fragment-without-sampler shader"
)
