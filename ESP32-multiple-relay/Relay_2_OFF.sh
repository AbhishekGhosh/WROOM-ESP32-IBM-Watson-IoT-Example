#!/bin/bash  

CURL='/usr/bin/curl'

curl -u <use-the-API-Key>:<use-auth-token> -H "Content-Type: text/plain" -v -X POST http://<your org>.messaging.internetofthings.ibmcloud.com:1883/api/v0002/application/types/<yourDeviceType>/devices/<yourDeviceId>/commands/pin2 -d "off"

# or you can redirect it into a file:

# $CURL  > watson.log
