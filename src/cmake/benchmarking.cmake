CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

# CREATE_NAMED_BENCHMARK(foo bar.cc) creates a benchmark named `foo` from the
# file `bar.cc`.
MACRO(CREATE_NAMED_BENCHMARK NAME FILENAME)
    ADD_EXECUTABLE(${NAME} ${FILENAME})
    ADD_DEPENDENCIES(${NAME} googlebenchmark)
    TARGET_LINK_LIBRARIES(${NAME}
        ${GBENCH_LIBS_DIR}/libbenchmark.a
        pthread
    )
ENDMACRO(CREATE_NAMED_BENCHMARK)

# CREATE_BENCHMARK(foo) creates a benchmark named `foo` from the file `foo.cc`.
MACRO(CREATE_BENCHMARK NAME)
    CREATE_NAMED_BENCHMARK(${NAME} ${NAME}.cc)
ENDMACRO(CREATE_BENCHMARK)
