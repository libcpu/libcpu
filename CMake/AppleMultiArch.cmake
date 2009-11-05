IF (APPLE)
  EXEC_PROGRAM (sw_vers ARGS -productVersion OUTPUT_VARIABLE MACOSX_VERSION)

  IF (MACOSX_VERSION MATCHES "10.4")
    #
    # Build for MacOS X 10.4, all 2 architectures, also ppc64
    # since we do not depend upon any strange frameworks (true?).
    # 
    SET (CMAKE_OSX_ARCHITECTURES "i386;ppc;ppc64")
  ELIF (MACOSX_VERSION MATCHES "10.5")
    #
    # Build for MacOS X 10.5, all 4 architectures.
    # 
    SET (CMAKE_OSX_ARCHITECTURES "i386;x86_64;ppc;ppc64")
  ELIF (MACOSX_VERSION MATCHES "10.6")
    #
    # For MacOS X 10.6, all Intel architectures.
    #
    SET (CMAKE_OSX_ARCHITECTURES "i386;x86_64")
  ENDIF (MACOSX_VERSION MATCHES "10.4")
ENDIF (APPLE)
