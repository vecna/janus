ADD_EXECUTABLE(
    janus
    packet_queue
    os_cmds
    janus
    main
)

SET( PACKAGELIST PCAP EVENT )

FOREACH( PACKAGE ${PACKAGELIST} )

    FIND_PACKAGE( ${PACKAGE} REQUIRED )
    MESSAGE( "${PACKAGE}: FOUND" )
    SET( HAVE_LIB${PACKAGE} 1 )
    INCLUDE_DIRECTORIES( ${${PACKAGE}_INCLUDE_DIRS} )
    TARGET_LINK_LIBRARIES( janus ${${PACKAGE}_LIBRARY} )
    MESSAGE( STATUS "${PACKAGE} includes: ${${PACKAGE}_INCLUDE_DIRS}" )
    MESSAGE( STATUS "${PACKAGE} libs: ${${PACKAGE}_LIBRARIES}" )

ENDFOREACH(PACKAGE ${PACKAGELIST})

SET( CMAKE_C_FLAGS "-g3 -O3 -Wall -pedantic ${CMAKE_C_FLAGS}" )

CONFIGURE_FILE(config.h.in config.h)

INSTALL(
    TARGETS janus
    RUNTIME DESTINATION /usr/local/sbin
)
