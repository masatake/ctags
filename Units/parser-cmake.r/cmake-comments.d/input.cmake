# this is a test of comments set(DO_NOT_TAG "foo")

#[[
multi-linecomments
option(DO_NOT_TAG "foo" OFF)
] not the end
]]set(tag_this)

add_custom_target(# comment set(NO_TAG "foo")
    # anothe rline comment
    good_target# this is legal comment placement I think
    ALL)

add_library(another_good_target# <-- target
    SHARED # <-- lib type
    gmock-all.cc
    )
