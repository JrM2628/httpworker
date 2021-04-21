from flask import Flask

app = Flask(__name__)
#16 MB max download size
app.config['MAX_CONTENT_LENGTH'] = 16 * 1024 * 1024
app.config['UPLOAD_FOLDER'] = 'uploads'
app.secret_key = b'csec476malware'
app.malware_key = "CSEC476"

from sqlweb import auth
from sqlweb import botmanager
from sqlweb import dbmain
dbmain.initdb()
