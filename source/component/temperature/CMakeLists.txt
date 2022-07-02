# Setting component name
set(COMPONENT_NAME "Temperature")

# Setting component description
set(COMPONENT_DESCRIPTION "This component manages the temperature measurements")

add_new_component(NAME
                    ${COMPONENT_NAME}
                  DESCRIPTION
                    ${COMPONENT_DESCRIPTION}
                  SOURCE_LIST
                    source/Temperature.cpp
                  INCLUDE_LIST
                    include
                  LINK_LIST
                    device
)