idf_component_register(
    SRCS "main.c" "st7789.c" "images.c"
    INCLUDE_DIRS "." "../includes"
    EMBED_FILES "om.bin"
)