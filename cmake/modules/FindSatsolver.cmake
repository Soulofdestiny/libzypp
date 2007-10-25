
if(SATSOLVER_INCLUDE_DIR AND SATSOLVER_LIBRARY)
	# Already in cache, be silent
	set(SATSOLVER_FIND_QUIETLY TRUE)
endif(SATSOLVER_INCLUDE_DIR AND SATSOLVER_LIBRARY)

set(SATSOLVER_LIBRARY)
set(SATSOLVER_INCLUDE_DIR)

FIND_PATH(SATSOLVER_INCLUDE_DIR satsolver/solvable.h
	/usr/include
	/usr/local/include
)

FIND_FILE(SATSOLVER_LIBRARY libsatsolver.la
	PATHS
        /usr/lib64
        /usr/local/lib64
        /usr/lib
        /usr/local/lib
)

#FIND_LIBRARY(SATSOLVER_LIBRARY NAMES satsolver
#	PATHS
#	/usr/lib
#	/usr/local/lib
#)

if(SATSOLVER_INCLUDE_DIR AND SATSOLVER_LIBRARY)
   MESSAGE( STATUS "satsolver found: includes in ${SATSOLVER_INCLUDE_DIR}, library in ${SATSOLVER_LIBRARY}")
   set(SATSOLVER_FOUND TRUE)
else(SATSOLVER_INCLUDE_DIR AND SATSOLVER_LIBRARY)
   MESSAGE( STATUS "** satsolver not found")
   MESSAGE( STATUS "** install package libsatsolver-devel")
   MESSAGE( STATUS "** (http://svn.opensuse.org/svn/zypp/trunk/sat-solver)")
endif(SATSOLVER_INCLUDE_DIR AND SATSOLVER_LIBRARY)

MARK_AS_ADVANCED(SATSOLVER_INCLUDE_DIR SATSOLVER_LIBRARY)