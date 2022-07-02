#!/bin/bash
OUTPUT_PATH="grow.cfg"
THERMOMETER_ENABLED=false
PROJECT_VERSION="invalid"

while getopts ":o:tv:" opt; do
  case $opt in
    o)
      OUTPUT_PATH="$OPTARG"
      ;;
    t)
      THERMOMETER_ENABLED=true
      ;;
    v)
      PROJECT_VERSION="$OPTARG"
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
    :)
      echo "Option -$OPTARG requires an argument." >&2
      exit 1
      ;;
  esac
done

echo -e "project_version = \"${PROJECT_VERSION}\";\n" > "${OUTPUT_PATH}"
if [ "$THERMOMETER_ENABLED" = true ] ; then
    echo -e "Temperature =\n{\n\tpoll_time = 1000;\n};" >> "${OUTPUT_PATH}"
fi

echo -e "Publisher =\n{" >> "${OUTPUT_PATH}"
if [ "$THERMOMETER_ENABLED" = true ] ; then
    echo -e "\tTemperature = 7000;" >> "${OUTPUT_PATH}"
fi

echo -e "};" >> "${OUTPUT_PATH}"

exit 0
