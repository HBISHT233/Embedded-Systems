idf_component_register(
    SRCS "main.c" "st7789.c" "images.c"
    INCLUDE_DIRS "." "../includes" "../images" 
    EMBED_FILES "temp.bin" 
                "humidity.bin"
                "w_hot.bin" "w_rain.bin" "w_snow.bin" "w_wind.bin" 
                "korea.bin"
                "sun.bin" "Snow.bin" "rain.bin"
                "hot_wind.bin"
)