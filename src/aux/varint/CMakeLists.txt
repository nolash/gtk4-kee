add_library(varint varint.c)
set_property(TARGET varint  PROPERTY C_VISIBILITY_PRESET default)
set_property(TARGET varint PROPERTY VISIBILITY_INLINES_HIDDEN OFF)
include_directories(.)
