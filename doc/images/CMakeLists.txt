
SET(LOCAL_FILES
    boost-inside.jpeg
    Fig1-originalData.png
    Fig2-cost.png
    Fig4-costUndirected.png
    Fig6-undirected.png
    with_restrictions.png
    restrictions.png
    )

foreach (f ${LOCAL_FILES})
    configure_file(${f} "${f}" COPYONLY)
    list(APPEND LOCAL_IMG_FILES  ${CMAKE_CURRENT_BINARY_DIR}/${f})
endforeach()

set(PROJECT_IMG_FILES ${PROJECT_IMG_FILES} ${LOCAL_IMG_FILES} PARENT_SCOPE)

