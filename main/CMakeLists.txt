idf_component_register(SRCS "listeners.cpp" "animation.cpp" "christmas_garland.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES led_strip
		    PRIV_REQUIRES esp_driver_gpio
)


