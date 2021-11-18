from flask import request
from flask import render_template
from flask import redirect, url_for
from flask import session

from c2 import app
from c2.app import get_db
from c2 import dbmain


@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        conn = get_db()
        auth_status, user_name = dbmain.authenticate_user(conn, request.form['username'], request.form['password'])
        if auth_status:
            session['username'] = user_name
            return redirect(url_for('index'))
        return "Incorrect login credentials"
    else:
        return render_template('login.html')


@app.route('/logout')
def logout():
    session.pop('username', None)
    return redirect(url_for('index'))


@app.route('/signup', methods=['GET', 'POST'])
def signup():
    if request.method == 'POST':
        conn = get_db()
        if dbmain.add_user(conn, request.form['email'], request.form['username'], request.form['password']):
            return redirect(url_for('login'))
        else:
            return "Error creating user. Maybe username is taken?"
    else:
        if session.get("username") is None:
            return render_template('adduser.html')
        else:
            return redirect(url_for('index'))
