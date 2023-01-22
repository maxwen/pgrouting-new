add_library(trsp_sqlite OBJECT
    # keeping these for backwards compatibility
    trsp.c
    trsp_core.cpp
    GraphDefinition.cpp
    )
link_directories(sqlite3)
