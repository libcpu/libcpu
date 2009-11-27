IF (APPLE)
  # if MACOSX_DEPLOYMENT_TARGET env is set, use that.
  SET (MACOSX_VERSION $ENV{MACOSX_DEPLOYMENT_TARGET})
  IF (NOT MACOSX_VERSION)
    EXEC_PROGRAM (sw_vers ARGS -productVersion OUTPUT_VARIABLE MACOSX_VERSION)
  ENDIF (NOT MACOSX_VERSION)

  MESSAGE(STATUS "Using SDK for MacOS X ${MACOSX_VERSION}")

  IF (MACOSX_VERSION MATCHES "10.4")
    #
    # Build for MacOS X 10.4, all 2 architectures, also ppc64
    # since we do not depend upon any strange frameworks (true?).
    # 
    SET (CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.4u.sdk/")
    SET (CMAKE_OSX_ARCHITECTURES "i386;ppc;ppc64")
    ADD_DEFINITIONS(-faltivec)
  ELSEIF (MACOSX_VERSION MATCHES "10.5")
    #
    # Build for MacOS X 10.5, all 4 architectures.
    # 
    SET (CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.5.sdk/")
    SET (CMAKE_OSX_ARCHITECTURES "i386;x86_64;ppc;ppc64")
    ADD_DEFINITIONS(-faltivec)
  ELSEIF (MACOSX_VERSION MATCHES "10.6")
    #
    # For MacOS X 10.6, all Intel architectures.
    #
    SET (CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.6.sdk/")
    SET (CMAKE_OSX_ARCHITECTURES "i386;x86_64")
  ENDIF (MACOSX_VERSION MATCHES "10.4")
ENDIF (APPLE)
