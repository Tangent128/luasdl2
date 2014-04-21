# - Locate and check sys/queue.h presence
#
# SYSQUEUE_FOUND - system has sys/queue.h
# SYSQUEUE_SLIST - queue.h has SLIST macros
# SYSQUEUE_STAILQ - queue.h has STAILQ macros
# SYSQUEUE_LIST - queue.h has LIST macros
# SYSQUEUE_TAILQ - queue.h has TAILQ macros
# SYSQUEUE_SIMPLEQ - queue.h has SIMPLEQ macros
# SYSQUEUE_CIRCLEQ - queue.h has CIRCLEQ macros
# SYSQUEUE_TAILQ - queue.h has TAILQ macros
#
# Some macros are not available on every systems, these variables are
# defined if they are found in the queue.h file
#
# SYSQUEUE_SLIST_FOREACH - queue.h has SLIST_FOREACH macros
# SYSQUEUE_SLIST_FOREACH_SAFE - queue.h has SLIST_FOREACH_SAFE macros
# SYSQUEUE_STAILQ_FOREACH - queue.h has STAILQ_FOREACH macros
# SYSQUEUE_STAILQ_FOREACH - queue.h has STAILQ_FOREACH_SAFE macros
# SYSQUEUE_LIST_FOREACH - queue.h has LIST_FOREACH macros
# SYSQUEUE_LIST_FOREACH_SAFE - queue.h has LIST_FOREACH_SAFE macros
# SYSQUEUE_TAILQ_FOREACH - queue.h has TAILQ_FOREACH macros
# SYSQUEUE_TAILQ_FOREACH - queue.h has TAILQ_FOREACH_SAFE macros
#

include(CheckIncludeFile)
include(CheckSymbolExists)

check_include_file(sys/queue.h SYSQUEUE_FOUND)
if (SYSQUEUE_FOUND)
	foreach (type "SLIST" "STAILQ" "LIST" "TAILQ" "SIMPLEQ" "CIRCLEQ")
		check_symbol_exists(${type}_HEAD sys/queue.h SYSQUEUE_${type})

		if (SYSQUEUE_${type})
			# Check for _FOREACH.
			check_symbol_exists(
			    ${type}_FOREACH
			    sys/queue.h
			    SYSQUEUE_${type}_FOREACH
			)

			# Check for _FOREACH_SAFE.
			check_symbol_exists(
			    ${type}_FOREACH_SAFE
			    sys/queue.h
			    SYSQUEUE_${type}_FOREACH_SAFE
			)
		endif ()
	endforeach ()
endif ()
