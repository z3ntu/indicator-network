add_library(
    menuharness
    MatchResult.cpp
    MatchUtils.cpp
    MenuItemMatcher.cpp
    MenuMatcher.cpp
)

target_link_libraries(
    menuharness
    ${GLIB_LDFLAGS}
)
