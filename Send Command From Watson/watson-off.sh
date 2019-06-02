CURL='/usr/bin/curl'
curl -u <use-the-Key>:<use-auth-token> -H "Content-Type: text/plain" -v -X POST http://tko2rj.messaging.internetofthings.ibmcloud.com:1883/api/v0002/application/types/DevBoard/devices/ESP32/commands/gpio -d "off"

# or you can redirect it into a file:

# $CURL  > watson.log
