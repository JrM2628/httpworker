## Bot communication (HTTP POST)
| endpoint   | arguments | description |
| :---       |  :----  | :---       |
| /heartbeat | N/A |Main endpoint for checking in. Returns queued up command|
| /info | JSON object containing "publicip", "username", "computername", "region", "memory", "netinfo", and "osinfo"|Endpoint for receiving info from bot and updating record in DB|
| /ps | JSON data {"procid":"procname", "procid":"procname"...} |Endpoint for receiving process list and updating record in DB|
| /upload | File 'file' being uploaded |Endpoint for receiving file uploads and writing them to /[uploads]/[uuid]/[timestamp].bin|
| /out | JSON data {"command":command, "output":output} |Endpoint for receiving command output and writing it to DB|


## Data API (HTTP GET)
Requires authentication
| endpoint   | arguments | description |
| :---       |  :---- | :--- |
| /api/v1/ip | N/A | Returns IP address of sender (authentication not required) |
| /api/v1/bots | N/A | JSON string containing all bots, their most recent checkin time, and all IP addresses  |
| /api/v1/stats | N/A | JSON string containing the number of bots and users in the database |
| /api/v1/bot/<uuid> | UUID of bot | JSON string containing bot info: "publicip", "username", "computername", "region", "memory", "netinfo", and "osinfo" |
| /api/v1/processlist/<uuid> | UUID of bot | JSON string containing process list {"procid":"procname", "procid":"procname"...} |
| /api/v1/network/<uuid> | UUID of bot | Network data of bot {{"adaptername"}:{"ip":ip, "mac":mac}, ...} |
| /api/v1/username | N/A | IP address |
| /api/v1/uploads | N/A | Not yet functioning |


## Action API (HTTP POST)
Requires authentication
| endpoint   | arguments | description |
| :---       |  :----  | :--- |
| /action/run | id, cmd | Queue up a command to run for a given bot UUID |
| /action/upload | id, path | Queue up a file to upload for a given bot UUID |
| /action/download | id, url, filepath | Queue up a file to download (from URL to filepath) for a given bot UUID |
| /action/kill | id, pid | Kill a process by PID for a given bot UUID |
| /action/info | id | Request client information from a given bot UUID |
| /action/ps | id | Request an updated process list from a given bot UUID |
| /action/ss | id | Request a screenshot from a given bot UUID |
| /action/loadlibrary | id, librarypath | Queue up a DLL to be loaded from a given bot UUID |
| /action/clear | id | Delete any queued up commands for a given bot UUID |
| /action/delete | id | Delete a bot and command history for a given bot UUID |


## Authentication API (HTTP POST)
| endpoint   | arguments | description |
| :---       |  :----  | :---       |
| /login | username, password | Attempts to authenticate a user with the supplied credentials|
| /logout | N/A | Pops the session and logs the user out |
| /signup | email, username, password | Create an account with the given email, username, and password (case sensitive) |