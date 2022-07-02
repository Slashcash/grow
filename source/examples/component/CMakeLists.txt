# Setting component name
set(COMPONENT_NAME "Dummy")

# Setting component description
set(COMPONENT_DESCRIPTION "This is a dummy component used as an example")

add_new_component(NAME
                    ${COMPONENT_NAME}
                  DESCRIPTION
                    ${COMPONENT_DESCRIPTION}
                  SOURCE_LIST
                    source/Dummy.cpp
                  INCLUDE_LIST
                    include
)
