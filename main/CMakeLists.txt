idf_component_register(SRCS "animation.cpp" "color.cpp" "brightness.cpp" "speed.cpp" "christmas_garland.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES led_strip
		    PRIV_REQUIRES esp_driver_gpio
)


